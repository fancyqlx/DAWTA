#include "dawta.hpp"

auto config = readConfig();
int N = std::get<0>(config);
int bits = std::get<1>(config);
BigInteger M = std::get<2>(config);

std::tuple<int,int,BigInteger> readConfig(){
    ifstream fin("./src/config",ifstream::in);
    int N;
    BigInteger M;
    int bits;

    std::string line;
    getline(fin,line);
    size_t pos = line.find("=");
    std::string value = line.substr(pos+1);
    N = std::stoi(value);

    getline(fin,line);
    pos = line.find("=");
    value = line.substr(pos+1);
    bits = std::stoi(value); 
    M = generateRandom(std::stoi(value));

    return std::make_tuple(N,bits,M);
}



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
    std::uniform_int_distribution<unsigned long long int> dist(static_cast<unsigned long long>(std::pow(2,63)),
                                                                static_cast<unsigned long long>(std::pow(2,64)-1));
    std::string ret = std::to_string(dist(e2));
    return BigInteger(stringToBigInteger(ret));
}



BigInteger generateRandom(int bits){
    std::random_device rd;
    std::mt19937_64 e2(rd());
    std::uniform_int_distribution<unsigned long long int> dist(static_cast<unsigned long long>(std::pow(2,bits-1))
                                                                ,static_cast<unsigned long long>(std::pow(2,bits)-1));
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
        new HexEncoder(
            new StringSink(encoded)
        ) // HexEncoder
    ); // StringSource
    //std::cout << "encode: " << encoded << std::endl;
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



size_t recursion(std::vector<std::pair<unsigned long long int,unsigned long long int>> &range, 
                const std::map<std::shared_ptr<socketx::Connection>, std::vector<std::string>> &stage3_map,
                unsigned long long int left, unsigned long long int right, int num){

    if(num<1 || right-left<1) return 0;
    size_t bitComplexity = 0;

    std::map<std::shared_ptr<socketx::Connection>, std::vector<BigInteger>> crypto_map;
    auto interval = static_cast<unsigned long long>((right-left)/num);
    //cout<<"left="<<left<<" Interval="<<interval<<" "<<"num="<<num<<endl;

    unsigned long long int point = static_cast<unsigned long long>(log2(N)/log2(log2(N)));
    if(point < 3) point = 3;

    for(auto it=stage3_map.begin();it!=stage3_map.end();++it){
        auto randomNum = std::stoull((it->second)[0]);
        auto key1 = (it->second)[1];
        auto key2 = (it->second)[2];

        /*Simulate encryption*/
        unsigned long long int left_s = left, right_s = left + interval;
        std::vector<int> vec(num,0);
        for(int i=0;i<num;++i){
            if(randomNum>=left_s && randomNum<=right_s){
                vec[i] = 1;
                break;
            }
            left_s += interval;
            right_s += interval;
        }

        std::string hashCode1 = hashFunc(key1,std::to_string(num));
        std::string hashCode2 = hashFunc(key2,std::to_string(num));
        BigInteger F = (hexToDecimal(hashCode1)-hexToDecimal(hashCode2));
        std::string cryptoStr = "";
        for(int i=0;i<num;++i){
            if(vec[i]){
                cryptoStr += bigIntegerToString((F+1)%M) + " ";
            }
            else 
                cryptoStr += bigIntegerToString(F%M) + " ";
        }
        bitComplexity += cryptoStr.size()*8;

        /*
        * Simulate decryption
        * Construct vector of cryptoStr
        */
        std::vector<BigInteger> crypto_vec;
        size_t begin = 0;
        while(begin!=cryptoStr.size()){
            auto pos = cryptoStr.find(" ", begin);
            std::string str = cryptoStr.substr(begin,pos-begin);
            crypto_vec.push_back(stringToBigInteger(str));
            begin = pos + 1;
        }
        crypto_map[it->first] = crypto_vec;
    }
    
    /*Compute the ball of each partition*/
    std::vector<BigInteger> sum_vec(num, BigInteger());
    for(int i=0;i<num;++i){
        for(auto it_map = crypto_map.begin();it_map!=crypto_map.end();++it_map){
            sum_vec[i] += (it_map->second)[i];
        }
        sum_vec[i] %= M;
        //cout<<"recursion: sum_vec = "<<sum_vec[i]<<endl;
        if(sum_vec[i]==BigInteger()) continue;
        else if(sum_vec[i]==BigInteger(1)){
            range.push_back({left+i*interval, left+(i+1)*interval});
        }
        else if(sum_vec[i]>=BigInteger(2) && sum_vec[i]<=stringToBigInteger(std::to_string(point))){
            bitComplexity += recursion(range, stage3_map, left+i*interval, left+(i+1)*interval, static_cast<int>(ceil(std::pow(log2(N),4.0/3))));
        }
        else{
            bitComplexity += recursion(range, stage3_map, left+i*interval, left+(i+1)*interval, static_cast<int>(ceil(std::pow(log2(N),3))));
        }
    }
    return bitComplexity;
}



