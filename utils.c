#include <gmp.h>
#include <stdlib.h>

//multiply matrices n x m matrix A with m x k matrix B and store the result in n x k matrix C
void multiply(mpz_t** A, mpz_t** B, mpz_t** C, int n, int m, int k, mpz_t p) 
{
	for (int i = 0; i < n; i++) 
	{
		for (int j = 0; j < k; j++) 
		{
			for (int o = 0; o < m; o++) 
			{
				mpz_addmul(C[i][j], A[i][o], B[o][j]);
			}
			mpz_mod (C[i][j], C[i][j], p);
		}
	}
}

// transpose matrix A of size n x m and store the result in B
void transpose(mpz_t** A, mpz_t** B, int n, int m) 
{
	for (int i = 0; i < n; i++) 
	{
		for (int j = 0; j < m; j++)
		{
			mpz_set (B[j][i], A[i][j]);
		}	
	}
}

//The factorial of a 'input' number
void factorial(mpz_t result, int input)
{
  	if (input <= 1) 
  	{
    		mpz_set_ui(result, 1);
 	} 
 	else 
 	{
    		mpz_t tmp_result;
    		mpz_init (tmp_result);
    		factorial(tmp_result, input - 1);
    		mpz_mul_ui(result, tmp_result, input);
  	}
  	return;
}

//m choose d
unsigned long mChoosed(int m, int d)
{	
	mpz_t tmpt_mCd;
	mpz_t factorial_m;
	mpz_t factorial_d;
	mpz_t factorial_md;
	
	mpz_init (tmpt_mCd);
	mpz_init (factorial_m);
	mpz_init (factorial_d);
	mpz_init (factorial_md);
	
	factorial(factorial_m, m);
	factorial(factorial_d, d);
	factorial(factorial_md, m - d);
	mpz_mul (tmpt_mCd, factorial_md, factorial_d);
	mpz_cdiv_q (tmpt_mCd, factorial_m, tmpt_mCd);
	
	mpz_clear (factorial_m);
	mpz_clear (factorial_d);
	mpz_clear (factorial_md);
	
	return mpz_get_ui(tmpt_mCd);
}

//Calculates the determinant of a matrix
mpz_t *Determinant(int n, mpz_t matrix[n][n], int matrix_size)
{
	mpz_t *det;
	det = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init (det[0]);
	mpz_t submatrix[n][n];
	
	if (matrix_size == 1)
	{
		mpz_set (det[0], matrix[0][0]);
		
		return det;
	}
    	else if (matrix_size == 2)
    	{
    		mpz_t rop;
    		mpz_init (rop);
    		mpz_mul (rop, matrix[0][0], matrix[1][1]);
    		mpz_submul (rop,  matrix[0][1], matrix[1][0]);
    		mpz_set (det[0], rop);
    		
    		return det;
    	}	
    	else
    	{
    		//loop through each element of the first matrix row
        	for (int i = 0; i < matrix_size; i++)
        	{
            		// get the submatrix
            		int subi = 0;  // row index for submatrix
            		for (int ii = 1; ii < matrix_size; ii++)
            		{
                		int subj = 0;  // column index for submatrix
                		for (int jj = 0; jj < matrix_size; jj++)
                		{
                    			if (jj == i)
                        			continue;
                    			mpz_init_set (submatrix[subi][subj], matrix[ii][jj]);
                    			subj++;
                		}
                	subi++;
            		}

            		// calculate the determinant of the submatrix
            		mpz_t *subdet;
			subdet = (mpz_t *) malloc(sizeof(mpz_t));
			mpz_init (subdet[0]);
            		subdet = Determinant(n, submatrix, matrix_size - 1);

            		// add the subdeterminant to the determinant
            		if (i % 2 == 0)
            		{
            			mpz_addmul(det[0], matrix[0][i], subdet[0]);
            		}
            		else
            		{
            			mpz_submul(det[0], matrix[0][i], subdet[0]);
            		}
    		}
    	}

	return det;
}


