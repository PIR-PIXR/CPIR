#include <gmp.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "blst/include/blst.h"
#include "LMC.h"
#include "utils.h"

#define SIZE 255 //Each element's size is 256 bits
#define DIGITS 77 //Number of digits in an element

//Client creates random queries (binary vectors size n) (position = i_index)
mpz_t** QueriesGen_Client (int position, int n, int k, mpz_t p)
{
	mpz_t tmp;
	mpz_init (tmp);
	gmp_randstate_t state;
	gmp_randinit_mt (state);

	mpz_t** queries;
	queries = (mpz_t**) malloc(k * sizeof(int*));
	
	mpz_t* q_sum;
	q_sum = (mpz_t*) malloc(n * sizeof(mpz_t));
	
	for (int i = 0; i < n; i++)
	{
		mpz_init (q_sum[i]);
	}
	
	//Create randomly (k - 1) queries
	for (int i = 0; i < (k - 1); i++)
	{
		queries[i] = (mpz_t*) malloc(n * sizeof(mpz_t));
		for (int j = 0; j < n; j++)
		{
			mpz_init (queries[i][j]);
			mpz_urandomb (tmp, state, SIZE);
			mpz_set (queries[i][j], tmp);
			mpz_add (q_sum[j], q_sum[j], queries[i][j]);
		}
	}
	
	for (int i = 0; i < n; i++)
	{
		mpz_mod (q_sum[i], q_sum[i], p);
	}
	
	queries[k - 1] = (mpz_t*) malloc(n * sizeof(mpz_t));
	for (int j = 0; j < n; j++)
	{	
		mpz_init (queries[k - 1][j]);
		if (j == position)
		{
			mpz_ui_sub (queries[k - 1][j], 1, q_sum[j]);
		}
		else
		{
			mpz_ui_sub (queries[k - 1][j], 0, q_sum[j]);
		}
		mpz_mod (queries[k - 1][j], queries[k - 1][j], p);
	}
			
	return queries;
}

//The server will respond the anwser when received query
mpz_t** AnwGen_Server (int m, int n, mpz_t** queries, mpz_t** DataBase_T, int k, mpz_t p)
{			
	mpz_t** answers; //[k][m_value[j] + 1]
	answers = (mpz_t**) malloc(k * sizeof(int*));
	
	for (int i = 0; i < k; i++)
	{
		answers[i] = (mpz_t*) malloc((m + 1) * sizeof(mpz_t));
		for (int j = 0; j < (m + 1); j++)
		{
			mpz_init (answers[i][j]);
		}
	}
	
	multiply(queries, DataBase_T, answers, k, n, m + 1, p);
	
	return answers;
}

//After received all anwsers from two servers, the client will decode to get x_i and h(x_i) values
void Extract_Client (int m, int k, mpz_t p, mpz_t** answers, mpz_t result[m + 1])
{	
	for (int i = 0; i < m + 1; i++)
	{
		mpz_init (result[i]);
	}
		
	for (int i = 0; i < k; i++)
	{
		for (int j = 0 ; j < m + 1; j ++)
		{
			mpz_add (result[j], result[j], answers[i][j]);
		}
	}
	
	for (int i = 0; i < m + 1; i++)
	{
		mpz_mod (result[i], result[i], p);
	}
}

//Server computes y_i = F_i.x and send proof_i to the Client
double WitnessGen(int n_value, int k_server, mpz_t** queries, blst_scalar *x, blst_scalar **F, blst_p1 *H, blst_p1 proof[k_server], blst_scalar y[k_server], int i, clock_t start, clock_t stop)
{
	char tmp[DIGITS];
	double s_time = 0.0;
	
	for (int jj = 1; jj <= n_value; jj++)
	{
		mpz_get_str(tmp, 10, queries[i][jj - 1]);
		byte *in = tmp;
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
