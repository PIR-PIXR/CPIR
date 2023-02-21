// LMC.h
#pragma once

void mod_r(blst_scalar *a);

void setup(const int n, const int q, blst_p2 *G, blst_p1 *H);

void com(const int n, const blst_scalar *x, const blst_p2 *G, blst_p2* out_C);

void open(const int n, const int q, const blst_scalar *x, const blst_scalar *F, const blst_p1 *H, blst_p1 *out_proof);

bool verify(const int n, const int q, const blst_scalar *F, const blst_scalar *y, const blst_p2 *G, const blst_p1 *H, const blst_p2 *com, const blst_p1 *proof);
