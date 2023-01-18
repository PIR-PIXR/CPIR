// BE.h
#pragma once

//Create Vandermonde matrix
mpz_t** vandermonde(int n, mpz_t p);

//A simple method for finding the inverse matrix of Vandermonde matrix
mpz_t* S(int k, int j, int m, unsigned long max_n, long** E_val);

//A simple method for finding the inverse matrix of Vandermonde matrix
mpz_t* P(int j, int m);

//Finding inverse of the Vandermonde matrix (V_inv)
mpz_t** vandermonde_inverse(int k, mpz_t p, long*** E_val);

//Generate all possible distinct points of of a fixed length n with the Hamming weight d
long** generate(unsigned long n, unsigned long d, unsigned long max_n);

//Clients creates random queries (binary vectors size alpha*n)
mpz_t** QueriesGen_Client (int i_index, int n, int alpha, int t_private, mpz_t** V, mpz_t p);

//Split Database size m*n to new database size m*alpha*n transpose
mpz_t** SplitDatabase(int m, int n, int alpha, mpz_t* DataBase, mpz_t* hash_cols, mpz_t p);

//After received all anwsers from all servers, the client will extract to get x_i and h(x_i) values
void Extract_Client (int alpha, int m, mpz_t** Reconstruct, mpz_t result[m + 1], mpz_t p);
