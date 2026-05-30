#pragma once

//#include <bits/stdc++.h>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

class BigInt {
private:

	std::vector<int> vectorDigits;
	std::string digits;

public:

	BigInt (std::string &);
	BigInt (const char * s);
	BigInt (unsigned long int n = 0);
	BigInt (BigInt & obj);
	//BigInt();

	friend int bigIntLength(const BigInt &);

	friend void bigIntPrint(const BigInt &);
	friend bool Null(const BigInt &);
	friend void bigInt_divide_by_2(BigInt &);

	int length();

	int getString(const BigInt &);
	void fillVector();

	std::vector<int> getVectorDigits();


	BigInt &operator = (const BigInt &);

	friend bool operator == (const BigInt &, const BigInt &);
	friend bool operator < (const BigInt &, const BigInt &);
	friend bool operator > (const BigInt &, const BigInt &);
	friend bool operator <= (const BigInt &, const BigInt &);
	friend bool operator >= (const BigInt &, const BigInt &);
	friend bool operator != (const BigInt &, const BigInt &);

	friend BigInt &operator += (BigInt & num1, const BigInt & num2);
	friend BigInt operator + (const BigInt &, const BigInt &);

	friend BigInt &operator -= (BigInt & num1, const BigInt & num2);
	friend BigInt operator - (const BigInt & num1, const BigInt & num2);

	friend BigInt &operator *= (BigInt & num1, const BigInt & num2);
	friend BigInt operator * (const BigInt &, const BigInt &);

	friend BigInt &operator /= (BigInt &, const BigInt &);
	friend BigInt operator / (const BigInt &num1, const BigInt &num2);
	friend BigInt &operator %= (BigInt & num1, const BigInt & num2);
	friend BigInt operator % (const BigInt & num1, const BigInt & num2);

	friend BigInt &operator ^= (BigInt & num1, const BigInt & num2);
	friend BigInt operator ^ (const BigInt &num1, const BigInt &num2);

	int operator[] (const int index)const;

	friend std::ostream &operator << (std::ostream &, const BigInt &);
	friend std::istream &operator >> (std::istream &, BigInt &);


};

//structure for operating with signed BigInts
//(needed for extended Euclidius algorhitm)
struct signedBigInt {

	BigInt value;
	int sign;

};

signedBigInt mul(const signedBigInt &num1, const BigInt &num2); //multiply function
signedBigInt add(const signedBigInt &num1, const signedBigInt &num2); //add function
signedBigInt sub(const signedBigInt &num1, const signedBigInt &num2); //substitute function


