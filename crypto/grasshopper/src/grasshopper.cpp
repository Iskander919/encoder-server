/*
 * grasshopper.cpp
 *
 *  Created on: 19 нояб. 2025 г.
 *      Author: iskan
 */

#include "../inc/grasshopper.h"

Grasshopper::Grasshopper() {

	//key 1:
	key_1 = {

			(char)0x88, (char)0x99, (char)0xAA, (char)0xBB,
			(char)0xCC, (char)0xDD, (char)0xEE, (char)0xFF,
			(char)0x00, (char)0x11, (char)0x22, (char)0x33,
			(char)0x44, (char)0x55, (char)0x66, (char)0x77

	};


	key_2 = {

			(char)0xFE, (char)0xDC, (char)0xBA, (char)0x98,
			(char)0x76, (char)0x54, (char)0x32, (char)0x10,
			(char)0x01, (char)0x23, (char)0x45, (char)0x67,
			(char)0x89, (char)0xAB, (char)0xCD, (char)0xEF

	};

	keyChain = grasshopperExpandKey(key_1, key_2);

}

//function X:
std::vector<char> Grasshopper::grasshopperX(std::vector<char> &aVect, std::vector<char> &bVect) {

	std::vector<char> result(blocSize);

	for(int i = 0; i < blocSize; i++) {

		result[i] = (char)(aVect[i] ^ bVect[i]);

	}

	return result;

}

//function S
std::vector<char> Grasshopper::grasshopperS(std::vector<char> inData, bool reverse = false) {

	std::vector<char> result(inData.size());

	for (int i = 0; i < blocSize; i++) {

		int data = (int)inData[i];

		if (data < 0) data += 256;

		if (reverse == false) result[i] = pi[data];

		else result[i] = reversePi[data];

	}

	return result;

}


//multiplication in Galua fields function
char Grasshopper::mul(char a, char b) {

	char result = 0, highBit;

	for (int i = 0; i < 8; i++) {

		if((b & 1) == 1) result ^= a;

		highBit = (char) (a & 0x80);

		a <<= 1;

		if(highBit < 0) a ^= 0xC3;

		b >>= 1;

	}

	return result;

}


//function R
std::vector<char> Grasshopper::grasshopperR(std::vector<char> state) {

	char a15 = 0;
	std::vector<char> internal(16, 0);

	for (int i = 0; i < 16; i++) a15 ^= mul(state[i], linVect[i]);

	for (int i = 15; i > 0; i--) {

		internal[i] = state[i - 1];

	}

	internal[0] = a15;
	return internal;

}



//function L
std::vector<char> Grasshopper::grasshopperL(std::vector<char> inData, bool reverse = false) {

	std::vector<char> result(inData.size());
	std::vector<char> internal = inData;

	for(int i = 0; i < 16; i++) {

		if(reverse == false) internal = grasshopperR(internal);

		//else internal = grasshopperReverseR(internal);
		else internal = grasshopperReverseR(internal);

	}

	result = internal;

	return result;

}

//function R^(-1)
/*
std::vector<char> grasshopperReverseR(std::vector<char> state) {

	char a0 = state[15];
	std::vector<char> internal(16);

	for(int i = 0; i < 16; i++) a0 ^= (char)(mul(state[i], linVect[i]));

	for (int i = 1; i < 16; i++) {

		internal[i] = state[i - 1];

	}

	internal[0] = a0;
	return internal;

}
*/

std::vector<char> Grasshopper::grasshopperReverseR(const std::vector<char>& in) {
    //if (in.size() != 16) throw std::runtime_error("R_inv: input must be 16 bytes");

    std::array<uint8_t,16> b;

    for (int i = 0; i < 16; ++i) b[i] = static_cast<uint8_t>(in[i]);

    std::array<uint8_t,16> a;

    for (int i = 0; i < 15; ++i)
        a[i] = b[i+1];


    uint8_t acc = b[0];

    for (int i = 0; i < 15; ++i) acc ^= mul(linVect[i], static_cast<char>(a[i]));

    a[15] = acc;

    std::vector<char> out(16);

    for (int i = 0; i < 16; ++i) out[i] = static_cast<char>(a[i]);

    return out;
}

