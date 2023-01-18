// verifyhash.h
#pragma once

//After decoded to get x_i, the Client will check by hash(x_i) and checked with decoded h(x_i) value
bool Verify_Client (int m, mpz_t p, mpz_t* result);
