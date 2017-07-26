#include "dawta.hpp"

BigInteger generateY(BigInteger x){
    BigInteger ret = G;
    printf("begin generateY....\n");
    while(x != BigInteger(ZERO)){
        ret *= G;
        --x;
    }
    printf("end generateY....\n");
    return ret%P;
}

BigInteger generateRandom(){
    std::random_device rd;
    std::mt19937_64 e2(rd());
    std::uniform_int_distribution<long long int> dist(std::llround(std::pow(2,62)),std::llround(std::pow(2,63)-1));
    std::string ret = std::to_string(dist(e2));
    return BigInteger(stringToBigInteger(ret));
}

std::string hashFunc(std::string key, std::string plain){
    std::string mac, encoded;
    try
    {
            HMAC< SHA256 > hmac((byte*)key.c_str(), key.length());

            StringSource(plain, true,
                    new HashFilter(hmac,
                            new StringSink(mac)
                    ) // HashFilter      
            ); // StringSource
    }
    catch(const CryptoPP::Exception& e)
    {
            std::cerr << e.what() << std::endl;
    }

    encoded.clear();
    StringSource(mac, true,
            new Base64Encoder(
                    new StringSink(encoded)
            ) // Base64Encoder
    ); // StringSource
    std::cout << "encode: " << encoded << std::endl;
    return encoded;
}