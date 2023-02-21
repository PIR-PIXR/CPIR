#include <gmp.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "blst/include/blst.h"
#include "LMC.h"
#include "utils.h"

#define SIZE 255 //Each element's size is 256 bits
#define DIGITS 77 //Number of digits in an element

int position = 0; //position of E mapping function from 0 to n_value

//Generate all possible distinct points of a fixed length m_mapping with the Hamming weight k
void functionE(int m_mapping, int k, int dir, int pos, int top, int** E_val, int d_mapping, unsigned long max_n) 
{
   	static char seq[2000];
    	if (top) memset(seq, 0, sizeof(seq));
    	for (int i = 1; i <= m_mapping - k + 1; i++, pos += dir) 
    	{
        	seq[pos] = '1';
        	if (k > 1) functionE(m_mapping - i, k - 1, i % 2 ? dir : -dir, pos + dir * (i % 2 ? 1 : m_mapping - i), 0, E_val, d_mapping, max_n);
        	else 
        	{
        		int count = 0;
        		for (int j = 0; j < sizeof(seq); j++)
        		{
        			if (seq[j] == '1') 
        			{
        				E_val[position][count] = j;
        				count++;
        			}
        			if (count == d_mapping) j = sizeof(seq);
        			
        		}
        		position++;
        		if (position == max_n) position = 0;
        	}
        	seq[pos] = '0';
    	}
    	if (top) memset(seq, 0, sizeof(seq));
}

void functionE_full(int m_mapping, int k, int dir, int pos, int top, int** E_full, int d_mapping, unsigned long max_n) 
{
   	static char seq[2000];
    	if (top) memset(seq, 0, sizeof(seq));
    	for (int i = 1; i <= m_mapping - k + 1; i++, pos += dir) 
    	{
        	seq[pos] = '1';
        	if (k > 1) functionE_full(m_mapping - i, k - 1, i % 2 ? dir : -dir, pos + dir * (i % 2 ? 1 : m_mapping - i), 0, E_full, d_mapping, max_n);
        	else 
        	{
        		int count = 0;
        		for (int j = 0; j < sizeof(seq); j++)
        		{
        			if (seq[j] == '1') 
        			{
        				E_full[position][j] = 1;
        				count++;
        			}
        			if (count == d_mapping) j = sizeof(seq);
        			
        		}
        		position++;
        		if (position == max_n) position = 0;
        	}
        	seq[pos] = '0';
    	}
    	if (top) memset(seq, 0, sizeof(seq));
}

//Client generates random points V
void Random_V(int size_n, int size_m, mpz_t random[size_n][size_m], mpz_t p)
{
	mpz_t tmp;
	mpz_init (tmp);
	gmp_randstate_t state;
	gmp_randinit_mt (state);
	
	for (int i = 0; i < size_n; i++)
   	{
   		for (int j = 0; j < size_m; j++)
   		{
   			mpz_init(random[i][j]);
   			//Randomly create an element in Z_p
			mpz_urandomb (tmp, state, SIZE);
			mpz_mod (random[i][j], tmp, p);
   		}
   	}
}

//Client generates random lambdas lambda, lambda^2, ..., lambda^t
void Random_Lambda(int size_n, int size_m, mpz_t random[size_n][size_m], mpz_t p)
{
	mpz_t tmp;
	mpz_init (tmp);
	gmp_randstate_t state;
	gmp_randinit_mt (state);
	
	for (int i = 0; i < size_n; i++)
   	{
   		mpz_init(random[i][0]);
   		//Randomly create an element in Z_p
		mpz_urandomb (tmp, state, SIZE);
		mpz_mod (random[i][0], tmp, p);
		
   		for (int j = 1; j < size_m; j++)
   		{
   			mpz_init(random[i][j]);
			mpz_mul(random[i][j], random[i][0], random[i][j - 1]);
			mpz_mod (random[i][j], random[i][j], p);
   		}
   	}
}

