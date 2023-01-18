// GenCKGS.h
#pragma once

//Clients creates random queries (binary vectors size n) (position = i_index)
mpz_t** QueriesGen_Client (int position, int n, int k, mpz_t p);

//The server will respond the anwser when received query
mpz_t** AnwGen_Server (int m, int n, mpz_t** queries, mpz_t** DataBase_T, int k, mpz_t p);

//After received all anwsers from two servers, the client will decode to get x_i and h(x_i) values
void Extract_Client (int m, int k, mpz_t p, mpz_t** answers, mpz_t result[m + 1]);
