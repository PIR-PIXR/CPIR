<p align="center">
  <img width="476" height="249" src="https://user-images.githubusercontent.com/87842051/213029654-f513939d-4e7e-4baa-8a6a-5b74aa4484e8.png">
</p>

# CPIR: A "Committed Private Information Retrieval" Library that achieves high performance and flexibility.

## Abstract
A private information retrieval (PIR) scheme allows a client to retrieve a data item xi among n items x1, x2, . . . , xn from k servers, without revealing what i is even when t < k servers collude and try to learn i. Such a PIR scheme is said to be t-private. A PIR scheme is v-verifiable if the client can verify the correctness of the retrieved xi even when v ≤ k servers are colluding and try to fool the client by sending manipulated data. Most of the previous works in the literature on PIR assumed that v < k, leaving the case of all-colluding servers open. We propose a generic method that combines a linear map commitment (LMC) and an arbitrary linear PIR scheme to produce a k-verifiable PIR scheme, termed a committed PIR scheme. Such a scheme guarantees that even in the worst scenario when all servers are under the control of an attacker, although the privacy is unavoidably lost, the client won’t be fooled into accepting an incorrect xi. A committed PIR can also provide accountability and Byzantine-robustness, which allows the client to tell which servers have sent incorrect data and then recover the correct data, albeit at the cost of heavier computation. We demonstrate the practicality of the committed PIR schemes by implementing the combinations of an LMC and three well-known PIR schemes on top of the GMP library and blst, the current fastest C library for pairings on elliptic curves. You can find a copy of the paper [here]().

---
## Main Contributions
- We propose a novel modular approach that combines a linear map commitment scheme and a linear PIR scheme to construct a committed PIR scheme that provides verifiability, accountability, and Byzantine-robustness, apart from the traditional privacy. Our proposed scheme is capable of preventing the client from accepting an incorrect data item, even when *all* k servers are malicious and send manipulated data. Most previous works can only tolerate upto k − 1 malicious servers.
- We carry out three case studies discussing the constructions of committed PIR schemes using a specific linear map commitment on top of the three
well-known representative PIR schemes: the CKGS scheme (the very first PIR scheme), the WY scheme (lowest upload cost, aka query size), and the BE scheme (lowest download cost, aka answer size). The LMC primitive incurs only a constant-size communication overhead.
- We implemented all three schemes in C on top of the GMP library (for efficient handling of large numbers) and the blst library, the current fastest library for elliptic curve operations and pairings. All three achieved reasonably running times demonstrating the practicality of our proposal.