//Client generates Queries (Point Q) belonging {F^m}_p 
void QueryGen_Client(int i_index,int d_mapping, int k_server, int m_mapping, int t_private, int n_value, int** E_val, mpz_t queries_Q[k_server][m_mapping], mpz_t lambda_random[k_server][t_private], mpz_t V_random[t_private][m_mapping],  mpz_t p)
{
	mpz_t rop;
	mpz_init (rop);
	
	
	for (int i = 0; i < k_server; i++)
   	{
   		int count = 0;
   		for (int j = 0; j < m_mapping; j++)
   		{
   			mpz_init(queries_Q[i][j]);
   			
   			for (int t = 0; t < t_private; t++)
   			{
   				mpz_addmul (rop, lambda_random[i][t], V_random[t][j]);
   			}
   			
   			if (count < d_mapping)
   			{
   				if (j == E_val[i_index][count]) 
	   			{
	   				mpz_add_ui (rop, rop, 1);
	   				count++;
	   			}
   			}
   			
   			mpz_mod (queries_Q[i][j], rop, p);
   			mpz_init (rop);
   		}
   	}
}

//Calculates Function F(Q)
mpz_t *FunctionF_Server(int n_value, int m_value, int m_mapping, int d_mapping, mpz_t query_Q[m_mapping], mpz_t *DataBase, mpz_t *hash_cols, int** E_val, mpz_t p)
{
	mpz_t tmp;
	mpz_init (tmp);
	mpz_t *F_Q;
	F_Q = (mpz_t *) malloc((m_value + 1) * sizeof(mpz_t));
	
	for (int i = 0; i < (m_value + 1); i++)
	{
		mpz_init (F_Q[i]);
	}
	
	for (int i = 0; i < n_value; i++)
	{	
		mpz_init_set_ui (tmp, 1);
		//Multiplies all z_l at E(i)_l = 1
		for (int j = 0; j < d_mapping; j++)
		{
			mpz_mul (tmp, tmp, query_Q[E_val[i][j]]);
		}
		
		int l = 0;
		for (l = 0; l < m_value; l++)
		{
			mpz_addmul (F_Q[l], DataBase[i * m_value + l], tmp);
		}
		mpz_addmul (F_Q[l], hash_cols[i], tmp);
	}
	
	for (int i = 0; i < (m_value + 1); i++)
	{
		mpz_mod (F_Q[i], F_Q[i], p);
	}
	
	return F_Q;
}

//Calculates Derivative of F with respect to z_pos at point Q_h
mpz_t *DerivativeF_Server(int n_value, int m_value, int m_mapping, int z_pos, mpz_t query_Q[m_mapping], mpz_t *DataBase, mpz_t *hash_cols, int** E_val, mpz_t p)
{
	mpz_t tmp;
	mpz_init (tmp);
	mpz_t *derF_Q;
	derF_Q = (mpz_t *) malloc((m_value + 1) * sizeof(mpz_t));
		
	for (int i = 0; i < (m_value + 1); i++)
	{
		mpz_init (derF_Q[i]);
	}
	
	for (int i = 0; i < n_value; i++)
	{	
		mpz_init_set_ui (tmp, 1);
		
		if (E_val[i][z_pos] == 1)
		{
			for (int j = 0; j < m_mapping; j++)
			{
				//Multiplies all z_l at E(i)_l = 1
				if (E_val[i][j] == 1 && j != z_pos)
				{
					mpz_mul (tmp, tmp, query_Q[j]);
				}
			}
			
			int l = 0;
			for (l = 0; l < m_value; l++)
			{
				mpz_addmul (derF_Q[l], DataBase[i * m_value + l], tmp);
			}
			mpz_addmul (derF_Q[l], hash_cols[i], tmp);
		}
	}
	
	for (int i = 0; i < (m_value + 1); i++)
	{
		mpz_mod (derF_Q[i], derF_Q[i], p);
	}
	
	return derF_Q;
}

//Client reconstructs the value of f'(lambda_h)
mpz_t *Re_der_f(int server_ID, int l_pos, int t_private, int k_server, int m_value, int m_mapping, mpz_t *derF_Q, mpz_t V_random[t_private][m_mapping], mpz_t lambda_random[k_server][t_private], mpz_t p)
{
	mpz_t *der_f_l;
	mpz_t tmp;
	mpz_t tmp_lambda;
	mpz_init (tmp_lambda);
	
	der_f_l = (mpz_t *) malloc((m_value + 1) * sizeof(mpz_t));
	
	for (int i = 0; i < (m_value + 1); i++)
	{
		mpz_init (der_f_l[i]);
		mpz_init_set (tmp, V_random[0][l_pos]);
		
		for (int j = 1; j < t_private; j++)
		{
			mpz_mul_ui (tmp_lambda, lambda_random[server_ID][j - 1], (j + 1));
			//Set tmp to tmp + lambda × V
			mpz_addmul (tmp, tmp_lambda, V_random[j][l_pos]);
		}
	
		mpz_mul (der_f_l[i], derF_Q[i], tmp);
	}

	return 	der_f_l;
}


