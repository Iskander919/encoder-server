
#include "rsa.h"


RSA::RSA() {

	generatePrimes(10000);

	bits = 8;
	rounds = 20;

	e = 65537;

	// generating p and q:
	do {

		num1 = getPrimeCandidate(bits);

	}

	while (!millerRabinPassed(num1, rounds));

	while (1) {

		num2 = getPrimeCandidate(bits);
		if(num2 == num1) continue;

		if (millerRabinPassed(num2, rounds)) break;

	}

	pq  = pqMultiplication(num1, num2);
	phi = EulerFunction(num1, num2);
	gcd = extendedGCD(e, phi, x, y);

	if (x.sign == -1) {

		d = phi - (x.value % phi);

	}

	else {

		d = x.value % phi;

	}

	encryptedValue = BigInt("0");
	decryptedValue = BigInt("0");

}

std::vector<int> RSA::encrypt(std::string msg) {

	encryptedValue = modPow(msg, e, pq);

	std::array<int, 64> array;

	std::vector<int> result;

	//std::cout << encryptedValue << std::endl;

	for(int i = encryptedValue.length() - 1; i >= 0; i--) {

		array[i] = encryptedValue[i];

	}

	for(int i = encryptedValue.length() - 1; i >= 0; i--) {

		result.push_back(array[i]);

	}

	return result;

}

std::vector<int> RSA::decrypt() {

	decryptedValue = modPow(encryptedValue, d, pq);

	//std::cout << decryptedValue << std::endl;

	std::vector<int> resultDecrypted;

	for(int i = decryptedValue.length() - 1; i >= 0; i--) {

		resultDecrypted.push_back(decryptedValue[i]);

	}

	return resultDecrypted;

}


/**
 * @brief
 * @param
 * @return
 */
BigInt RSA::generateRandom(int bits) {

	srand(time(NULL));

	BigInt max("0");
	BigInt min("0");

	max = (BigInt("2")^BigInt(bits)) - BigInt("1");
	min = (BigInt("2")^(BigInt(bits) - BigInt(1))) + BigInt("1");

	return min + (BigInt(rand()) % (max - min + BigInt("1")));

}

/**
 * @brief
 * @param
 * @return
 */
BigInt RSA::getPrimeCandidate(int bits) {

	while(1) {

		BigInt primeCandidate = generateRandom(bits);
		bool isDivisible = false;

		for (int divisor : fPrimeNums) {

			if(primeCandidate % BigInt(divisor) == BigInt("0")) {

				isDivisible = true;
				break;

			}

			if(BigInt(divisor) * BigInt(divisor) > primeCandidate) break;

		}

		if (!isDivisible) return primeCandidate;

	}

}


void RSA::generatePrimes(int n) {

	int i = 0;
	int j = 0;

	for (i = 0; i <= n; i++) {								//filling vector with all numbers from 0 to n

		if(i == 0 || i == 1) {fPrimeNums.push_back(0);}

		else {fPrimeNums.push_back(i);}

	}

	i = 0;

	while(i <= n) {


		if (fPrimeNums[i] != 0) {

			j = i + i;

			while(j <= n) {

				fPrimeNums[j] = 0;
				j += i;

			}

		}

		i++;

	}

	fPrimeNums.erase(std::remove(fPrimeNums.begin(), fPrimeNums.end(), 0), fPrimeNums.end());


	/*
	for(int p = 0; p <= fPrimeNums.size(); p++) {

		std::cout << fPrimeNums[p] << std::endl;
	}
	*/
}

//the function that implements Rabin-Miller test.
//returns true if the primeCandidate probably prime and false if it isn't prime.
//The more rounds value, the more precise the result
/**
 * @brief
 * @param
 * @return
 */
bool RSA::millerRabinPassed(BigInt candidate, int rounds) {

	if (candidate < BigInt("2") || candidate % BigInt("2") == BigInt("0")) return false;
	if (candidate == BigInt("3")) return true;

	BigInt candidateOne("0");
	candidateOne = candidate - BigInt("1");
	BigInt divisionTwoRounds("0");
	BigInt multiplier("0");

	while(candidateOne % BigInt("2") == BigInt("0")) { //represent in the form of (2**divisionTwoRounds) * multiplier

		candidateOne /= BigInt("2");
		divisionTwoRounds += 1;

	}

	multiplier = (candidate - BigInt("1")) / (BigInt("2") ^ divisionTwoRounds);

	for (int i = 0; i < rounds; i++) {

		BigInt randomNum;
		BigInt x;
		bool flag = false;

		//generating a random number between 2 and candidate - 2:
		randomNum = BigInt("2") + BigInt(rand()) % (candidate - BigInt("2")  -  BigInt("2")  +  BigInt("1"));

		//calculating x = randomNum ^ multiplier % candidate
		x = modPow(randomNum, multiplier, candidate);

		if (x == BigInt("1") || x == candidate - BigInt("1")) {

			continue;

		}

		for (int j = 0; j < divisionTwoRounds - 1; j++) {

			x = modPow(x, BigInt("2"), candidate);
			if (x == BigInt("1")) continue;

			if (x == candidate - BigInt("1")) {

				flag = true;
				break;

			}

		}

		if (flag == true) continue;

		return false;

	}

	return true;

}

/**
 * @brief
 * @param
 * @return
 */
BigInt RSA::modPow(BigInt base, BigInt exponent, BigInt mod) {

	base %= mod;
	BigInt result("1");

	while (exponent > BigInt("0")) {

	    if ((exponent % 2) != BigInt("0")) result = (result * base) % mod;
	    base = (base * base) % mod;

	    exponent /= 2;

	}

	return result;
}

/**
 * @brief
 * @param
 * @return
 */
BigInt RSA::pqMultiplication(BigInt p, BigInt q) {

	return p * q;

}

/**
 * @brief
 * @param
 * @return
 */
BigInt RSA::EulerFunction(BigInt p, BigInt q) {

	return (p - BigInt("1")) * (q - BigInt("1"));

}

/**
 * @brief
 * @param
 * @return
 */
BigInt RSA::extendedGCD(BigInt a, BigInt b, signedBigInt &x, signedBigInt &y)
{
    if (b == BigInt("0")) {
        x.value = BigInt("1");
        x.sign  = 1;

        y.value = BigInt("0");
        y.sign  = 1;

        return a;
    }

    signedBigInt x1, y1;
    BigInt g;

    g = extendedGCD(b, a % b, x1, y1);

    x = y1;

    signedBigInt temp = mul(y1, a / b);
    y = sub(x1, temp);

    return g;

}