---
## Experimental setup
We ran our experiments on Ubuntu 22.04.1 LTS environment where the processor was Intel® Core™ i5-1035G1 CPU @ 1.00GHz×8, 15GB System memory. We compiled our C code with GCC version 11.3.0,GMP 6.2.1 for calculating the large number, Openssl 2022 for hashing data with SHA3-256, and [blst](https://github.com/supranational/blst) v.0.3.10 2022 for LMC parts. Our program is more than 3000 lines of C language and is published on [Github](https://github.com/PIR-PIXR/CPIR).

---
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

### Executing LM-CKGS (2-CKGS)
    $ gcc -o ComCKGS ComCKGSmain.c database.c LMC.c verifyhash.c CKGS.c -lcrypto -lgmp -L /FULL-PATH/ComPIR/blst/lib/ -lblst
    $ ./ComCKGS
### Executing LM-CKGS (k-CKGS)
    $ gcc ComGenCKGS ComGenCKGSmain.c database.c LMC.c verifyhash.c GenCKGS.c utils.c -lcrypto -lgmp -L /FULL-PATH/ComPIR/blst/lib/ -lblst
    $ ./ComGenCKGS
### Executing LM-WY
    $ gcc -o ComWY ComWYmain.c database.c LMC.c verifyhash.c WY.c utils.c -lcrypto -lgmp -L /FULL-PATH/ComPIR/blst/lib/ -lblst
    $ ./ComWY
### Executing LM-BE
    $ gcc -o ComBE ComBEmain.c database.c LMC.c verifyhash.c BE.c utils.c -lcrypto -lgmp -lm -L /FULL-PATH/ComPIR/blst/lib/ -lblst
    $ ./ComBE

---
## Performance
The implementation results show that our schemes with extra protection for the Client will increase the **constant** practical time on both servers and the client sides compared to the original PIR. We implemented multiple database sizes and several servers in the system. The overhead of LM is constant with the same size n, so when the database size increases, the percentage of LM time reduces significantly in the total LM-PIR time (see Fig. 1). The computation time for servers and the Client is below one second in most scenarios when the number of colluding servers is k − 1, except in the LM-WY case.

<p align="center">
  <img width="400" height="300" src="https://user-images.githubusercontent.com/87842051/213037442-67e59f59-e73d-424f-9b39-6fcf5ddc3461.png"> <img width="400" height="300" src="https://user-images.githubusercontent.com/87842051/213038130-c65c1388-7e95-4100-af92-c71a4337a0f9.png">
</p>
<strong> Fig. 1.</strong> The comparison of the average server and client computation times regraded to three different LM-PIR cases (LM-CKGS, LM-WY, and LM-BE) where k = 2, t = 1, n = 2^10, and m is changing (2^0, 2^12, 2^14, 2^16). When m increases, the total LM-PIR computation time increases, but the LM computation time parts are constant. It means that the percentage of LMC-related computation time decreases significantly when the vertical database increases.

<p align="center">
  <img width="400" height="300" src="https://user-images.githubusercontent.com/87842051/213037482-bde95eb1-f9e2-4f92-a6d5-4b5b52c77194.png"> <img width="400" height="300" src="https://user-images.githubusercontent.com/87842051/213037508-84804e8b-b4d2-4d13-9bd0-bb5894c9422f.png">
</p>
<strong> Fig. 2.</strong> The comparison of the average server and client computation times between LM-CKGS and LM-BE schemes where n = 2^10, m = 2^12, t = (k − 1) for both schemes, t = 1 for LM-BE scheme, and k increases from 3 to 6. Assume (k − 1) colluding servers in the system, the LM-BE scheme performs quite better on the server side. In contrast, the LM-CKGS computational cost is slightly lower on the Client side. For the LM-BE scheme, there is a tradeoff when t changes and higher t means higher computation costs.

<p align="center">
  <img width="400" height="300" src="https://user-images.githubusercontent.com/87842051/213037466-2a36574d-e036-4f78-bcc9-c20a6e7201e9.png"> <img width="400" height="300" src="https://user-images.githubusercontent.com/87842051/213037495-19092435-2fae-4333-bd27-cecead75c659.png">
</p>
<strong> Fig. 3.</strong> The comparison of the average server and client computation times regraded to three different LM-PIR cases (LM-CKGS, LM-WY, and LM-BE) where k = 2, t = 1, m = 2^10, and n increases from 2^8 to 2^12. When n increases, the total LM-PIR computation time increases, especially the LM computation time increases significantly. It means that the percentage of LMC-related computation time increases significantly when the horizontal database increases.

<p align="center">
  <img width="400" height="300" src="https://user-images.githubusercontent.com/87842051/213037471-3d83cbcd-fdd1-4cd7-803c-b81f8e17b6f2.png"> <img width="400" height="300" src="https://user-images.githubusercontent.com/87842051/213037500-01795d8e-65c5-4e3a-a0f9-94b8198d146c.png">
</p>
<strong> Fig. 4.</strong> The comparison of the average server and client computation times of the LM-WY scheme where t = 1, n = 2^10, m = (2^10, 2^12), and (d, k) =
((3, 2), (4, 3), (5, 3), (6, 4)). When d increases, the LM-PIR computation time on the server side tends to decrease because the number of answers and witnesses decreases regarded to O(n^(1/d)). However, when d increases, the size of queries also grows. It is why the computation cost of higher d is slightly higher in some cases, but in general, the computation time on the server side reduces when d rises. On the Client side, the computation cost trend is similar to the computation time on the server. However, d increases lead to an increase in k, so the total computation time on the Client side grows.

---
## ACKNOWLEDGMENTS 
This work was supported by the Australian Research Council through the Discovery Project under Grant DP200100731 and carried out on Oracle virtual machines, supported by Oracle for Research.
