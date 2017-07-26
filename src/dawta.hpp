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
#include <queue>
#include <bitset>
#include <algorithm>
#include <map>
#include <fstream>
using std::ofstream;

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

#include "BigIntegerLibrary.hh"

#include "cryptlib.h"
using CryptoPP::Exception;

#include "hmac.h"
using CryptoPP::HMAC;

#include "sha.h"
using CryptoPP::SHA256;

#include "base64.h"
using CryptoPP::Base64Encoder;

#include "filters.h"
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::HashFilter;


const BigInteger P = stringToBigInteger(std::string("340282363487254643170864374573732807431"));
const BigInteger G = BigInteger(2);
const int ZERO = 0;
const int N = 3;

BigInteger generateY(BigInteger x);

BigInteger generateRandom();
        
std::string hashFunc(std::string key, std::string plain);

BigInteger hexToDecimal(std::string hex);

#endif