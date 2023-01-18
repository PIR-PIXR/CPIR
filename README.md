<p align="center">
  <img width="476" height="249" src="https://user-images.githubusercontent.com/87842051/213029654-f513939d-4e7e-4baa-8a6a-5b74aa4484e8.png">
</p>

# CPIR: A Committed Private Information Retrieval Library that achieves high performance and flexibility.

## Abstract. 
A private information retrieval (PIR) scheme allows a client to retrieve a data item xi among n items x1, x2, . . . , xn from k servers, without revealing what i is even when t < k servers collude and try to learn i. Such a PIR scheme is said to be t-private. A PIR scheme is v-verifiable if the client can verify the correctness of the retrieved xi even when v ≤ k servers are colluding and try to fool the client by sending manipulated data. Most of the previous works in the literature on PIR assumed that v < k, leaving the case of all-colluding servers open. We propose a generic method that combines a linear map commitment (LMC) and an arbitrary linear PIR scheme to produce a k-verifiable PIR scheme, termed a committed PIR scheme. Such a scheme guarantees that even in the worst scenario when all servers are under the control of an attacker, although the privacy is unavoidably lost, the client won’t be fooled into accepting an incorrect xi. A committed PIR can also provide accountability and Byzantine-robustness, which allows the client to tell which servers have sent incorrect data and then recover the correct data, albeit at the cost of heavier computation. We demonstrate the practicality of the committed PIR schemes by implementing the combinations of an LMC and three well-known PIR schemes on top of the GMP library and blst, the current fastest C library for pairings on elliptic curves. You can find a copy of the paper [here]().

## Experimental setup
We ran our experiments on Ubuntu 22.04.1 LTS environment where the processor was Intel® Core™ i5-1035G1 CPU @ 1.00GHz×8, 15GB System memory. We compiled our C code with GCC version 11.3.0,GMP 6.2.1 for calculating the large number, Openssl 2022 for hashing data with SHA3-256, and [blst](https://github.com/supranational/blst) v.0.3.10 2022 for LMC parts. Our program is more than 3000 lines of C language and is published on [Github](https://github.com/PIR-PIXR/CPIR).

## Compiling CPIR
### Installing Libraries

- #### GCC
      $ sudo apt update
      $ sudo apt upgrade
      $ sudo apt install build-essential
- #### GMP
      Download GMP from https://gmplib.org/
      Extract
      $ sudo apt-get install m4
      $ ./configure
      $ make
      $ sudo make install
      $ make check
- #### Openssl
      $ sudo apt install libssl-dev
- #### Blst
      $ git clone https://github.com/supranational/blst.git
      $ cd blst
      $ ./build.sh
      Copy file libblst.a created to folder blst/lib of the project (CPIR folder)

### Testing LM-CKGS (2-CKGS)
    $ gcc -o ComCKGS ComCKGSmain.c database.c LMC.c verifyhash.c CKGS.c -lcrypto -lgmp -L /FULL-PATH/ComPIR/blst/lib/ -lblst
    $ ./ComCKGS
### Testing LM-CKGS (k-CKGS)
    $ gcc ComGenCKGS ComGenCKGSmain.c database.c LMC.c verifyhash.c GenCKGS.c utils.c -lcrypto -lgmp -L /FULL-PATH/ComPIR/blst/lib/ -lblst
    $ ./ComGenCKGS
### Testing LM-WY
    $ gcc -o ComWY ComWYmain.c database.c LMC.c verifyhash.c WY.c utils.c -lcrypto -lgmp -L /FULL-PATH/ComPIR/blst/lib/ -lblst
    $ ./ComWY
### Testing LM-BE
    $ gcc -o ComBE ComBEmain.c database.c LMC.c verifyhash.c BE.c utils.c -lcrypto -lgmp -lm -L /FULL-PATH/ComPIR/blst/lib/ -lblst
    $ ./ComBE
   
## Performance
<p align="center">
  <img width="448" height="341" src="https://user-images.githubusercontent.com/87842051/213037442-67e59f59-e73d-424f-9b39-6fcf5ddc3461.png"> <img width="448" height="341" src="https://user-images.githubusercontent.com/87842051/213038130-c65c1388-7e95-4100-af92-c71a4337a0f9.png">
</p>
Fig. 1. The comparison of the average server and client computation times regraded to three different LM-PIR cases (LM-CKGS, LM-WY, and LM-BE) where k = 2, t = 1, n = 210, and m is changing (20, 212, 214, 216). When m increases, the total LM-PIR computation time increases, but the LM computation time parts are constant. It means that the percentage of LMC-related computation time decreases significantly when the vertical database increases.

<p align="center">
  <img width="476" height="249" src="https://user-images.githubusercontent.com/87842051/213029654-f513939d-4e7e-4baa-8a6a-5b74aa4484e8.png">
</p>

<p align="center">
  <img width="476" height="249" src="https://user-images.githubusercontent.com/87842051/213029654-f513939d-4e7e-4baa-8a6a-5b74aa4484e8.png">
</p>

<p align="center">
  <img width="476" height="249" src="https://user-images.githubusercontent.com/87842051/213029654-f513939d-4e7e-4baa-8a6a-5b74aa4484e8.png">
</p>
