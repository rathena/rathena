This is a Mersenne Twister pseudorandom number generator
with period 2^19937-1 with improved initialization scheme,
modified on 2002/1/26 by Takuji Nishimura and Makoto Matsumoto. 
modified on 2005/4/26 by Mutsuo Saito

Contents of this tar ball:
readme-mt.txt	 this file
mt19937ar.c	 the C source (ar: initialize by ARray)
mt19937ar.h      the C header file for mt19937ar
mtTest.c         the C test main program of mt19937ar.c
mt19937ar.out	 Test outputs of six types generators. 1000 for each

1. Initialization
  The initialization scheme for the previous versions of MT
(e.g. 1999/10/28 version or earlier) has a tiny problem, that
the most significant bits of the seed is not well reflected 
to the state vector of MT.

This version (2002/1/26) has two initialization schemes:
init_genrand(seed) and init_by_array(init_key, key_length).

init_genrand(seed) initializes the state vector by using
one unsigned 32-bit integer "seed", which may be zero.

init_by_array(init_key, key_length) initializes the state vector 
by using an array init_key[] of unsigned 32-bit integers
of length key_kength. If key_length is smaller than 624,
then each array of 32-bit integers gives distinct initial
state vector. This is useful if you want a larger seed space
than 32-bit word.

2. Generation
After initialization, the following type of pseudorandom numbers
are available. 

genrand_int32() generates unsigned 32-bit integers.
genrand_int31() generates unsigned 31-bit integers.
genrand_real1() generates uniform real in [0,1] (32-bit resolution). 
genrand_real2() generates uniform real in [0,1) (32-bit resolution). 
genrand_real3() generates uniform real in (0,1) (32-bit resolution).
genrand_res53() generates uniform real in [0,1) with 53-bit resolution.

Note: the last five functions call the first one. 
if you need more speed for these five functions, you may
suppress the function call by copying genrand_int32() and
replacing the last return(), following to these five functions.

3. main()
main() is an example to initialize with an array of length 4,
then 1000 outputs of unsigned 32-bit integers, 
then 1000 outputs of real [0,1) numbers. 

4. The outputs
The output of the mt19937ar.c is in the file mt19937ar.out.
If you revise or translate the code, check the output
by using this file. 

5. Cryptography
This generator is not cryptoraphically secure. 
You need to use a one-way (or hash) function to obtain 
a secure random sequence.

6. Correspondence
See:
URL http://www.math.keio.ac.jp/matumoto/emt.html
email matumoto@math.keio.ac.jp, nisimura@sci.kj.yamagata-u.ac.jp

7. Reference
M. Matsumoto and T. Nishimura,
"Mersenne Twister: A 623-Dimensionally Equidistributed Uniform  
Pseudo-Random Number Generator",
ACM Transactions on Modeling and Computer Simulation,
Vol. 8, No. 1, January 1998, pp 3--30.

-------
Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
All rights reserved.
Copyright (C) 2005, Mutsuo Saito
All rights reserved.