//getting constants function:
/*
void getConsts() {

	std::vector<std::vector<char>> iterNum(32, std::vector<char>(16));

	for(int i = 0; i < 32; i++) {

		for(int j = 0; j < blocSize; j++) { iterNum[i][j] = 0; }

		iterNum[i][0] =(char) (i+1);

	}

	for(int i = 0; i < 32; i++) {

		iterC[i] = grasshopperL(iterNum[i], false);

	}

}
*/

std::vector<std::vector<char>> Grasshopper::_getConsts() {

	std::vector<std::vector<char>> iterConst(32, std::vector<char>(16, 0));

	for(int i = 0; i < 32; i++) {

		std::vector<char> bloc(16, 0);
		bloc[15] = (char)(i + 1);

		bloc = grasshopperL(bloc, false);

		iterConst[i] = bloc;

	}

	return iterConst;

}

//Feistel slots transform function
std::vector<std::vector<char>> Grasshopper::grasshopperF(std::vector<char> inKey1, std::vector<char> inKey2, std::vector<char> iterConst) {

	std::vector<char> internal;
	std::vector<char> outKey2 = inKey1;

	internal = grasshopperX(inKey1, iterConst);
	internal = grasshopperS(internal, false);
	internal = grasshopperL(internal, false);


	std::vector<char> outKey1 = grasshopperX(internal, inKey2);

	std::vector<std::vector<char>> key(2);
	key[0] = outKey1;
	key[1] = outKey2;

	return key;

}

//round keys calculating function
std::vector<std::vector<char>> Grasshopper::grasshopperExpandKey(std::vector<char> key1, std::vector<char> key2) {

	std::vector<std::vector<char>> keyChain(10, std::vector<char>(64));

	std::vector<std::vector<char>> iter12(2);
	std::vector<std::vector<char>> iter34(2);

	std::vector<std::vector<char>> consts(32, std::vector<char>(16));
	consts = _getConsts();

	keyChain[0] = key1;
	keyChain[1] = key2;

	iter12[0] = key1;
	iter12[1] = key2;

	for(int i = 0; i < 4; i++) {

		iter34 = grasshopperF(iter12[0], iter12[1], consts[0 + 8 * i]);
		iter12 = grasshopperF(iter34[0], iter34[1], consts[1 + 8 * i]);
		iter34 = grasshopperF(iter12[0], iter12[1], consts[2 + 8 * i]);
		iter12 = grasshopperF(iter34[0], iter34[1], consts[3 + 8 * i]);
		iter34 = grasshopperF(iter12[0], iter12[1], consts[4 + 8 * i]);
		iter12 = grasshopperF(iter34[0], iter34[1], consts[5 + 8 * i]);
		iter34 = grasshopperF(iter12[0], iter12[1], consts[6 + 8 * i]);
		iter12 = grasshopperF(iter34[0], iter34[1], consts[7 + 8 * i]);

		keyChain[2 * i + 2] = iter12[0];
		keyChain[2 * i + 3] = iter12[1];

	}

	return keyChain;

}


//encrypt function
std::vector<char> Grasshopper::grasshopperEncrypt(std::vector<char> bloc) {

	//std::vector<std::vector<char>> keyChain = grasshopperExpandKey(key_1, key_2);


	std::vector<char> outBloc(blocSize);
	outBloc = bloc;

	for(int i = 0; i < 9; i++) {

		outBloc = grasshopperX(keyChain[i], outBloc);
		outBloc = grasshopperS(outBloc, false);
		outBloc = grasshopperL(outBloc, false);

	}

	outBloc = grasshopperX(outBloc, keyChain[9]);

	return outBloc;

}

