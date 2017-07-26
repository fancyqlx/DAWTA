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
	try{
		HMAC< SHA256 > hmac((byte*)key.c_str(), key.length());

		StringSource(plain, true,
			new HashFilter(hmac,
				new StringSink(mac)
			) // HashFilter      
		); // StringSource
	}
	catch(const CryptoPP::Exception& e){
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

BigInteger hexToDecimal(std::string hex){
	std::map<char,BigInteger> mp = {
		{'a',10},{'b',11},{'c',12},{'d',13},{'e',14},{'f',15},
		{'A',10},{'B',11},{'C',12},{'D',13},{'E',14},{'F',15}
	};
	BigInteger ret;
	for(int i=0;i<hex.size();++i){
		if(isdigit(hex[i])){
			ret = ret * 16 + (hex[i]-'0');
		}
		else if(mp.count(hex[i])){
			ret = ret * 16 + mp[hex[i]];
		}else{
			printf("Error: hexToDecimal...\n");
			exit(0);
		}
	}
	return ret;
}
