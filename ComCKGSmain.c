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
#include "LMC.h"
#include "verifyhash.h"
#include "CKGS.h"

#define DIGITS 77 //Number of digits in an element
#define R "73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001" //R is prime number in BLS12-381. It is also the prime number p in Z_p

int main() {
	/*//case1: k = 2, t = 1, n = 2^10: running m (1, 1024, 4096, 16384, 65536)
    	//n_value is the number of columns in the database
	int n_value[] = {1024};
	//m_value is the number of rows in the database
	int m_value[] = {1, 1024, 4096, 16384, 65536};*/
	
	//case2: k = 2, t = 1, m = 2^10: running n (256, 1024, 4096, 16384, 65536)
	//n_value is the number of columns in the database
	int n_value[] = {256, 1024, 4096};
	//m_value is the number of rows in the database
	int m_value[] = {1024};
	
	//k is the number of servers
	int k_server[] = {2};
	int i_index;
	bool verifyPIR;
	int** queries;
	char tmp[DIGITS];
	int lengthn = sizeof(n_value)/sizeof(n_value[0]);
	int lengthm = sizeof(m_value)/sizeof(m_value[0]);
	clock_t start, stop; //The time counters
	double s_PIR_time, c_PIR_time, s_LMC_time, c_LMC_time;
	
	FILE *fp;
	//fp = fopen("graphs/case1.txt", "a+"); // open file for reading and writing (append if file exists)
	fp = fopen("graphs/case2.txt", "a+"); // open file for reading and writing (append if file exists)
	
	//initialization of a prime number p
	mpz_t p;
	mpz_init_set_str (p, R, 16);
	gmp_printf ("p = %Zd \n", p);
	queries = (int**) malloc(k_server[0] * sizeof(int*));
	
	mpz_t *DataBase; //Database
    	mpz_t *hash_cols; //Hash values of each column in the DataBase
	
    	blst_p2 Com; //commitment of x
    	blst_p2 C[k_server[0]]; //commitment of x
    	blst_scalar y[k_server[0]]; //y_i = F_i.x 
    	blst_p1 proof[k_server[0]]; //proof of y_i
    	
    	for (int n = 0; n < lengthn; n++)
	{
		printf ("\nThe value of n is %d \n", n_value[n]);
		
		fprintf(fp, "%d \n", n_value[n]); // write number n to file
		
		blst_p2 *G = malloc(sizeof(blst_p2)*(n_value[n]+1));
    		blst_p1 *H = malloc(sizeof(blst_p1)*(2*n_value[n]+1));
		blst_scalar *x = malloc(sizeof(blst_scalar)*(n_value[n]+1)); //address of an array of blst_scalar
		blst_scalar **F; //[2][(n_value[i] + 1)] coefficients of polynomials Fi
		
		F = (blst_scalar**) malloc(sizeof(blst_scalar*) * k_server[0]);
		
		for (int i = 0; i < k_server[0]; i++) 
		{
			F[i] = (blst_scalar*) malloc(sizeof(blst_scalar)*(n_value[n] + 1));
		}
		
		//0. Setup LMC
    		start = clock();
    		setup(n_value[n], G, H);
    		stop = clock();
    		printf("\nLMC: Setup = %lf seconds\n\n", (double) (stop - start) / CLOCKS_PER_SEC);
		
		for (int m = 0; m < lengthm; m++)
		{	
			printf ("The value of m is %d\n", m_value[m]);
			
			fprintf(fp, "%d \n", m_value[m]); // write number m to file
			
			mpz_t** answers; //[2][m_value[m] + 1]
			mpz_t result[m_value[m] + 1];
			i_index = rand()%n_value[n];
			hash_cols = (mpz_t *) malloc(n_value[n] * sizeof(mpz_t));
			DataBase = (mpz_t *) malloc((m_value[m] * n_value[n]) * sizeof(mpz_t));
			
			printf ("-----------------START-------------- \n");
			printf ("              Desired x_%d \n\n", i_index);
			s_PIR_time = 0.0;
			c_PIR_time = 0.0;
			s_LMC_time = 0.0;
			c_LMC_time = 0.0;
			//----------------------------------------------Step 1: Data Owner-----------------------------------------------//
			
			//1.1. creates randomly each element in the database.
			start = clock();
			DataBase = DbGen_Server(m_value[m], n_value[n], p);
			stop = clock();
			printf ("DbGen time = %f seconds\n", (double) (stop - start) / CLOCKS_PER_SEC);
    	
    			//1.2. calculates hash values of n db's columns.
			start = clock();
    			hash_cols = HashGen_Server(m_value[m], n_value[n], DataBase, p);
    			stop = clock();  
    			printf ("HashGen time = %f seconds\n\n", (double) (stop - start) / CLOCKS_PER_SEC);
    			
			//Stored all x_i in blst_scalar
    			for (int o = 1; o <= n_value[n]; o++)
    			{
				mpz_get_str(tmp, 10, hash_cols[o - 1]);
        			byte *in = tmp;
        			blst_scalar_from_be_bytes(&x[o], in, 32);
    			}
    			
    			//1.3. Commites to hashes values.
    			start = clock();
    			com(n_value[n], x, G, &Com);
    			stop = clock();
    			printf("LMC: Commitment time = %lf seconds\n\n", (double) (stop - start) / CLOCKS_PER_SEC);
    			
    			for (int ii = 0; ii < k_server[0]; ii++)
	    		{
	    			C[ii] = Com;
	    		}
    			
			//-----------------------------------------------Step 2. Client----------------------------------------------//
			
			//2.1 creates random queries
			start= clock();
			queries = QueriesGen_Client (i_index, n_value[n]);
			stop = clock();
    			printf ("Chor: QueryGen time = %f seconds\n\n", (double) (stop - start) / CLOCKS_PER_SEC);
			
			c_PIR_time += (double) (stop - start) / CLOCKS_PER_SEC;
			
			//-----------------------------------------------Step 3. Server----------------------------------------------//
	
			//3.1. Anwsers for the queries
			start= clock();
			answers = AnwGen_Server (m_value[m], n_value[n], queries, DataBase, hash_cols, p);
			stop = clock();  
    			printf ("Chor: AnwGen time = %f seconds\n", (double) (stop - start) / CLOCKS_PER_SEC);
			
			s_PIR_time += (double) (stop - start) / CLOCKS_PER_SEC;
			
			//3.2. Computes y_i = F_i.x and send proof_i to the Client
		    	for (int ii = 0; ii < k_server[0]; ii++)
			{
		    		s_LMC_time += WitnessGen(n_value[n], k_server[0], queries, x, F, H, proof, y, ii, start, stop);
			}
			printf("\n\n");
		
			//-----------------------------------------------Step 4. Client----------------------------------------------//
			//4.1. Verifies proofi before decoding
			for (int ii = 0; ii < k_server[0]; ii++)
			{
				start = clock();
	    			assert(verify(n_value[n], F[ii], &y[ii], G, H, &C[ii], &proof[ii]));
	    			stop = clock();
	    			printf("LMC: Verify y_%d time = %lf seconds\n", ii, (double) (stop - start) / CLOCKS_PER_SEC);
	    			
	    			c_LMC_time += (double) (stop - start) / CLOCKS_PER_SEC;
			}
			printf("\n");
			
			//4.3. Extracts to get x_i
			start = clock();
			Extract_Client (m_value[m], n_value[n], p, queries, i_index, answers, result);
			stop = clock();  
    			printf ("Chor: Decode x_i time = %f seconds\n", (double) (stop - start) / CLOCKS_PER_SEC);
    			
    			c_PIR_time += (double) (stop - start) / CLOCKS_PER_SEC;
			
			//4.4. Verifies the hash values of db_i
			start = clock();
			verifyPIR = Verify_Client (m_value[m], p, result);
			stop = clock();
    			printf ("Chor: Verify hash values time = %f seconds\n\n", (double) (stop - start) / CLOCKS_PER_SEC);
			
			c_LMC_time += (double) (stop - start) / CLOCKS_PER_SEC;
			
			if (verifyPIR)
			{
				printf ("***** PASSED! Client received correct x_%d \n\n", i_index);
			}
			else
			{
				printf ("***** FAILED! Client received uncorrect x_%d \n\n", i_index);
			}
			
			fprintf(fp, "%f \n", s_PIR_time/k_server[0]); // write number n to file
			fprintf(fp, "%f \n", c_PIR_time); // write number n to file
			fprintf(fp, "%f \n", s_LMC_time/k_server[0]); // write number n to file
			fprintf(fp, "%f \n", c_LMC_time); // write number n to file
		}
	}
    
    fclose(fp); // close file
    
    return 0;
}