//coefficients of polynomials in Server i for LMC
void coefficients(mpz_t coef, int n_value, int** E_val, int k, int m_mapping, mpz_t queries_Q[k][m_mapping], char tmp[DIGITS], mpz_t p, blst_scalar **F, int i)
{       
	for (int iii = 1; iii <= n_value; iii++)
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
		//blst_scalar_from_be_bytes(&F[i][0][iii], in, 32);
		blst_scalar_from_be_bytes(F[i] + iii, in, 32);
						
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
			//blst_scalar_from_be_bytes(&F[i][pos + 1][iii], in, 32);
			blst_scalar_from_be_bytes(F[i] + (pos + 1)*(n_value+1) + iii, in, 32);
		}
	}
}

//Server computes y_i = F_i.x and send proof_i to the Client
double WitnessGen(int m_mapping, int k_server, int n_value, blst_scalar *x, blst_scalar **F, blst_p1 *H, blst_p1 proof[k_server], blst_scalar y[m_mapping + 1], int i, clock_t start, clock_t stop)
{
	double s_time = 0.0;
    
	for (int l = 0; l < (m_mapping + 1); l++)
	{
		blst_fr Fi_r, x_r, yi_r, tmpi_r, sumi_r;
		//blst_fr_from_scalar(&Fi_r, &F[i][l][1]);
		blst_fr_from_scalar(&Fi_r, F[i] + l*(n_value+1) + 1);
		blst_fr_from_scalar(&x_r, x + 1);
		blst_fr_mul(&yi_r, &Fi_r, &x_r);
		    			
		for (int o = 2; o <= n_value; o++)
		{
			blst_fr_from_scalar(&Fi_r, F[i] + l*(n_value+1) + o);
			blst_fr_from_scalar(&x_r, x + o);
			blst_fr_mul(&tmpi_r, &Fi_r, &x_r);
			blst_fr_add(&sumi_r, &yi_r, &tmpi_r);
			yi_r = sumi_r;
		}
		blst_scalar_from_fr(y + l, &yi_r);
	}
	
	//4.3. Sends proof_i to the Client as for F_Q
	start = clock();
	open(n_value, m_mapping + 1, x, F[i], H, &proof[i]);
	stop = clock();
	printf("LMC: Proof[Server_%d] time = %lf seconds\n\n", i, (double) (stop - start) / CLOCKS_PER_SEC);
			    			
	s_time += (double) (stop - start) / CLOCKS_PER_SEC;
	
	return s_time;
}

//Client starts to reconstruct x_i
void Reconstruct_Client(int degree_d, int k_server, int t_private, int m_value, mpz_t lambda_random[k_server][t_private], mpz_t result[m_value + 1], mpz_t p, mpz_t** F_Q, mpz_t** derf_lambda)
{
	mpz_t* det;
	mpz_t* det_a;
	mpz_t rop;
	mpz_init (rop);
	mpz_t matrix[degree_d + 1][degree_d + 1];
		    		
	for (int i = 0; i < degree_d + 1; i++)
	{
		for (int j = 0; j < degree_d + 1; j++)
		{
		    	mpz_init (matrix[i][j]);
		}
	}
		    		
	//Creates Matrix D
	int id = 0; //Server_ID
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
	for (int j = 0; j < (m_value + 1); j++)
	{
		for (int i = 0; i < k_server; i++)
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
				else i = k_server;
			}
			else i = k_server;	   			
		}
		id = 0;
					
		det_a = Determinant((degree_d + 1), matrix, (degree_d + 1));
		mpz_mul (rop, det_a[0], det[0]);
		mpz_mod (rop, rop, p);
		mpz_init_set(result[j], rop);
		mpz_clear(det_a[0]);
		free(det_a);
	}
}
