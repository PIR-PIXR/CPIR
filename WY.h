// WY.h
#pragma once

#define DIGITS 77 //Number of digits in an element

//Generate all possible distinct points of a fixed length m_mapping with the Hamming weight k
void functionE(int m_mapping, int k, int dir, int pos, int top, int** E_val, int d_mapping, unsigned long max_n);

void functionE_full(int m_mapping, int k, int dir, int pos, int top, int** E_full, int d_mapping, unsigned long max_n);

//Client generates random points V
void Random_V(int size_n, int size_m, mpz_t random[size_n][size_m], mpz_t p);

//Client generates random lambdas lambda, lambda^2, ..., lambda^t
void Random_Lambda(int size_n, int size_m, mpz_t random[size_n][size_m], mpz_t p);

//Client generates Queries (Point Q) belonging {F^m}_p 
void QueryGen_Client(int i_index,int d_mapping, int k_server, int m_mapping, int t_private, int n_value, int** E_val, mpz_t queries_Q[k_server][m_mapping], mpz_t lambda_random[k_server][t_private], mpz_t V_random[t_private][m_mapping],  mpz_t p);

//Calculates Function F(Q)
mpz_t *FunctionF_Server(int n_value, int m_value, int m_mapping, int d_mapping, mpz_t query_Q[m_mapping], mpz_t *DataBase, mpz_t *hash_cols, int** E_val, mpz_t p);

//Calculates Derivative of F with respect to z_pos at point Q_h
mpz_t *DerivativeF_Server(int n_value, int m_value, int m_mapping, int z_pos, mpz_t query_Q[m_mapping], mpz_t *DataBase, mpz_t *hash_cols, int** E_val, mpz_t p);

//Client reconstructs the value of f'(lambda_h)
mpz_t *Re_der_f(int server_ID, int l_pos, int t_private, int k_server, int m_value, int m_mapping, mpz_t *derF_Q, mpz_t V_random[t_private][m_mapping], mpz_t lambda_random[k_server][t_private], mpz_t p);

//coefficients of polynomials in Server i for LMC
void coefficients(mpz_t coef, int n_value, int** E_val, int k, int m_mapping, mpz_t queries_Q[k][m_mapping], char tmp[DIGITS], mpz_t p, blst_scalar **F, int i);

//Server computes y_i = F_i.x and send proof_i to the Client
double WitnessGen(int m_mapping, int k_server, int n_value, blst_scalar *x, blst_scalar **F, blst_p1 *H, blst_p1 proof[k_server], blst_scalar y[k_server], int i, clock_t start, clock_t stop);

//Client starts to reconstruct x_i
void Reconstruct_Client(int degree_d, int k_server, int t_private, int m_value, mpz_t lambda_random[k_server][t_private], mpz_t result[m_value + 1], mpz_t p, mpz_t** F_Q, mpz_t** derf_lambda);
