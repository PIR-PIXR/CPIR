// database.h
#pragma once

//Server creates randomly each element in the database (m*n) and the hash values of n db's columns.
mpz_t *DbGen_Server(int m, int n, mpz_t p);

//Server calculates hash values of n db's columns.
mpz_t *HashGen_Server(int m, int n, mpz_t *X_db, mpz_t p);