//decrypt function
std::vector<char> Grasshopper::grasshopperDecrypt(std::vector<char> bloc) {

	std::vector<char> outBloc(blocSize);
	outBloc = bloc;

	outBloc = grasshopperX(outBloc, keyChain[9]);

	for(int i = 8; i >= 0; i--) {

		outBloc = grasshopperL(outBloc, true);
		outBloc = grasshopperS(outBloc, true);

		outBloc = grasshopperX(keyChain[i], outBloc);

	}

	return outBloc;

}

/**
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

//function X:
std::vector<char> grasshopperX(std::vector<char> &aVect, std::vector<char> &bVect) {

	std::vector<char> result(blocSize);

	for(int i = 0; i < blocSize; i++) {

		result[i] = (char)(aVect[i] ^ bVect[i]);

	}

	return result;

}

//function S
std::vector<char> grasshopperS(std::vector<char> inData, bool reverse = false) {

	std::vector<char> result(inData.size());

	for (int i = 0; i < blocSize; i++) {

		int data = (int)inData[i];

		if (data < 0) data += 256;

		if (reverse == false) result[i] = pi[data];

		else result[i] = reversePi[data];

	}

	return result;

}


//multiplication in Galua fields function
char mul(char a, char b) {

	char result = 0, highBit;

	for (int i = 0; i < 8; i++) {

		if((b & 1) == 1) result ^= a;

		highBit = (char) (a & 0x80);

		a <<= 1;

		if(highBit < 0) a ^= 0xC3;

		b >>= 1;

	}

	return result;

}


//function R
std::vector<char> grasshopperR(std::vector<char> state) {

	char a15 = 0;
	std::vector<char> internal(16, 0);

	for (int i = 0; i < 16; i++) a15 ^= mul(state[i], linVect[i]);

	for (int i = 15; i > 0; i--) {

		internal[i] = state[i - 1];

	}

	internal[0] = a15;
	return internal;

}



//function L
std::vector<char> grasshopperL(std::vector<char> inData, bool reverse = false) {

	std::vector<char> result(inData.size());
	std::vector<char> internal = inData;

	for(int i = 0; i < 16; i++) {

		if(reverse == false) internal = grasshopperR(internal);

		//else internal = grasshopperReverseR(internal);
		else internal = grasshopperReverseR(internal);

	}

	result = internal;

	return result;

}

//function R^(-1)
/*
std::vector<char> grasshopperReverseR(std::vector<char> state) {

	char a0 = state[15];
	std::vector<char> internal(16);

	for(int i = 0; i < 16; i++) a0 ^= (char)(mul(state[i], linVect[i]));

	for (int i = 1; i < 16; i++) {

		internal[i] = state[i - 1];

	}

	internal[0] = a0;
	return internal;

}
*/

std::vector<char> grasshopperReverseR(const std::vector<char>& in) {
    //if (in.size() != 16) throw std::runtime_error("R_inv: input must be 16 bytes");

    std::array<uint8_t,16> b;

    for (int i = 0; i < 16; ++i) b[i] = static_cast<uint8_t>(in[i]);

    std::array<uint8_t,16> a;

    for (int i = 0; i < 15; ++i)
        a[i] = b[i+1];


    uint8_t acc = b[0];

    for (int i = 0; i < 15; ++i) acc ^= mul(linVect[i], static_cast<char>(a[i]));

    a[15] = acc;

    std::vector<char> out(16);

    for (int i = 0; i < 16; ++i) out[i] = static_cast<char>(a[i]);

    return out;
}

//getting constants function:
/*
void getConsts() {

	std::vector<std::vector<char>> iterNum(32, std::vector<char>(16));

	for(int i = 0; i < 32; i++) {

		for(int j = 0; j < blocSize; j++) { iterNum[i][j] = 0; }

		iterNum[i][0] =(char) (i+1);

	}

	for(int i = 0; i < 32; i++) {

		iterC[i] = grasshopperL(iterNum[i], false);

	}

}
*/

std::vector<std::vector<char>> _getConsts() {

	std::vector<std::vector<char>> iterConst(32, std::vector<char>(16, 0));

	for(int i = 0; i < 32; i++) {

		std::vector<char> bloc(16, 0);
		bloc[15] = (char)(i + 1);

		bloc = grasshopperL(bloc, false);

		iterConst[i] = bloc;

	}

	return iterConst;

}

