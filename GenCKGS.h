// GenCKGS.h
#pragma once

//Client creates random queries (binary vectors size n) (position = i_index)
mpz_t** QueriesGen_Client (int position, int n, int k, mpz_t p);

//The server will respond the anwser when received query
mpz_t** AnwGen_Server (int m, int n, mpz_t** queries, mpz_t** DataBase_T, int k, mpz_t p);

//After received all anwsers from two servers, the client will decode to get x_i and h(x_i) values
void Extract_Client (int m, int k, mpz_t p, mpz_t** answers, mpz_t result[m + 1]);

//Server computes y_i = F_i.x and send proof_i to the Client
double WitnessGen(int n_value, int k_server, mpz_t** queries, blst_scalar *x, blst_scalar **F, blst_p1 *H, blst_p1 proof[k_server], blst_scalar y[k_server], int i, clock_t start, clock_t stop);
