

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <ctime>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <fstream>

#include "../bigint/bigint.h"

class RSA {

public:

	RSA();

	std::vector<int> encrypt(std::string msg);

	std::vector<int> decrypt();

private:

	int bits,
	    rounds;

	signedBigInt x;
	signedBigInt y;

	BigInt pq;   // p*q
	BigInt phi;  // (p-1)(q-1)
	BigInt d;
	BigInt gcd;  // result of Euclid algorhitm (finding greatest common divisor)
	BigInt e;

	BigInt num1;
	BigInt num2;

	BigInt encryptedValue;
	BigInt decryptedValue;

	// first prime nums (needed for primacy test)
	std::vector<int> fPrimeNums;

	// generating big prime numbers functions:

	BigInt generateRandom(int bits);
	BigInt getPrimeCandidate(int bits);
	BigInt modPow(BigInt base, BigInt exponent, BigInt mod);
	void   generatePrimes(int n);
	bool   millerRabinPassed(BigInt candidate, int rounds);

	// RSA implementing functions:
	BigInt pqMultiplication(BigInt p, BigInt q);
	BigInt EulerFunction(BigInt p, BigInt q);
	BigInt extendedGCD(BigInt a, BigInt b, signedBigInt& x, signedBigInt& y);


};
