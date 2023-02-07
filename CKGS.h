// CKGS.h
#pragma once

//Client creates random queries (binary vectors size n) (position = i_index)
int** QueriesGen_Client (int position, int n);

//The Server will respond the anwsers when received query
mpz_t** AnwGen_Server (int m, int n, int** queries, mpz_t *X_db,  mpz_t *hash_cols, mpz_t p);

//Server computes y_i = F_i.x and send proof_i to the Client
double WitnessGen(int n_value, int k_server, int** queries, blst_scalar *x, blst_scalar **F, blst_p1 *H, blst_p1 proof[k_server], blst_scalar y[k_server], int i, clock_t start, clock_t stop);

//After received all anwsers from two servers, the client will decode to get x_i and h(x_i) values
void Extract_Client (int m, int n, mpz_t p, int** queries, int position, mpz_t** answers, mpz_t result[m + 1]);
