#include <stdio.h>
#include <string.h>
#include "blst/include/blst.h"
#include "LMC.h"

#define R "73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001" //R is prime number in BLS12-381. It is also the prime number p in Z_p

void mod_r(blst_scalar *a)
{
	char str[64];
    	
    	for (int h = 0; h < 32; h++)
    	{
        	sprintf(str+(h*2), "%02x", R[h]); 
        }s
        
        blst_fr order, a_r;
        byte *BLST_381_r = str;
    	blst_fr_from_hexascii(&order, BLST_381_r);
    	blst_fr_from_scalar(&a_r, a);
    	
    	while(!blst_sk_check(&a_r))
    	{
        	blst_fr_sub(&a_r, &a_r, &order);
    	}
    	
    	blst_scalar_from_fr(a, &a_r);
}

void setup(const int n, blst_p2 *G, blst_p1 *H)
{
    	byte *IKM1 = "comitted-private-information-retrieval";
    	byte *IKM2 = "CKGS-GenCKGS-WY-BE";

    	// generate 256-bit secret (in Zr)
    	blst_scalar alpha, z, tmp;
    	blst_fr alpha_r, z_r, tmp_r;

    	blst_keygen(&alpha, IKM1, strlen(IKM1), NULL, 0);
    	mod_r(&alpha);

    	blst_keygen(&z, IKM2, strlen(IKM2), NULL, 0);
    	mod_r(&z);

    	blst_fr_from_scalar(&alpha_r, &alpha);
    	blst_fr_from_scalar(&z_r, &z);

    	//Gj = g2^{alpha^j}, j=1..l: G1 = g2^alpha, G2 = G1^alpha,...
    	blst_p2_mult(G+1, blst_p2_generator(), alpha.b, 256); // G1 = g2^alpha
    	
    	for (int i = 2; i <= n; i++) 
    	{
        	blst_p2_mult(G+i, G+i-1, alpha.b, 256); //G[i] = G[i-1]^alpha
    	}

    	//Hj = g1^{z*alpha^j}, j=1..2*l: H1 = g1^{z.alpha}, H2 = H1^alpha
    	blst_fr_mul(&tmp_r, &z_r, &alpha_r);
    	blst_scalar_from_fr(&tmp, &tmp_r);

    	blst_p1_mult(H+1, blst_p1_generator(), tmp.b, 256); // H1 = g1^z.alpha
    	
    	for (int i = 2; i <= 2*n; i++) 
    	{
        	blst_p1_mult(H+i, H+i-1, alpha.b, 256);
    	}
}

void com(const int n, const blst_scalar *x, const blst_p2 *G, blst_p2* out_C)
{
    	// out_C = G1^x1 . G2^x2 ... Gl^xl
    	blst_p2 tmp, sum, current_sum;
    	blst_p2_mult(&current_sum, G+1, x[1].b, 256); //G1^x1
    	
    	for (int i = 2; i <= n; i++)
    	{
        	blst_p2_mult(&tmp, G+i, x[i].b, 256);
        	blst_p2_add_or_double(&sum, &current_sum, &tmp);
        	current_sum = sum;
    	}
    	
    	*out_C = sum;
}

void open(const int n, const blst_scalar *x, const blst_scalar *F, const blst_p1 *H, blst_p1 *out_proof)
{
    	blst_scalar s[2*n + 1];
    	uint64_t zero[4] = {0, 0, 0, 0};
    	blst_fr F_r, x_r, tmp_mul_r, tmp_add_r, s_r;
    
    	for (int delta = 1 - n; delta <= n - 1; delta++) //delta = j - j', delta != 0
    	{ 
        	if (delta != 0) 
        	{
            		blst_scalar_from_uint64(&s[delta + n + 1], zero);
            		blst_fr_from_scalar(&s_r, &s[delta + n + 1]);
            
            	for (int j = 1; j <= n; j++) 
            	{
                	if ((1 <= j - delta) && (j - delta <= n)) //j != j' and j' in [l]
                	{ 
                    		blst_fr_from_scalar(&F_r, F + j - delta); //F[j']
                    		blst_fr_from_scalar(&x_r, x + j);
                    		blst_fr_mul(&tmp_mul_r, &F_r, &x_r);
                    		blst_fr_add(&tmp_add_r, &s_r, &tmp_mul_r);
                    		s_r = tmp_add_r;
                	}
            	}
            	
            	blst_scalar_from_fr(&s[delta + n + 1], &s_r);
            	
        	}
    	}
    
    	blst_p1_mult(out_proof, H+2,s[2].b, 256);
    	blst_p1 tmp_mul, tmp_add;
    
    	for (int k = 3; k <= 2*n; k++) 
    	{
        	if (k != n+1) 
        	{
            		//proof = Product{H[k]^s[k]}
            		blst_p1_mult(&tmp_mul, H+k, s[k].b, 256);
            		blst_p1_add_or_double(&tmp_add, out_proof, &tmp_mul);
            		*out_proof = tmp_add;
        	}
    	}
}

bool verify(const int n, const blst_scalar *F, const blst_scalar *y, const blst_p2 *G, const blst_p1 *H, blst_p2 *C, const blst_p1 *proof)
{
    	// u = product{H_{l+1-j}^f_j, j=1..n}
    	const blst_p2 *g2 = blst_p2_generator();
    	blst_p1 u;
    	blst_p1 tmp_add, tmp_mul;
    	blst_p1_mult(&u, H + n, F[1].b, 256);
    
    	for (int j = 2; j <= n; j++)
    	{
        	blst_p1_mult(&tmp_mul, H + n + 1 - j, F[j].b, 256);
        	blst_p1_add_or_double(&tmp_add, &u, &tmp_mul);
        	u = tmp_add;
    	}
    
    	// v = H_l^y
    	blst_p1  v;
    	blst_p1_mult(&v, H + n, y -> b, 256);

    	blst_p2_cneg(C,1);
    	blst_p2_affine C_aff, G1_aff, g2_aff;
    	blst_p2_to_affine(&C_aff, C);
    	blst_p2_to_affine(&G1_aff, G+1);
    	blst_p2_to_affine(&g2_aff, g2);

    	blst_p1_affine u_aff, v_aff, proof_aff;
    	blst_p1_to_affine(&u_aff, &u);
    	blst_p1_to_affine(&v_aff, &v);
    	blst_p1_to_affine(&proof_aff, proof);

    	blst_fp12 e, tmp_product, tmp_miller1, tmp_miller2, tmp_miller3, ret;
    	blst_miller_loop(&tmp_miller1, &C_aff, &u_aff);
    	blst_miller_loop(&tmp_miller2, &G1_aff, &v_aff);
    	blst_fp12_mul(&tmp_product, &tmp_miller1, &tmp_miller2);
    	blst_miller_loop(&tmp_miller3, &g2_aff, &proof_aff);
    	blst_fp12_mul(&e, &tmp_product, &tmp_miller3);

    	blst_final_exp(&ret, &e);
    
    	return blst_fp12_is_one(&ret);
}
