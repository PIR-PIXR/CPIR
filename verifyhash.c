#include <gmp.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include "verifyhash.h"

#define SIZE 255 //Each element's size is 256 bits
#define DIGITS 77 //Number of digits in an element

//After decoded to get x_i, the Client will check by hash(x_i) and checked with decoded h(x_i) value
bool Verify_Client (int m, mpz_t p, mpz_t* result)
{
	char sd[64];
	char tmpstr[DIGITS];
	unsigned int sha_len;
    	unsigned char sha_value[EVP_MAX_MD_SIZE];
	
	mpz_t tmpHash;
	mpz_t digestmod;
	mpz_init (tmpHash);
	
    	const EVP_MD *sha;
    	EVP_MD_CTX *context;
	context = EVP_MD_CTX_new();
	sha = EVP_sha3_256();//Hash function SHA3-256
	
	EVP_DigestInit_ex2(context, sha, NULL);

	for (int i = 0; i < m; i++)
	{		
    		//Convert element to string
		mpz_get_str(tmpstr, 10, result[i]);
		EVP_DigestUpdate(context, tmpstr, strlen(tmpstr));
	}
	
	EVP_DigestFinal_ex(context, sha_value, &sha_len);
    	EVP_MD_CTX_free(context);
    		
    	//Convert sha_value (digest) to string
    	for (int i = 0; i < sha_len; i++)
    	{
        	sprintf(sd+(i*2), "%02x", sha_value[i]); 
        }
    
	//modulo digest into Z_p
	mpz_init_set_str (digestmod, sd, 16);
	mpz_mod (tmpHash, digestmod, p);
	
	//Verifies two hashes values
	if (mpz_cmp (tmpHash, result[m]) == 0)
	{		
		return true;
	}
	return false;
}
