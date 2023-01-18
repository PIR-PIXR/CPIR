#include <gmp.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 255 //Each element's size is 256 bits

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
