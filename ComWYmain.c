#include <gmp.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include "blst/include/blst.h"
#include "database.h"
#include "utils.h"
#include "verifyhash.h"
#include "LMC.h"
#include "WY.h"

#define DIGITS 77 //Number of digits in an element
#define R "73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001" //R is prime number in BLS12-381. It is also the prime number p in Z_pcd

int main() 
{
	int id;
	int i_index;
	char tmp[DIGITS];
	int m_mapping;
	
	//case1: k = 2, t = 1, n = 2^10: running m (1, 1024, 4096, 16384, 65536)
	//n_value is the number of columns in the database
	int n_value[] = {1024};
	//m_value is the number of rows in the database
	int m_value[] = {1, 1024, 4096, 16384, 65536};
	int k_server[] = {2}; // 2*k >= d_mapping*t_private + 1
	int d_mapping[] = {3};
	int t_private = 1;
	int degree_d = d_mapping[0]*t_private;
	
	/*//case2: k = 2, t = 1, m = 2^10: running n (256, 1024, 4096, 16384, 65536)
	//n_value is the number of columns in the database
	int n_value[] = {256, 1024, 4096};
	//m_value is the number of rows in the database
	int m_value[] = {1024};
	int k_server[] = {2}; // 2*k >= d_mapping*t_private + 1
	int d_mapping[] = {3};
	int t_private = 1;
	int degree_d = d_mapping[0]*t_private;*/
	
	int** E_val;//store all E mapping values with d_mapping posiitons of 1 [max_n][d_mapping]
	int** E_full;//store all E mapping values
	int lengthm = sizeof(m_value)/sizeof(m_value[0]);
	int lengthn = sizeof(n_value)/sizeof(n_value[0]);
	int lengthk = sizeof(k_server)/sizeof(k_server[0]);
	double time, s_PIR_time, c_PIR_time, s_LMC_time, c_LMC_time;
	clock_t start, stop; //The time counters
	
	//initialization of a prime number p
	mpz_t p;
	mpz_init_set_str (p, R, 16);
	gmp_printf ("p = %Zd \n", p);
	mpz_t rop;
	mpz_init (rop);
	mpz_t coef;
	mpz_init (coef);
	gmp_randstate_t state;
	gmp_randinit_mt (state);
	
	mpz_t* det;
	mpz_t* det_a;
	mpz_t** F_Q; //[k_server][m_value + 1]
	mpz_t*** derF_Q; //[k_server][m_mapping][m_value + 1]
	mpz_t** derf_lambda; //[k_server][m_value + 1]
	mpz_t* tmp_derf_lambda;
	mpz_t* DataBase; //Database
	mpz_t* hash_cols; //Hash values of each column in the DataBase
	
	FILE *fp;
	fp = fopen("graphs/case1.txt", "a+"); // open file for reading and writing (append if file exists)
	//fp = fopen("graphs/case2.txt", "a+"); // open file for reading and writing (append if file exists)
	
	bool onetime = false;
	
	for (int k = 0; k < lengthk; k++)
	{
		printf ("\nThe value of k is %d \n", k_server[k]);
		fprintf(fp, "%d \n", k_server[k]); // write number k to file
		
		//t_private = k_server[k] -1; //case 4
		//int degree_d = d_mapping[k]*t_private; //case 4
		m_mapping = 1;
		//m_mapping = O(d*n^(1/d)) (e.g., m_mapping = 14, d = 4)
		unsigned long max_n = mChoosed(m_mapping, d_mapping[k]);
		
		F_Q = (mpz_t**) malloc(k_server[k] * sizeof(int*));
		derF_Q = (mpz_t***) malloc(k_server[k] * sizeof(int**));
		derf_lambda = (mpz_t**) malloc(k_server[k] * sizeof(int*));
		
	   	for (int n = 0; n < lengthn; n++)
	    	{
	    		printf ("\nThe value of n is %d \n\n", n_value[n]);
			fprintf(fp, "%d \n", n_value[n]); // write number n to file
			
	    		//Finding max_n >= n_value
			while (max_n < n_value[n])
			{
				m_mapping++;
				max_n = mChoosed(m_mapping, d_mapping[k]);
			}
			
			int m_original = m_mapping;
			
			printf ("m_mapping = %d \n", m_mapping);
			
			E_val = (int**) malloc(max_n * sizeof(int*));
			E_full = (int**) malloc(max_n * sizeof(int*));
			
			for (int i = 0; i < max_n; i++) 
			{
				E_val[i] = (int*) malloc(d_mapping[k] * sizeof(int));
				for (int j = 0; j < d_mapping[k]; j++)
				{
					E_val[i][j] = 0;
				}
				
				E_full[i] = (int*) malloc(m_mapping * sizeof(int));
				for (int j = 0; j < m_mapping; j++)
				{
					E_full[i][j] = 0;
				}
			}
			
			//Randomly picks V^1,..., V^t belonging {F^m}_p 
			mpz_t V_random[t_private][m_mapping];
			//Randomly picks lambda_1,..., lambda_t belonging F_p 
			mpz_t lambda_random[k_server[k]][t_private];
			//Queries
			mpz_t queries_Q[k_server[k]][m_mapping];
			
		    	for (int i = 0; i < k_server[k]; i++) 
			{
				derF_Q[i] = (mpz_t**) malloc(m_mapping * sizeof(int*));
			}
		    	
			i_index = rand()%n_value[n];
			
			blst_p2 Com; //commitment of x
			blst_p2 C[k_server[k]][m_mapping+1]; 
		    	blst_scalar y[k_server[k]][m_mapping+1]; //e.g., y1 = F1.x ; y2 = F2.x for 2 servers
		    	blst_p1 proof[k_server[k]][m_mapping+1]; //proofs
				
			blst_p2 *G = malloc(sizeof(blst_p2)*(n_value[n]+1));
		    	blst_p1 *H = malloc(sizeof(blst_p1)*(2*n_value[n]+1));
			blst_scalar *x = malloc(sizeof(blst_scalar)*(n_value[n]+1)); //address of an array of blst_scalar
			
			blst_scalar ***F; //[k_server][m_mapping + 1][(n_value+1)] coefficients of polynomials Fi
			
			F = (blst_scalar***) malloc(sizeof(blst_scalar**) *k_server[k]);
			
			for (int i = 0; i < k_server[k]; i++) 
			{
				F[i] = (blst_scalar**) malloc(sizeof(blst_scalar*)*(m_mapping+1));
			}
			
			for (int i = 0; i < k_server[k]; i++) 
			{	
				for ( int j = 0; j < (m_mapping+1); j++)
				{
					F[i][j] = (blst_scalar*) malloc(sizeof(blst_scalar)*(n_value[n]+1));
				}
			}
			
			printf ("-----------------START-------------- \n");
			printf ("              Desired x_%d \n\n", i_index);
			
			//1. Setup WY
			//1.0 Client generates all possible max_n distinct points of Hamming weight d
			start = clock();
			functionE(m_mapping, d_mapping[k], 1, 0, 1, E_val, d_mapping[k], max_n);
		   	functionE_full(m_mapping, d_mapping[k], 1, 0, 1, E_full, d_mapping[k], max_n); 
		    	stop = clock();
		    	printf("* WY: E mapping function: Generated %ld distinct points (%d-bits) with Hamming weight of %d = %lf seconds\n\n", max_n, m_mapping, d_mapping[k], (double) (stop - start) / CLOCKS_PER_SEC);
		   	
		   	//1.1 Client generates random points V belonging {F^m}_p 
		   	start = clock();
		   	Random_V(t_private, m_mapping, V_random, p);
		   	stop = clock();
		   	printf("* WY: Client: Generated %d random points V = %lf seconds\n\n",t_private ,(double) (stop - start) / CLOCKS_PER_SEC);
		   	
		   	//1.2 Client generates random lambdas belonging F_p 
		   	start = clock();
		   	Random_Lambda(k_server[k], t_private, lambda_random, p);
		   	stop = clock();
		    	printf("* WY: Client: Generated %d random random lambdas = %lf seconds\n\n",k_server[k] ,(double) (stop - start) / CLOCKS_PER_SEC);
		   	
		   	for (int m = 0; m < lengthm; m++)
		   	{
		   		printf ("The value of m is %d\n", m_value[m]);
		   		fprintf(fp, "%d \n", m_value[m]); // write number m to file
		   		
		   		s_PIR_time = 0.0;
				c_PIR_time = 0.0;
				s_LMC_time = 0.0;
				c_LMC_time = 0.0;
				
				tmp_derf_lambda = (mpz_t *) malloc((m_value[m] + 1) * sizeof(mpz_t));
		    		mpz_t result[m_value[m] + 1];
		    		
		    		
				//2. Client generates Queries (Point Q) belonging {F^m}_p
			   	start = clock();
			   	QueryGen_Client(i_index, d_mapping[k], k_server[k], m_mapping, t_private, n_value[n], E_val, queries_Q, lambda_random, V_random, p);
			   	stop = clock();
			    	printf("* WY: Client: Generated %d queries = %lf seconds\n\n",k_server[k] ,(double) (stop - start) / CLOCKS_PER_SEC);
			   	
			   	c_PIR_time += (double) (stop - start) / CLOCKS_PER_SEC;
				
		    		//----------------------------------------------Step 3: Server-----------------------------------------------//
		    		//3.0. Setup LMC
				start = clock();
				setup(n_value[n], G, H);
				stop = clock();
				printf("\nLMC: Setup = %lf seconds\n\n", (double) (stop - start) / CLOCKS_PER_SEC);
				    	
		    		if (!onetime)
				{
			    		DataBase = (mpz_t *) malloc((m_value[m] * n_value[n]) * sizeof(mpz_t));
			    		//3.1 Server creates randomly each element in the database.
					start = clock();
					DataBase = DbGen_Server(m_value[m], n_value[n], p);
					stop = clock();
					printf ("DbGen time = %f seconds\n", (double) (stop - start) / CLOCKS_PER_SEC);
					
					hash_cols = (mpz_t *) malloc(n_value[n] * sizeof(mpz_t));
					//3.2 Server calculates hash values of n db's columns.
					start = clock();
			    		hash_cols = HashGen_Server(m_value[m], n_value[n], DataBase, p);
			    		stop = clock();
			    		printf ("HashGen time = %f seconds\n\n", (double) (stop - start) / CLOCKS_PER_SEC);
			    		
			    		if (lengthm == 1 && lengthn == 1) onetime = true;
			    	}
		    		
		    		//Stored all x_i in blst_scalar
		    		for (int o = 1; o <= n_value[n]; o++)
		    		{
					mpz_get_str(tmp, 10, hash_cols[o - 1]);
					byte *in = tmp;
					blst_scalar_from_be_bytes(&x[o], in, 32);
		    		}
		    		
		    		//3.3. Server commites to hashes values.
		    		start = clock();
		    		com(n_value[n], x, G, &Com);
		    		stop = clock();
		    		printf("LMC: Commitment time = %lf seconds\n\n", (double) (stop - start) / CLOCKS_PER_SEC);
		    		
		    		for (int i = 0; i < k_server[k]; i++)
		    		{
		    			for (int j = 0; j < m_mapping + 1; j++)
		    			{
		    				C[i][j] = Com;
		    			}
		    			
		    		}
		    		
		    		//----------------------------------------------Step 4: Server-----------------------------------------------//
			   	for (int i = 0; i < k_server[k]; i++)
				{
					derf_lambda[i] = (mpz_t*) malloc((m_value[m] + 1) * sizeof(mpz_t));
					id = 2*i;
					
					for (int l = 0; l < (m_value[m] + 1); l++)
					{
						mpz_init (derf_lambda[i][l]);
					}
					
					//4.1 Anwsers for the query - Each server calculates F(Q_i)
			   		start = clock();
			   		F_Q[i] = FunctionF_Server(n_value[n], m_value[m], m_mapping, d_mapping[k], queries_Q[i], DataBase, hash_cols, E_val, p);
			   		stop = clock();
			    		printf("* WY: Server %d: Calculated F(Q_%d) = %lf seconds\n\n", i, i, (double) (stop - start) / CLOCKS_PER_SEC);
			    		
			    		s_PIR_time += (double) (stop - start) / CLOCKS_PER_SEC;
			    		
			    		//coefficients of polynomials in Server i
			    		for (int iii = 1; iii <= n_value[n]; iii++)
					{	
						mpz_set_ui (coef, 1);
						//coefficients of polynomials in Server i as for F_Q
						for (int jjj = 0; jjj < m_mapping; jjj++)
						{
							//Multiplies all z_l at E(i)_l = 1
							if (E_val[iii - 1][jjj] == 1)
							{
								mpz_mul (coef, coef, queries_Q[i][jjj]);
							}
						}
						mpz_mod (coef, coef, p);
						mpz_get_str(tmp, 10, coef);
						byte *in = tmp;
						blst_scalar_from_be_bytes(&F[i][0][iii], in, 32);
						
						//coefficients of polynomials in Server i as for derivative F/z at Q
						for (int pos = 0; pos < m_mapping; pos ++)
						{
							mpz_set_ui (coef, 1);
							if (E_val[iii - 1][pos] == 1)
							{
								for (int jjj = 0; jjj < m_mapping; jjj++)
								{
									//Multiplies all z_l at E(i)_l = 1
									if (E_val[iii - 1][jjj] == 1 && jjj != pos)
									{
										mpz_mul (coef, coef, queries_Q[i][jjj]);
									}
								}

							}
							mpz_mod (coef, coef, p);
							mpz_get_str(tmp, 10, coef);
							in = tmp;
							blst_scalar_from_be_bytes(&F[i][pos + 1][iii], in, 32);
						}
					}
			    		
			    		
			    		//4.2. Computes y_i = F_i.x and send to the Client
			    		for (int l = 0; l < (m_mapping + 1); l++)
					{
						blst_fr Fi_r, x_r, yi_r, tmpi_r, sumi_r;
		    				blst_fr_from_scalar(&Fi_r, &F[i][l][1]);
		    				blst_fr_from_scalar(&x_r, &x[1]);
		    				blst_fr_mul(&yi_r, &Fi_r, &x_r);
		    			
		    				for (int o = 2; o <= n_value[n]; o++)
		    				{
							blst_fr_from_scalar(&Fi_r, &F[i][l][o]);
							blst_fr_from_scalar(&x_r, &x[o]);
							blst_fr_mul(&tmpi_r, &Fi_r, &x_r);
							blst_fr_add(&sumi_r, &yi_r, &tmpi_r);
							yi_r = sumi_r;
		    				}
		    				blst_scalar_from_fr(&y[i][l], &yi_r);
		    				
		    				//4.3. Sends proof_i to the Client as for F_Q
			    			start = clock();
			    			open(n_value[n], x, F[i][l], H, &proof[i][l]);
			    			stop = clock();
			    			printf("LMC: Proof[Server_%d][%d] time = %lf seconds\n\n", i, l, (double) (stop - start) / CLOCKS_PER_SEC);
			    			
			    			s_LMC_time += (double) (stop - start) / CLOCKS_PER_SEC;
					}

			   		time = 0.0;	
			   		for (int j = 0; j < m_mapping; j++)
			   		{
			   			//4.2 Anwsers for the query - Each server calculates Derivative of F with respect to z_pos at point Q
			   			start = clock();
			   			derF_Q[i][j] = DerivativeF_Server(n_value[n], m_value[m], m_mapping, j, queries_Q[i], DataBase, hash_cols, E_full, p);
			   			stop = clock();
			    			printf("* WY: Server %d: Calculated derF(Q_%d)_z%d = %lf seconds\n\n", i, i, j, (double) (stop - start) / CLOCKS_PER_SEC);
			    			
			    			s_PIR_time += (double) (stop - start) / CLOCKS_PER_SEC;

						//-----------------------------------------------Step 5. Client----------------------------------------------//
			    			//5.2. Client reconstructs the each value of f'(lambda_h)
			    			start = clock();
			    			tmp_derf_lambda = Re_der_f(i, j, t_private, k_server[k], m_value[m], m_mapping, derF_Q[i][j], V_random, lambda_random, p);
			    				
			    			for (int l = 0; l < (m_value[m] + 1); l++)
						{
							mpz_add (derf_lambda[i][l], derf_lambda[i][l], tmp_derf_lambda[l]);	
						}
						stop = clock();
			    			time += (double) (stop - start) / CLOCKS_PER_SEC;	
				   	}
				   	
				   	start = clock();
				   	for (int l = 0; l < (m_value[m] + 1); l++)
					{
						mpz_mod (derf_lambda[i][l], derf_lambda[i][l], p);
					}
					stop = clock();
					time += (double) (stop - start) / CLOCKS_PER_SEC;
				   	printf("* WY: Client: Reconstructs f'(lambda_%d) = %lf seconds\n\n", i, time);
				   	
				   	c_PIR_time += time;
				}
				
				//5.1. Verifies proofi before decoding
				for (int ii = 0; ii < k_server[k]; ii++)
				{
					for (int ll = 0; ll < (m_mapping + 1); ll++)
					{
						start = clock();
				    		assert(verify(n_value[n], F[ii][ll], &y[ii][ll], G, H, &C[ii][ll], &proof[ii][ll]));
				    		stop = clock();
				    		printf("LMC: Verify y_[Server_%d][%d] time = %lf seconds\n\n", ii, ll, (double) (stop - start) / CLOCKS_PER_SEC);
				    		
				    		c_LMC_time += (double) (stop - start) / CLOCKS_PER_SEC;
					}
		    		}
			   	
			   	mpz_t matrix[degree_d + 1][degree_d + 1];
		    		
		    		for (int i = 0; i < degree_d + 1; i++)
		    		{
		    			for (int j = 0; j < degree_d + 1; j++)
		    			{
		    				mpz_init (matrix[i][j]);
		    			}
		    		}
		    		
			   	//5.2. Client starts to reconstruct x_i
			   	start = clock();
			   	//Creates Matrix D
				id = 0; //Server_ID
				for (int ii = 0; ii < (degree_d + 1); ii++)
			   	{
			   		if (ii % 2 == 0)
			   		{
			   			mpz_set_ui(matrix[ii][0], 1);
			   			mpz_set (matrix[ii][1], lambda_random[id][0]);
			   			id++;
			   		}
			   		else
			   		{
			   			mpz_set_ui(matrix[ii][0], 0);
			   			mpz_set_ui(matrix[ii][1], 1);
			   		}
			   		
			   		for (int jj = 2; jj < (degree_d + 1); jj++)
			   		{
			   			if (ii % 2 == 0)
			   			{
			   				mpz_mul (matrix[ii][jj], matrix[ii][1], matrix[ii][jj - 1]);
							mpz_mod (matrix[ii][jj], matrix[ii][jj], p);
			   			}
			   			else
			   			{
			   				mpz_mul_ui (matrix[ii][jj], matrix[ii - 1][jj - 1], jj);
			   				mpz_mod (matrix[ii][jj], matrix[ii][jj], p);
			   			}
			   		}
			   	}
			   	//det(D)
			   	det = Determinant((degree_d + 1), matrix, (degree_d + 1));
			   	mpz_invert (det[0], det[0], p);
			   	
			   	//Solving the linear equations for (d + 1) variables
			   	id = 0;
			   	for (int j = 0; j < (m_value[m] + 1); j++)
			   	{
			   		for (int i = 0; i < k_server[k]; i++)
			   		{
			   			if (id < (degree_d + 1))
			   			{
			   				mpz_set (matrix[id][0], F_Q[i][j]);
			   				id++;
			   				if (id < (degree_d + 1))
			   				{
			   					mpz_set (matrix[id][0], derf_lambda[i][j]);
			   					id++;
			   				}
			   				else i = k_server[k];
			   			}
			   			else i = k_server[k];
			   			
			   		}
			   		id = 0;
					
			   		det_a = Determinant((degree_d + 1), matrix, (degree_d + 1));
			   		mpz_mul (rop, det_a[0], det[0]);
			   		mpz_mod (rop, rop, p);
			   		mpz_init_set(result[j], rop);
			   		mpz_clear(det_a[0]);
			   		free(det_a);
			   	}
			   	stop = clock();
			   	printf ("* WY: Client: Recontructed x_i = %f seconds\n\n", (double) (stop - start) / CLOCKS_PER_SEC);
			   	
			   	c_PIR_time += (double) (stop - start) / CLOCKS_PER_SEC;
			   	
			   	//5.4. Verifies the hash values of db_i
				start = clock();
				bool verifyPIR = Verify_Client (m_value[m], p, result);
				stop = clock();
		    		printf ("* WY: Client: Verified hash values time = %f seconds\n\n", (double) (stop - start) / CLOCKS_PER_SEC);
		    		
		    		c_LMC_time += (double) (stop - start) / CLOCKS_PER_SEC;
					
				if (verifyPIR)
				{
					printf ("***** PASSED! Client received correct x_%d \n\n", i_index);
				}
				else
				{
					printf ("***** FAILED! Client received uncorrect x_%d \n\n", i_index);
				}
				
				fprintf(fp, "%f \n", s_PIR_time/k_server[k]); // write number n to file
				fprintf(fp, "%f \n", c_PIR_time); // write number n to file
				fprintf(fp, "%f \n", s_LMC_time/k_server[k]); // write number n to file
				fprintf(fp, "%f \n", c_LMC_time); // write number n to file
		   	}
		}
	}
	
	fclose(fp); // close file
	
    	return 0;
}


















