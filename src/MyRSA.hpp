/*
 * MyRSA.h
 *
 *  Created on: 2013-3-7
 *      Author: wzb
 */

#ifndef MYRSA_HPP_
#define MYRSA_HPP_

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <iostream>
#include <string>

#include "rsa.h"
#include "hex.h"
#include "osrng.h"
#include "files.h"
#include "md5.h"
#include "cryptlib.h"
using CryptoPP::Exception;
#include "hmac.h"
using CryptoPP::HMAC;
#include "sha.h"
using CryptoPP::SHA256;
#include "base64.h"
using CryptoPP::Base64Encoder;
#include "hex.h"
using CryptoPP::HexEncoder;
#include "filters.h"
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::HashFilter;
using CryptoPP::byte;


using namespace std;
using namespace CryptoPP;

class MyRSA
{
public:
	MyRSA();
	~MyRSA();

	std::string privString;
	std::string pubString;

	string MD5(const char * message);
	string MD5File(const char * filename);

	void GenerateRSAKey(unsigned int keyLength);

	string Encrypt(const char * message);
	string Decrypt(const char * cipher);
private:
	AutoSeededRandomPool _rng;
};

#endif /* MYRSA_HPP_ */