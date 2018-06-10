#ifndef DAWTA_HPP
#define DAWTA_HPP

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <poll.h>
#include <sys/stropts.h>
#include <sys/timerfd.h>
#include <time.h>
#include <math.h>

#include <string>
#include <cstring>
#include <iostream>
using std::cout;
using std::endl;
using std::cin;

#include <vector>
using std::vector;
#include <queue>
#include <bitset>
#include <algorithm>
#include <map>
#include <fstream>
#include <utility>
#include <tuple>
using std::ofstream;
using std::ifstream;

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>
#include <stdexcept>
#include <chrono>

#include <random>
#include <assert.h>

#include "BigIntegerLibrary.hh"

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

#include "Connection.hpp"


const BigInteger P = stringToBigInteger(std::string("340282363487254643170864374573732807431"));
const BigInteger G = BigInteger(2);
const int ZERO = 0;

extern int N;
extern BigInteger M;
extern int bits;
extern int K;

BigInteger generateY(BigInteger x);

BigInteger generateRandom();
BigInteger generateRandom(int bits);
        
std::string hashFunc(std::string key, std::string plain);

BigInteger hexToDecimal(std::string hex);

std::pair<size_t,long double> recursion(std::vector<std::pair<unsigned long long int,unsigned long long int>> &range, 
				const std::map<std::shared_ptr<socketx::Connection>, std::vector<std::string>> &stage3_map,
				unsigned long long int left, unsigned long long int right, int num);


size_t anti_recursion(std::vector<std::pair<unsigned long long int,unsigned long long int>> &range, 
                const std::map<std::shared_ptr<socketx::Connection>, std::vector<std::string>> &stage3_map,
                unsigned long long int left, unsigned long long int right, int num);


std::vector<BigInteger> simulateStage4(const std::map<std::shared_ptr<socketx::Connection>, std::vector<std::string>> &stage3_map, 
							const std::map<int, std::shared_ptr<socketx::Connection>> &stage4_map, 
							size_t &bitComplexity, long double &time_us);

std::tuple<int,int,int,BigInteger> readConfig();

std::vector<int> prime();

#endif