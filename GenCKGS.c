#include <gmp.h>
#include <stdlib.h>
#include "utils.h"

#define SIZE 255 //Each element's size is 256 bits
#define DIGITS 77 //Number of digits in an element

//Clients creates random queries (binary vectors size n) (position = i_index)
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
