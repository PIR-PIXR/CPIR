// CKGS.h
#pragma once

//Clients creates random queries (binary vectors size n) (position = i_index)
int** QueriesGen_Client (int position, int n);

//The server will respond the anwser when received query
mpz_t** AnwGen_Server (int m, int n, int** queries, mpz_t *X_db,  mpz_t *hash_cols, mpz_t p);

//After received all anwsers from two servers, the client will decode to get x_i and h(x_i) values
void Extract_Client (int m, int n, mpz_t p, int** queries, int position, mpz_t** answers, mpz_t result[m + 1]);
