/*
 * stribog.cpp
 *
 *  Created on: 27 нояб. 2025 г.
 *      Author: iskan
 */


#include "../inc/stribog.h"
#include "../inc/stribog_consts.h"

//
std::vector<uint8_t> Stribog::add(std::vector<uint8_t> num1, std::vector<uint8_t> num2) {

	std::vector<uint8_t> temp(64);

	int carry = 0;

	std::vector<uint8_t> tempA(64);
	std::vector<uint8_t> tempB(64);

	std::copy( num1.begin(), num1.end(), tempA.begin() + (64 - num1.size() ) );
	std::copy( num2.begin(), num2.end(), tempB.begin() + (64 - num2.size() ) );

	for(int i = 63; i >= 0; i--) {

		int sum = tempA[i] + tempB[i] + carry;

		temp[i] = (uint8_t)(sum & 0xFF);

		carry = (sum >> 8);

	}

	return temp;

}

//X-function
std::vector<uint8_t> Stribog::stribogX(const std::vector<uint8_t> input1,
		const std::vector<uint8_t> input2) {

	std::vector<uint8_t> result(64);

	for(int i = 0; i < 64; i++) {

		result[i] = input1[i] ^ input2[i];

	}

	return result;

}

std::vector<uint8_t> Stribog::stribogX(const std::vector<uint8_t> input1,
		const uint8_t input2[12][64], int index) {

	std::vector<uint8_t> result(64);

	for(int i = 0; i < 64; i++) {

		result[i] = input1[i] ^ input2[index][i];

	}

	return result;

}




std::vector<uint8_t> Stribog::stribogS(const std::vector<uint8_t> &input) {

	std::vector<uint8_t> result(64);

	for(int i = 0; i < 64; i++) {

		result[i] = pi[input[i]];

	}

	return result;

}

//P-function
std::vector<uint8_t> Stribog::stribogP(const std::vector<uint8_t> &input) {

	std::vector<uint8_t> result(64);

	for(int i = 0; i < 64; i++) {

		result[i] = input[tau[i]];

	}

	return result;

}


//L-function
//copies input to tempArray and then reverses it. Then copies all tempArray bits to tempBits and reverses tempBits.
//Then iterates tempBits and does XOR operation of t with matrixA if meets 1 in tempBits.
//Then compiles bytes of t to resultComponent. Copies resultComponent to result. Returns result.

std::vector<uint8_t> Stribog::stribogL(const std::vector<uint8_t> &input)
{

	std::vector<uint8_t> result(64);

	for(uint8_t i = 0; i < 8; i++) {

		uint64_t t = 0;

		std::vector<uint8_t> tempArray(8);

		std::copy(input.data() + i * 8, input.data() + i * 8 + 8, tempArray.data());

		std::reverse(tempArray.begin(), tempArray.end());

		std::vector<bool> tempBits(64);

		for(uint8_t byte : tempArray) {

			for(int bit = 0; bit < 8; bit ++) {

				tempBits.push_back((byte >> bit) & 1); //converting tempArray to array of bits

			}
		}

		std::reverse(tempBits.begin(), tempBits.end());

		for(size_t j = 0; j < tempBits.size(); j++) {

			if(tempBits[j] != false) {

				t ^= matrixA[j];

			}

		}

		uint8_t resultComponent[8];

		for(int k = 0; k < 8; k++) {

			resultComponent[7 - k] = (t >> (k * 8)) & 0xFF; //getting bytes of t

		}

		std::copy(resultComponent, resultComponent + 8, result.begin() + i*8);

	}

	return result;
}


//getting round keys function
std::vector<uint8_t> Stribog::stribogGetKeys(std::vector<uint8_t> key, int roundNum) {

	std::vector<uint8_t> result = key;

	result = stribogX(key, consts, roundNum);
	result = stribogS(result);
	result = stribogP(result);
	result = stribogL(result);


	return result;

}

std::vector<uint8_t> Stribog::stribogE(std::vector<uint8_t> key, const std::vector<uint8_t> msg) {

	std::vector<uint8_t> state = stribogX(key, msg); //doing XOR operation between key and msg

	for(int i = 0; i < 12; i++) {

		state = stribogS(state);
		state = stribogP(state);
		state = stribogL(state);
		key   = stribogGetKeys(key, i);

		state = stribogX(state, key);

	}

	return state;

}

//Gn compression function
//what is h? what is N?
std::vector<uint8_t> Stribog::stribogG(std::vector<uint8_t> N, std::vector<uint8_t> h, std::vector<uint8_t> msg) {

	std::vector<uint8_t> temp(64);
	temp = stribogX(h, N);

	temp = stribogS(temp);
	temp = stribogP(temp);
	temp = stribogL(temp);

	std::vector<uint8_t> eTransf = stribogE(temp, msg);

	eTransf  = stribogX(eTransf, h);

	std::vector<uint8_t> result = stribogX(eTransf, msg);


	return result;

}

std::vector<uint8_t> Stribog::stribogGetHash(std::vector<uint8_t> msg, int msgLength) {

	std::vector<uint8_t> h(64, 0);
	std::vector<uint8_t> N(64, 0);
	const std::vector<uint8_t> N0(64, 0);

	std::vector<uint8_t> padded(64, 0);
	padded[63] = 0x01;

	std::vector<uint8_t> sigma(64, 0);

	h = stribogG(N, h, msg);

	uint64_t bitLen = static_cast<uint64_t>(msg.size()) * 8;

	std::vector<uint8_t> mesLen(8, 0);

	for (int i = 0; i < 8; i++) {

	    mesLen[i] = (bitLen >> (i * 8)) & 0xFF;   // little-endian

	}

	std::reverse(mesLen.begin(), mesLen.end());

	N = add(N, mesLen);

	sigma = add(sigma, padded);

	h = stribogG(N0, h, N);
	h = stribogG(N0, h, sigma);

	return h;

}

std::vector<uint8_t> Stribog::getHash(const std::vector<uint8_t> message)
{

    std::vector<uint8_t> paddedMes(64, 0);
    std::vector<uint8_t> h(64);
    std::vector<uint8_t> N(64);
    std::vector<uint8_t> sigma(64);

    // padding
    if (message.size() == 63) {

    	for(int i = 1; i < 64; i++) {

    		paddedMes[i] = message[i - 1];

    	}

    	paddedMes[0] = 0x01;

    }

    else if(message.size() < 63) {

    	int index = 64 - message.size() - 1;
    	paddedMes[index] = 0x01;

    }

    else paddedMes = message;

    const std::vector<uint8_t> N_0(64, 0x00);

    h = stribogG(N_0, h, paddedMes); //ok

    N = {

        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x01,
        0xF8

    };

    sigma = add(sigma, paddedMes);

    h = stribogG(N_0, h, N);
    h = stribogG(N_0, h, sigma);

    return h;

}

uint8_t Stribog::hex_value(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    throw std::invalid_argument("Invalid hex character");
}

std::vector<uint8_t> Stribog::hex_to_vec8(const std::string &hex)
{
    if (hex.size() % 2 != 0)
        throw std::invalid_argument("Hex string must have even length");

    std::vector<uint8_t> result;
    result.reserve(hex.size() / 2);

    for (size_t i = 0; i < hex.size(); i += 2) {
        uint8_t hi = hex_value(hex[i]);
        uint8_t lo = hex_value(hex[i + 1]);
        result.push_back((hi << 4) | lo);
    }

    return result;
}





