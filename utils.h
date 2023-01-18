// utils.h
#pragma once

//multiply matrices n x m matrix A with m x k matrix B and store the result in n x k matrix C
void multiply(mpz_t** A, mpz_t** B, mpz_t** C, int n, int m, int k, mpz_t p);

//The factorial of a 'input' number
void factorial(mpz_t result, int input);

//m choose d
unsigned long mChoosed(int m, int d);

//Calculates the determinant of a matrix
mpz_t *Determinant(int n, mpz_t matrix[n][n], int matrix_size);
