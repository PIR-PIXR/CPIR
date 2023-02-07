#include <gmp.h>
#include <math.h>
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
#include "BE.h"

#define DIGITS 77 //Number of digits in an element
#define R "73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001" //R is prime number in BLS12-381. It is also the prime number p in Z_p

int main() 
{
	/*//case1: k = 2, t = 1, n = 2^10: running m (1, 1024, 4096, 16384, 65536)
    	//n_value is the number of columns in the database
	int n_value[] = {1024};
	//m_value is the number of rows in the database
	int m_value[] = {1, 1024, 4096, 16384, 65536};
	//k is the number of servers
	int k_server[] = {2}; //k = size_V
	int t_private = 1;*/
	
	/*//case2: k = 2, t = 1, m = 2^10: running n (256, 1024, 4096, 16384, 65536)
	//n_value is the number of columns in the database
	int n_value[] = {256, 1024, 4096};
	//m_value is the number of rows in the database
	int m_value[] = {1024};
	//k is the number of servers
	int k_server[] = {2}; //k = size_V
	int t_private = 1;*/
	
	//case4 or 5: t = k - 1 or 1, n = 2^10, m = 64 running k (3, 4, 5)
	//n_value is the number of columns in the database
	int n_value[] = {1024};
	//m_value is the number of rows in the database
	int m_value[] = {4096};
	//k is the number of servers
	int k_server[] = {3, 4, 5, 6}; //k = size_V
	int t_private; //t = k - 1
	
	int alpha; //alpha = k - t_private
	int i_index;
	int lengthm = sizeof(m_value)/sizeof(m_value[0]);
	int lengthn = sizeof(n_value)/sizeof(n_value[0]);
	int lengthk = sizeof(k_server)/sizeof(k_server[0]);
	long b; //number of bits
	long*** E_val; //[k][max_n][m_mapping]
	unsigned long max_n;
	double time; //the time counters
	char tmp[DIGITS];
	bool verifyPIR;
	bool onetime = false;
	clock_t start, stop;
	double s_PIR_time, c_PIR_time, s_LMC_time, c_LMC_time;
	
	FILE *fp;
	//fp = fopen("graphs/case1.txt", "a+"); // open file for reading and writing (append if file exists)
	//fp = fopen("graphs/case2.txt", "a+"); // open file for reading and writing (append if file exists)
	fp = fopen("graphs/case4.txt", "a+"); // open file for reading and writing (append if file exists)
	
	//initialization of a prime number p
	mpz_t p;
	mpz_init_set_str (p, R, 16);
	gmp_printf ("p = %Zd \n", p);
	mpz_t rop;
	mpz_t temp;
	mpz_t coef;
	mpz_init (rop);
	mpz_init (temp);
	mpz_init (coef);
	mpz_t* Sa;
	mpz_t* Pa;
	mpz_t** V_inv;
	mpz_t** V;
	mpz_t** queries;
	mpz_t* DataBase; //Database
	mpz_t** NewDB_T; //Database after splitting transpose
	mpz_t* hash_cols; //Hash values of each column in the DataBase
	mpz_t** Answers;
	mpz_t** Reconstruct;
	
	for (int k = 0; k < lengthk; k++)
	{
		//t_private = k_server[k] -1; //case 4
		t_private = 1; //case 5
		alpha = k_server[k] - t_private;
		printf ("\nThe value of k is %d \n", k_server[k]);
		fprintf(fp, "%d \n", k_server[k]); // write number k to file

		//Allocate memory
		E_val = (long***) malloc(k_server[k] * sizeof(long**));
		Answers = (mpz_t **) malloc(k_server[k] * sizeof(int*));
	    	Reconstruct = (mpz_t **) malloc(k_server[k] * sizeof(int*));
	    	
	    	blst_p2 Com; //commitment of x
		blst_p2 C[k_server[k]]; 
	    	blst_scalar y[k_server[k]]; //e.g., y1 = F1.x ; y2 = F2.x for 2 servers
	    	blst_p1 proof[k_server[k]]; //proofs
    	
	    	for (int n = 0; n < lengthn; n++)
	    	{
	    		fprintf(fp, "%d \n", n_value[n]); // write number n to file
	    		i_index = rand()%n_value[n];
	    		
	    		blst_p2 *G = malloc(sizeof(blst_p2) * (alpha * n_value[n] + 1));
		    	blst_p1 *H = malloc(sizeof(blst_p1) * (2 * alpha * n_value[n] + 1));
			blst_scalar *x = malloc(sizeof(blst_scalar) * (alpha * n_value[n] + 1)); //address of an array of blst_scalar
			blst_scalar **F; //[k_server][(alpha * n_value[n] + 1)] coefficients of polynomials Fi
			
			F = (blst_scalar**) malloc(sizeof(blst_scalar*) *k_server[k]);
			
			for (int i = 0; i < k_server[k]; i++) 
			{
				F[i] = (blst_scalar*) malloc(sizeof(blst_scalar)*(alpha * n_value[n] + 1));
			}
			
			//0. Setup LMC
		    	start = clock();
		    	setup(alpha*n_value[n], G, H);
		    	stop = clock();
		    	printf("\nLMC: Setup = %lf seconds\n\n", (double) (stop - start) / CLOCKS_PER_SEC);
	    		
		    	//Loop for all size of m_values
		    	for (int m = 0; m < lengthm; m++)
		   	{
		   		s_PIR_time = 0.0;
				c_PIR_time = 0.0;
				s_LMC_time = 0.0;
				c_LMC_time = 0.0;
				
		   		mpz_t result[m_value[m] + 1];
		   		
				for (int i = 0; i < (m_value[m] + 1); i++)
				{
					mpz_init (result[i]);
				}
				
		   		for (int i = 0; i < k_server[k]; i++) 
			   	{
			   		Answers[i] = (mpz_t*) malloc((m_value[m] + 1) * sizeof(mpz_t));
			   		Reconstruct[i] = (mpz_t*) malloc((m_value[m] + 1) * sizeof(mpz_t));
			    	}
			    	
			    	printf ("-----------------START-------------- \n");
				printf ("              m = %d \n", m_value[m]);
				printf ("              n = %d \n", n_value[n]);
				printf ("              k = %d \n", k_server[k]);
				printf ("              t = %d \n", t_private);
				printf ("              alpha = %d \n", alpha);
				printf ("              Desired x_%d \n\n", i_index);
				
				fprintf(fp, "%d \n", m_value[m]); // write number m to file
				
				//----------------------------------------------Step 0: Client-----------------------------------------------//
				
				V = (mpz_t**) malloc(k_server[k] * sizeof(mpz_t*));
				V_inv = (mpz_t**) malloc(k_server[k] * sizeof(mpz_t*));
	
				for (int i = 0; i < k_server[k]; i++) 
				{
					V[i] = (mpz_t*) malloc(k_server[k] * sizeof(mpz_t));
					V_inv[i] = (mpz_t*) malloc(k_server[k] * sizeof(mpz_t));
				}
			    	
			    	//0.1 Client generates Vandermonde matrix
			    	V = vandermonde(k_server[k], p);
			    	
			    	//Generate all possible distinct points of Hamming weight b
			    	for (int i = 0; i < k_server[k] - 1; i++)
			    	{
			    		b = i + 1;
					max_n = mChoosed(k_server[k] - 1, b);
					E_val[i] = (long**) malloc(max_n * sizeof(long*)); 
					E_val[i] = generate(k_server[k] - 1, b, max_n);
			    	} 	
			    
			    	//0.2 Finding inverse of the Vandermonde matrix (V_inv)
			    	V_inv = vandermonde_inverse(k_server[k], p, E_val);
				
				//----------------------------------------------Step 1: Data Owner-----------------------------------------------//
				if (!onetime)
				{
					DataBase = (mpz_t *) malloc((m_value[m] * n_value[n]) * sizeof(mpz_t));
				    	//1.1 creates randomly each element in the database.
					start = clock();
					DataBase = DbGen_Server(m_value[m], n_value[n], p);
					stop = clock();
					printf ("DbGen time = %f seconds\n", (double) (stop - start) / CLOCKS_PER_SEC);
					
					hash_cols = (mpz_t *) malloc(n_value[n] * sizeof(mpz_t));
					//1.2 calculates hash values of n db's columns.
					start = clock();
				    	hash_cols = HashGen_Server(m_value[m], n_value[n], DataBase, p);
				    	stop = clock();
				    	printf ("HashGen time = %f seconds\n", (double) (stop - start) / CLOCKS_PER_SEC);
				    	
				    	if (lengthm == 1 && lengthn == 1) onetime = true;
				}
				
				NewDB_T = (mpz_t **) malloc((alpha * n_value[n]) * sizeof(long*));
				for (long i = 0; i < (alpha * n_value[n]); i++) 
				{
					NewDB_T[i] = (mpz_t*) malloc((m_value[m] + 1) * sizeof(mpz_t));
				}
				//1.3 splits database
				start = clock();
				NewDB_T = SplitDatabase(m_value[m], n_value[n], alpha, DataBase, hash_cols, p);
				stop = clock();
				printf ("NewDbGen time = %f seconds\n\n", (double) (stop - start) / CLOCKS_PER_SEC);
				    	
				//Stored all x_i in blst_scalar (hash values)
		    		for (int o = 1; o <= alpha*n_value[n]; o++)
		    		{
					mpz_get_str(tmp, 10, NewDB_T[o - 1][m_value[m]]);
					byte *in = tmp;
					blst_scalar_from_be_bytes(&x[o], in, 32);
		    		}
		    		
		    		//1.4. commites to hashes values.
		    		start = clock();
		    		com(alpha*n_value[n], x, G, &Com);
		    		stop = clock();
		    		printf("LMC: Commitment time = %lf seconds\n\n", (double) (stop - start) / CLOCKS_PER_SEC);

		    		for (int i = 0; i < k_server[k]; i++)
		    		{
		    			C[i] = Com;
		    		}
				
			    	//-----------------------------------------------Step 2. Client----------------------------------------------//
			    	
				//2.1. Client creates queries (size k*(alpha*n))
				start= clock();
				queries = QueriesGen_Client (i_index, n_value[n], alpha, t_private, V, p);
				stop = clock();  
			    	printf ("Staircase: QueryGen time = %f seconds\n", (double) (stop - start) / CLOCKS_PER_SEC);
			    	
			    	c_PIR_time += (double) (stop - start) / CLOCKS_PER_SEC;
			    	
			    	//-----------------------------------------------Step 3. Server----------------------------------------------//
				
				//3.1. Anwsers for the queries
				start= clock();
				multiply(queries, NewDB_T, Answers, k_server[k], (alpha * n_value[n]), (m_value[m] + 1), p);
				stop = clock();  
			    	printf ("Staircase: AnwGen time = %f seconds\n", (double) (stop - start) / CLOCKS_PER_SEC);
			    	
			    	s_PIR_time += (double) (stop - start) / CLOCKS_PER_SEC;
			    		
			    	
			    	//Server computes y_i = F_i.x and send proof_i to the Client
			    	for (int i = 0; i < k_server[k]; i++)
				{
			    		s_LMC_time += WitnessGen(n_value[n], k_server[k], alpha, queries, x, F, H, proof, y, i, start, stop);
				}
				printf("\n\n");

				//-----------------------------------------------Step 4. Client----------------------------------------------//
				//4.1. Verifies proofi before decoding
				for (int i = 0; i < k_server[k]; i++)
				{
					start = clock();
		    			assert(verify(alpha*n_value[n], F[i], &y[i], G, H, &C[i], &proof[i]));
		    			stop = clock();
		    			printf("LMC: Verify y_%d time = %lf seconds\n", i, (double) (stop - start) / CLOCKS_PER_SEC);
		    			
		    			c_LMC_time += (double) (stop - start) / CLOCKS_PER_SEC;
				}
				printf("\n");	
				
				//4.2 Extracts x_i and h_i
				start= clock();
				multiply(V_inv, Answers, Reconstruct, k_server[k], k_server[k], (m_value[m] + 1), p);
				Extract_Client (alpha, m_value[m], Reconstruct, result, p);
				stop = clock();  
			    	printf ("Staircase: Extract x_i time = %f seconds\n", (double) (stop - start) / CLOCKS_PER_SEC);
			    	
			    	c_PIR_time += (double) (stop - start) / CLOCKS_PER_SEC;
				
				//4.3 Verifies the hash values of db_i
				start = clock();
				verifyPIR = Verify_Client (m_value[m], p, result);
				stop = clock();
			    	printf ("Staircase: Verify hash values time = %f seconds\n\n", (double) (stop - start) / CLOCKS_PER_SEC);
			    	
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
				fprintf(fp, "%f \n", c_LMC_time); // write number n to file}
			}
		}
	}
	fclose(fp); // close file
	
	return 0;
}
