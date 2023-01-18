#include <gmp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

//Create Vandermonde matrix
mpz_t** vandermonde(int n, mpz_t p) 
{
	mpz_t rop;
	mpz_init (rop);
	mpz_t** A;
	A = (mpz_t**) malloc(n * sizeof(mpz_t*));
	
	for (int i = 0; i < n; i++) 
	{
		A[i] = (mpz_t*) malloc(n * sizeof(mpz_t));
		
	}

	for (int i = 0; i < n; i++) 
	{
		for (int j = 0; j < n; j++) 
		{
			mpz_set_ui (rop, (double) pow(i + 1, j));
			mpz_mod (A[i][j], rop, p); // compute the i-th row of the matrix
		}
	}
	
	return A;
}

//A simple method for finding the inverse matrix of Vandermonde matrix
mpz_t* S(int k, int j, int m, unsigned long max_n, long** E_val)
{
	mpz_t* S; 
	S = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init (S[0]);
	mpz_t product;
	mpz_init (product);
	int c;
	unsigned long list[m - 1];
	
	if (k == 0)
	{
		mpz_set_ui (S[0], 1);
		return S;
	}
	else if (k < 0 || k > m)
	{
		mpz_set_ui (S[0], 0);
		return S;
	}
	
	if (j == 1)
	{
		c = 2;
	}
	else
	{
		c = 1;
	}
	
	for (long i = 0; i < (m - 1); i++)
	{
		list[i] = c;
		c++;
		if (c == j)
		{
			c++;
		}
	}
	
	for (unsigned long i = 0; i < max_n; i++)
	{
		mpz_set_ui (product, 1);
		for (long j = 0; j < (m - 1); j++)
		{
			//printf("E = %d ", E_val[i][j]);
			if (E_val[i][j] == 1)
			{
				mpz_mul_ui (product, product, list[j]);
				//printf("product = %f ",product);
			}
		}
		mpz_add (S[0], S[0], product);
	}
	
	return S;
}

//A simple method for finding the inverse matrix of Vandermonde matrix
mpz_t* P(int j, int m)
{
	int c;
	mpz_t* P; 
	mpz_t product;
	P = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init (P[0]);
	mpz_init (product);

	c = j;
	mpz_set_ui (product, 1);
	for (int i = 1; i <= m; i++)
	{
		if (i == j)
			i++;
		mpz_mul_ui (product, product, abs(c - i));
		//printf("product = %f \n",product);
	}
	
	mpz_set (P[0], product);
	
	return P;
}

//Finding inverse of the Vandermonde matrix (V_inv)
mpz_t** vandermonde_inverse(int k, mpz_t p, long*** E_val)
{
	mpz_t rop;
	mpz_t temp;
	mpz_t* Sa;
	mpz_t* Pa;
	mpz_t** V_inv;
	unsigned long max_n;
	
	mpz_init (rop);
	
	V_inv = (mpz_t**) malloc(k * sizeof(int*));
   	
   	for (int i = 0; i < k; i++) 
   	{
        	V_inv[i] = (mpz_t*) malloc(k * sizeof(mpz_t));

    	}
    	
    	for (int j = 1; j <= k; j++) 
	{
		Pa = P(j , k);
		mpz_t Pa_inv;
		mpz_init_set (Pa_inv, Pa[0]);
		mpz_set (rop, Pa[0]);
		mpz_invert (Pa_inv, rop, p);
			
		for (int i = 1; i <= k; i++) 
		{
			max_n = mChoosed(k - 1, (k - i));
			Sa = S((k - i) ,j , k, max_n, E_val[k - i - 1]);
			mpz_set_si (rop, pow(-1, (i+j)));
			mpz_mod (rop, rop, p);
			mpz_mul (rop, rop, Sa[0]);
			mpz_mul (rop, rop, Pa_inv);
			mpz_mod (V_inv[i - 1] [j - 1], rop, p);
		}
	}
	
	return V_inv;
}

