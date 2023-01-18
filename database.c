#include <gmp.h>
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include "database.h"

#define SIZE 255 //Each element's size is 256 bits
#define DIGITS 77 //Number of digits in an element

//Server creates randomly each element in the database (m*n) and the hash values of n db's columns.
mpz_t *DbGen_Server(int m, int n, mpz_t p)
{
	mpz_t tmp;
	mpz_t *X_db;
	mpz_init (tmp);
    	gmp_randstate_t state;
	gmp_randinit_mt (state);
	X_db = (mpz_t *) malloc((m * n) * sizeof(mpz_t));
	
	for (int i = 0; i < n; i++)
	{	
		for (int j = 0; j < m; j++)
		{
			mpz_init (X_db[i * m + j]);
			
			do
			{
				//Randomly create elements in the DB
				mpz_urandomb (tmp, state, SIZE);
				mpz_mod (tmp, tmp, p);
			}
			while((int) mpz_sizeinbase (tmp, 10) != DIGITS);
			
			mpz_set (X_db[i * m + j], tmp);
		}		
	}
	
	return X_db;
}

//Server calculates hash values of n db's columns.
mpz_t *HashGen_Server(int m, int n, mpz_t *X_db, mpz_t p)
{
	mpz_t *hash_cols;
	EVP_MD_CTX *context;
	mpz_t digestmod;
    	const EVP_MD *md;
    	char sdigest[64];
    	char tmpstr[DIGITS];
    	unsigned int md_len;
	unsigned char md_value[EVP_MAX_MD_SIZE];
    	md = EVP_sha3_256(); //Hash function SHA3-256
    	hash_cols = (mpz_t *) malloc(n * sizeof(mpz_t));
	
	for (int i = 0; i < n; i++)
	{
		context = EVP_MD_CTX_new();
    		EVP_DigestInit_ex2(context, md, NULL);
		
		for (int j = 0; j < m; j++)
		{
			//Convert element to string
			mpz_get_str(tmpstr, 10, X_db[i * m + j]);
			EVP_DigestUpdate(context, tmpstr, strlen(tmpstr));
		}
		
		EVP_DigestFinal_ex(context, md_value, &md_len);
    		EVP_MD_CTX_free(context);
    		
    		//Convert md_value (digest) to string
    		for (int h = 0; h < md_len; h++)
    		{
        		sprintf(sdigest+(h*2), "%02x", md_value[h]); 
        	}
    		
		//modulo digest into Z_p
		mpz_init_set_str (digestmod, sdigest, 16);
		mpz_mod (hash_cols[i], digestmod, p);
	}
	return hash_cols;
}
