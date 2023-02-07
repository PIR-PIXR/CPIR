#include <gmp.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "blst/include/blst.h"
#include "LMC.h"

#define SIZE 255 //Each element's size is 256 bits
#define DIGITS 77 //Number of digits in an element

//Clients creates random queries (binary vectors size n) (position = i_index)
int** QueriesGen_Client (int position, int n)
{
	int** queries;
	queries = (int**) malloc(2 * sizeof(int*));
	
	for (int i = 0; i < 2; i++)
	{
		queries[i] = (int*) malloc(n * sizeof(int));
	}
	
	for (int j = 0; j < n; j++)
	{
		queries[0][j] = rand()%2;
		queries[1][j] = queries[0][j];
	}
	
	if (queries[1][position] == 0)
	{
		queries[1][position] = 1;
	}
	else
	{
		queries[1][position] = 0;
	}
	
	return queries;
}

//The Server will respond the anwsers when received query
mpz_t** AnwGen_Server (int m, int n, int** queries, mpz_t *X_db,  mpz_t *hash_cols, mpz_t p)
{			
	mpz_t** answers; //[2][m_value[j] + 1]
	
	answers = (mpz_t**) malloc(2 * sizeof(int*));
	
	for (int i = 0; i < 2; i++)
	{
		answers[i] = (mpz_t*) malloc((m + 1) * sizeof(mpz_t));
		for (int j = 0; j < (m + 1); j++)
		{
			mpz_init (answers[i][j]);
		}
	}
	
	for (int i = 0; i < 2; i++)
	{	
		//Add all columns responded to 1's positions in the query
		for (int j = 0; j < n; j++)
		{
			if (queries[i][j] == 1)
			{
				for (int k = 0; k < m; k++)
				{
					mpz_add (answers[i][k], answers[i][k], X_db[j * m + k]);
				}	
				mpz_add (answers[i][m], answers[i][m], hash_cols[j]);
			}
		}
	}
	
	//Modulo the results and send to the Client
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < m + 1; j++)
		{
			mpz_mod (answers[i][j], answers[i][j], p);
		}
	}
	
	return answers;
}

//After received all anwsers from two servers, the client will extract to get x_i and h(x_i) values
void Extract_Client (int m, int n, mpz_t p, int** queries, int position, mpz_t** answers, mpz_t result[m + 1])
{	
	for (int i = 0; i < m + 1; i++)
	{
		mpz_init (result[i]);
	}
		
	if (queries[0][position] == 1)
	{
		
		for (int j = 0; j < m + 1; j++)
		{
			mpz_sub (result[j], answers[0][j], answers[1][j]);
			mpz_mod (result[j], result[j], p);
		}
	}
	else
	{	
		for (int j = 0; j < m + 1; j++)
		{
			mpz_sub (result[j], answers[1][j], answers[0][j]);
			mpz_mod (result[j], result[j], p);
		}
	}
}

//Server computes y_i = F_i.x and send proof_i to the Client
double WitnessGen(int n_value, int k_server, int** queries, blst_scalar *x, blst_scalar **F, blst_p1 *H, blst_p1 proof[k_server], blst_scalar y[k_server], int i, clock_t start, clock_t stop)
{
	char tmp[DIGITS];
	double s_time = 0.0;
	
	for (int jj = 1; jj <= n_value; jj++)
	{
		char* buffer = (char*) (&queries[i][jj - 1]);
		byte *in = buffer;
		blst_scalar_from_be_bytes(&F[i][jj], in, 32);
	}
				
	//Computes y_i = F_i.x and send to the Client
	blst_fr Fi_r, x_r, yi_r, tmpi_r, sumi_r;
	blst_fr_from_scalar(&Fi_r, &F[i][1]);
	blst_fr_from_scalar(&x_r, &x[1]);
	blst_fr_mul(&yi_r, &Fi_r, &x_r);
				
	for (int o = 2; o <= n_value; o++)
	{
		blst_fr_from_scalar(&Fi_r, &F[i][o]);
		blst_fr_from_scalar(&x_r, &x[o]);
		blst_fr_mul(&tmpi_r, &Fi_r, &x_r);
		blst_fr_add(&sumi_r, &yi_r, &tmpi_r);
		yi_r = sumi_r;
	}
	blst_scalar_from_fr(&y[i], &yi_r);
	    			
	//Sends proof_i to the Client
	start= clock();
	open(n_value, x, F[i], H, &proof[i]);
	stop = clock();
	s_time = (double) (stop - start) / CLOCKS_PER_SEC;
	printf("\nLMC: Proof_%d time = %lf seconds", i, s_time);
	
	return s_time;
}