//Generate all possible distinct points of of a fixed length n with the Hamming weight d
long** generate(unsigned long n, unsigned long d, unsigned long max_n)
{
	long** E_val;
	
	E_val = (long**) malloc(max_n * sizeof(long*));
	
	for (long i = 0; i < max_n; i++) 
	{
		E_val[i] = (long*) malloc(n * sizeof(long));
	}
    
	long ii = 0, jj = 0;
	
    	for (long i = 0; i < (1 << n); i++)
    	{
        	if (__builtin_popcount(i) == d)
        	{
           	 	for (long j = n - 1; j >= 0; j--)
           	 	{
           	 		if((i >> j) & 1 == 1)
           	 		{
           	 			E_val[ii][jj] = 1;
           	 		}
           	 		else
           	 		{
           	 			E_val[ii][jj] = 0;
           	 		}
           	 		jj++;
           	 	}
           	 	ii++;
           	 	jj = 0;
        	}	
        }
        
        return E_val;
}

//Clients creates random queries (binary vectors size alpha*n)
mpz_t** QueriesGen_Client (int i_index, int n, int alpha, int t_private, mpz_t** V, mpz_t p)
{
	mpz_t** queries;
	mpz_t** M;
	gmp_randstate_t state;
	gmp_randinit_mt (state);
	
	int size_V = t_private + alpha;
	
	M = (mpz_t**) malloc(size_V * sizeof(int*));
	queries = (mpz_t**) malloc(size_V * sizeof(int*));
	
	for (int i = 0; i < size_V; i++)
	{
		M[i] = (mpz_t*) malloc((n * alpha) * sizeof(mpz_t));
		queries[i] = (mpz_t*) malloc((n * alpha) * sizeof(mpz_t));
		for (int j = 0; j < (n * alpha); j++)
		{
			mpz_init (M[i][j]);
			mpz_init (queries[i][j]);
		}
	}
	
	//Set e_i = 00...1...0
	for (int i = 0; i < alpha; i++)
	{
		mpz_set_ui (M[i][(i*n + i_index)], 1);
	}
	
	//Set random r_i
	for (int i = alpha; i < size_V; i++)
	{
		for (int j = 0; j < (n * alpha); j++)
		{
			mpz_set_ui (M[i][j], rand()%2);
		}
	}
	
	multiply(V, M, queries, size_V, size_V, (n * alpha), p);
	
	return queries;
}

//Split Database size m*n to new database size m*alpha*n transpose
mpz_t** SplitDatabase(int m, int n, int alpha, mpz_t* DataBase, mpz_t* hash_cols, mpz_t p)
{
	mpz_t** NewDB_T; //Database after splitting transpose
	mpz_t rop;
	mpz_t temp;
	mpz_init (rop);
	mpz_init (temp);
	
	NewDB_T = (mpz_t **) malloc((alpha * n) * sizeof(mpz_t*));
	for (long i = 0; i < (alpha * n); i++) 
	{
		NewDB_T[i] = (mpz_t*) malloc((m + 1) * sizeof(mpz_t));
	}
	
	mpz_set_ui (temp, alpha);
	mpz_invert (temp, temp, p);
	
	for (int j = 0; j < m; j++)
	{
		for (int i = 0; i < n; i++)
		{
			mpz_mul (rop, temp, DataBase[i * m + j]);
			mpz_mod (rop, rop, p);
				
			for (int o = 0; o < alpha; o++)
			{
				mpz_set (NewDB_T[o*n + i][j], rop);
			}
		}		
	}
		
	for (int i = 0; i < n; i++)
	{	
		mpz_mul (rop, temp, hash_cols[i]);
		mpz_mod (rop, rop, p);
			
		for (int o = 0; o < alpha; o++)
		{
			mpz_set (NewDB_T[o*n + i][m], rop);
		}
	}
	
	return NewDB_T;
}

//After received all anwsers from all servers, the client will extract to get x_i and h(x_i) values
void Extract_Client (int alpha, int m, mpz_t** Reconstruct, mpz_t result[m + 1], mpz_t p)
{	
	for (int i = 0 ; i < alpha; i++)
	{
		for (int j = 0; j < (m + 1); j++)
		{
			mpz_add (result[j], result[j], Reconstruct[i][j]);
		}
	}
		
	for (int i = 0; i < (m + 1); i++)
	{
		mpz_mod (result[i], result[i], p);
	}
}