//Feistel slots transform function
std::vector<std::vector<char>> grasshopperF(std::vector<char> inKey1, std::vector<char> inKey2, std::vector<char> iterConst) {

	std::vector<char> internal;
	std::vector<char> outKey2 = inKey1;

	internal = grasshopperX(inKey1, iterConst);
	internal = grasshopperS(internal, false);
	internal = grasshopperL(internal, false);


	std::vector<char> outKey1 = grasshopperX(internal, inKey2);

	std::vector<std::vector<char>> key(2);
	key[0] = outKey1;
	key[1] = outKey2;

	return key;

}

//round keys calculating function
std::vector<std::vector<char>> grasshopperExpandKey(const std::vector<char> key1, const std::vector<char> key2) {

	std::vector<std::vector<char>> keyChain(10, std::vector<char>(64));

	std::vector<std::vector<char>> iter12(2);
	std::vector<std::vector<char>> iter34(2);

	std::vector<std::vector<char>> consts(32, std::vector<char>(16));
	consts = _getConsts();

	keyChain[0] = key1;
	keyChain[1] = key2;

	iter12[0] = key1;
	iter12[1] = key2;

	for(int i = 0; i < 4; i++) {

		iter34 = grasshopperF(iter12[0], iter12[1], consts[0 + 8 * i]);
		iter12 = grasshopperF(iter34[0], iter34[1], consts[1 + 8 * i]);
		iter34 = grasshopperF(iter12[0], iter12[1], consts[2 + 8 * i]);
		iter12 = grasshopperF(iter34[0], iter34[1], consts[3 + 8 * i]);
		iter34 = grasshopperF(iter12[0], iter12[1], consts[4 + 8 * i]);
		iter12 = grasshopperF(iter34[0], iter34[1], consts[5 + 8 * i]);
		iter34 = grasshopperF(iter12[0], iter12[1], consts[6 + 8 * i]);
		iter12 = grasshopperF(iter34[0], iter34[1], consts[7 + 8 * i]);

		keyChain[2 * i + 2] = iter12[0];
		keyChain[2 * i + 3] = iter12[1];

	}

	return keyChain;

}


//encrypt function
std::vector<char> grasshopperEncrypt(std::vector<char> bloc, std::vector<std::vector<char>> keyChain) { //std::vector<std::vector<char>> keyChain) {


	std::vector<char> outBloc(blocSize);
	outBloc = bloc;

	for(int i = 0; i < 9; i++) {

		outBloc = grasshopperX(keyChain[i], outBloc);
		outBloc = grasshopperS(outBloc, false);
		outBloc = grasshopperL(outBloc, false);

	}

	outBloc = grasshopperX(outBloc, keyChain[9]);

	return outBloc;

}

//decrypt function
std::vector<char> grasshopperDecrypt(std::vector<char> bloc, std::vector<std::vector<char>> keyChain) {

	std::vector<char> outBloc(blocSize);
	outBloc = bloc;

	outBloc = grasshopperX(outBloc, keyChain[9]);

	for(int i = 8; i >= 0; i--) {

		outBloc = grasshopperL(outBloc, true);
		outBloc = grasshopperS(outBloc, true);

		outBloc = grasshopperX(keyChain[i], outBloc);

	}

	return outBloc;

}

unsigned char Grasshopper::hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    throw std::invalid_argument("Invalid hex character");
}

std::vector<char> Grasshopper::hex_to_vector(const std::string& hex) {
    if (hex.size() % 2 != 0)
        throw std::invalid_argument("Hex string length must be even");

    std::vector<char> result;
    result.reserve(hex.size() / 2);

    for (size_t i = 0; i < hex.size(); i += 2) {
        unsigned char high = hex_value(hex[i]);
        unsigned char low  = hex_value(hex[i + 1]);
        result.push_back(static_cast<char>((high << 4) | low));
    }

    return result;
}