std::vector<BigInteger> simulateStage4(const std::map<std::shared_ptr<socketx::Connection>, std::vector<std::string>> &stage3_map, 
                            const std::map<int, std::shared_ptr<socketx::Connection>> &stage4_map, size_t &bitComplexity){
    /*Return vector*/
    std::vector<BigInteger> ret;

    /*Simulate N rounds*/
    for(int i=1;i<=N;++i){

        BigInteger oneRound = BigInteger();

        for(auto it=stage3_map.begin();it!=stage3_map.end();++it){
            auto randomNum = std::stoull((it->second)[0]);
            auto key1 = (it->second)[1];
            auto key2 = (it->second)[2];

            /*Simulate encryption*/

            std::string hashCode1 = hashFunc(key1,std::to_string(i));
            std::string hashCode2 = hashFunc(key2,std::to_string(i));
            BigInteger F = (hexToDecimal(hashCode1)-hexToDecimal(hashCode2));
            std::string cryptoStr = "";
            
            std::string randomNumStr = std::to_string(randomNum);
            assert(stage4_map.count(i));
            if(stage4_map.at(i) == (it->first))
                cryptoStr += bigIntegerToString((F+stringToBigInteger(randomNumStr))%M);
            else 
                cryptoStr += bigIntegerToString(F%M);
            
            bitComplexity += cryptoStr.size()*8;

            /*
            * Simulate decryption
            * Get the result of one round
            */
            oneRound += stringToBigInteger(cryptoStr);
        }
        oneRound %= M;
        ret.push_back(oneRound);
    }
    return ret;
}



size_t anti_recursion(std::vector<std::pair<unsigned long long int,unsigned long long int>> &range, 
                const std::map<std::shared_ptr<socketx::Connection>, std::vector<std::string>> &stage3_map,
                unsigned long long int left, unsigned long long int right, int num){

    if(num<1 || right-left<1) return 0;
    size_t bitComplexity = 0;

    std::map<std::shared_ptr<socketx::Connection>, std::vector<BigInteger>> crypto_map;
    auto interval = static_cast<unsigned long long>((right-left)/num);
    //cout<<"left="<<left<<" Interval="<<interval<<" "<<"num="<<num<<endl;

    unsigned long long int point = static_cast<unsigned long long>(log2(N)/log2(log2(N)));
    if(point < 3) point = 3;

    for(auto it=stage3_map.begin();it!=stage3_map.end();++it){
        auto randomNum = std::stoull((it->second)[0]);
        auto key1 = (it->second)[1];
        auto key2 = (it->second)[2];

        /*Simulate encryption*/
        unsigned long long int left_s = left, right_s = left + interval;
        std::vector<int> vec(num,0);
        for(int i=0;i<num;++i){
            if(randomNum>=left_s && randomNum<=right_s){
                vec[i] = 1;
                break;
            }
            left_s += interval;
            right_s += interval;
        }

        std::string hashCode1 = hashFunc(key1,std::to_string(num));
        std::string hashCode2 = hashFunc(key2,std::to_string(num));
        BigInteger F = (hexToDecimal(hashCode1)-hexToDecimal(hashCode2));
        std::string cryptoStr = "";
        for(int i=0;i<num;++i){
            if(vec[i]){
                cryptoStr += bigIntegerToString((F+1)%M) + " ";
            }
            else 
                cryptoStr += bigIntegerToString(F%M) + " ";
        }

        /*
        * Simulate decryption
        * Construct vector of cryptoStr
        */
        std::vector<BigInteger> crypto_vec;
        size_t begin = 0;
        while(begin!=cryptoStr.size()){
            auto pos = cryptoStr.find(" ", begin);
            std::string str = cryptoStr.substr(begin,pos-begin);
            crypto_vec.push_back(stringToBigInteger(str));
            begin = pos + 1;
        }
        crypto_map[it->first] = crypto_vec;
    }
    
    /*Compute the ball of each partition*/
    std::vector<BigInteger> sum_vec(num, BigInteger());
    for(int i=0;i<num;++i){
        for(auto it_map = crypto_map.begin();it_map!=crypto_map.end();++it_map){
            sum_vec[i] += (it_map->second)[i];
        }
        sum_vec[i] %= M;
        //cout<<"recursion: sum_vec = "<<sum_vec[i]<<endl;
        if(sum_vec[i]==BigInteger()) continue;
        else if(sum_vec[i]==BigInteger(1)){
            range.push_back({left+i*interval, left+(i+1)*interval});
        }
        else if(sum_vec[i]>=BigInteger(2) && sum_vec[i]<=stringToBigInteger(std::to_string(point))){
            anti_recursion(range, stage3_map, left+i*interval, left+(i+1)*interval, static_cast<int>(ceil(std::pow(log2(N),4.0/3))));
        }
        else{
            anti_recursion(range, stage3_map, left+i*interval, left+(i+1)*interval, static_cast<int>(ceil(std::pow(log2(N),3))));
        }
    }
    return bitComplexity;
}
