#include "EventLoop.hpp"
#include "Server.hpp"
#include "Threadx.hpp"
#include "dawta.hpp"
#include "MyRSA.hpp"

/* auto config = readConfig();
int N = config.first;
BigInteger M = config.second; */


long double  time_ms = 0; // Milliseconds
time_t  time_s;  // Seconds
struct timespec time_spec;
struct timespec time_spec_;

std::vector<std::shared_ptr<socketx::Connection>> connectionList;
std::map<std::shared_ptr<socketx::Connection>, socketx::Message> stage1_map;
std::map<std::shared_ptr<socketx::Connection>, std::vector<BigInteger>> stage2_map;
std::map<std::shared_ptr<socketx::Connection>, std::vector<std::string>> stage3_map;
std::vector<std::pair<unsigned long long int,unsigned long long int>> range;
std::map<int, std::shared_ptr<socketx::Connection>> stage4_map;
std::vector<BigInteger> finalResults;
size_t bitComplexity= 0;



int stage1(std::shared_ptr<socketx::Connection> conn){
    socketx::Message msg = conn->recvmsg();
    if(msg.getSize()==0){
        conn->handleClose();
        return 1;
    }
    /*bitComplexity*/
    bitComplexity += msg.getSize() * 8;

    /*Send message to the client next to it
    * stage increases to 2 after finishing it.
    */
    stage1_map[conn] = msg;
    /* std::ofstream fout("./data/aggregator_logs", std::ofstream::out | std::ofstream::app);
    fout<<"fd = "<<conn->getFD()<<" "<<"key = "<<msg.getData()<<endl;
    fout.close(); */

    auto it = std::find(connectionList.begin(),connectionList.end(),conn);
    if(it+1 == connectionList.end() && connectionList.size()==N+K){

        
        for(auto it_vec=connectionList.begin();it_vec!=connectionList.end();++it_vec){
            if(it_vec==connectionList.begin())
                (*it_vec)->sendmsg(stage1_map[*(connectionList.end()-1)]);
            else
                (*it_vec)->sendmsg(stage1_map[*(it_vec-1)]);
        }

        return 2;
    }
    return 1;
}

int stage2(std::shared_ptr<socketx::Connection> conn){
    socketx::Message msg = conn->recvmsg();
    if(msg.getSize()==0){
        conn->handleClose();
        return 2;
    }

    /*Compute the vector of the sum of crypto_vec*/
    
    auto it = std::find(connectionList.begin(),connectionList.end(),conn);
    if(it+1 == connectionList.end() && connectionList.size()==N+K){

        /*Send a responds message to client*/
        std::string responds = "stage3";
        msg = socketx::Message(const_cast<char *>(responds.c_str()),responds.size()+1);
        for(auto it_vec=connectionList.begin();it_vec!=connectionList.end();++it_vec){
            (*it_vec)->sendmsg(msg);
        }
        return 3;
    }
    return 2;
}

/*
*  Receive randomNum, key1, key2 to simulate recursion.
*/
int stage3(std::shared_ptr<socketx::Connection> conn){
    socketx::Message msg = conn->recvmsg();
    if(msg.getSize()==0){
        conn->handleClose();
        return 3;
    }

    std::string parameterStr(msg.getData());
    /* std::ofstream fout("./data/aggregator_logs", std::ofstream::out | std::ofstream::app);
    fout<<"fd = "<<conn->getFD()<<" "<<"parameterStr = "<<parameterStr<<endl;
    fout.close(); */

    /*Extract parameter*/
    size_t begin = 0;
    auto pos = parameterStr.find(" ", begin);
    std::string randomNumStr = parameterStr.substr(begin,pos-begin);
    begin = pos + 1;
    pos = parameterStr.find(" ", begin);
    std::string key1 = parameterStr.substr(begin,pos-begin);
    begin = pos + 1;
    pos = parameterStr.find(" ", begin);
    std::string key2 = parameterStr.substr(begin,pos-begin);
    std::vector<std::string> parameter_vec = {randomNumStr, key1, key2};
   
    stage3_map[conn] = parameter_vec;


    /*Compute the vector of the sum of crypto_vec*/
    auto it = std::find(connectionList.begin(),connectionList.end(),conn);
    if(it+1 == connectionList.end() && connectionList.size()==N+K){

        
        
        std::string responds = "state4";
        

        msg = socketx::Message(const_cast<char *>(responds.c_str()),responds.size()+1);
        for(auto it_list = connectionList.begin();it_list!=connectionList.end();++it_list){
            (*it_list)->sendmsg(msg);
        }
        cout<<"Sending responds to clients..."<<endl;
        return 4;
    }
    return 3;
}


std::string rsa_example(std::string message){
    unsigned int keyLength = 1024;

    MyRSA rsa;

    //cout << "============encrypt and decrypt================" << endl;
    rsa.GenerateRSAKey(keyLength);
    string ciphertext = rsa.Encrypt(message.c_str());
    //cout << "The cipher is : " << ciphertext << endl;

    string decrypted = rsa.Decrypt(ciphertext.c_str());
    //cout << "The recover is : " << decrypted << endl;
    return ciphertext;
}


void enterSim(size_t &bitComplexity, long double &time_ms){
    
    int k = 101;
    BigInteger Yn = generateRandom();
    for(int i=0;i<K;i++){
        // random permutation
        std::vector<int> seq;  
        for (int j = 0; j < N+i; ++j) {  
            seq.push_back(j);  
        }
        
        // obtain a time-based seed:  
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();  
        std::shuffle(seq.begin(), seq.end(), std::default_random_engine(seed));

        //OTN
        // for every new participant i, we need run one OTN
        clock_gettime(CLOCK_REALTIME,&time_spec);  
        for(int j=0; j<N+i;++j){
            // for each OTN, we need run N+i RSA
            std::string str = rsa_example(std::to_string(seq[j]));
            bitComplexity += str.size()*8;
        }
        // each participant need receive Yn and k
        bitComplexity += (bigIntegerToString(Yn).size()*8+std::to_string(k).size()*8)*(N+i)*2;
        clock_gettime(CLOCK_REALTIME,&time_spec_);
        time_ms += (static_cast<long double>(time_spec_.tv_sec - time_spec.tv_sec)*1000+
                            static_cast<long double>(time_spec_.tv_nsec - time_spec.tv_nsec)/(1.0e6));
    }

}

int stage4(std::shared_ptr<socketx::Connection> conn, socketx::EventLoop *loop_){
    socketx::Message msg = conn->recvmsg();
    if(msg.getSize()==0){
        conn->handleClose();
        return 4;
    }

    std::string IDStr(msg.getData());
    /* std::ofstream fout("./data/aggregator_logs", std::ofstream::out | std::ofstream::app);
    fout<<"fd = "<<conn->getFD()<<" "<<"IDStr = "<<IDStr<<endl;
    fout.close(); */

    stage4_map[std::stoi(IDStr)] = conn;


    auto it = std::find(connectionList.begin(),connectionList.end(),conn);
    if(it+1 == connectionList.end() && connectionList.size()==N+K){

        /*Simulating ID generation*/

        cout<<"Simulate participant entering..............!"<<endl;

        enterSim(bitComplexity, time_ms);
       

        cout<<"Simulate stage4.............!"<<endl;
        /*Begin to simulate stage4*/
        /*Simulate N+K rounds*/
        for(int i=1;i<=N+K;++i){

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
                clock_gettime(CLOCK_REALTIME,&time_spec);
                /*
                * Simulate decryption
                * Get the result of one round
                */
                oneRound += stringToBigInteger(cryptoStr);
                clock_gettime(CLOCK_REALTIME,&time_spec_);
                time_ms += (static_cast<long double>(time_spec_.tv_sec - time_spec.tv_sec)*1000+
                            static_cast<long double>(time_spec_.tv_nsec - time_spec.tv_nsec)/(1.0e6));
            }
            oneRound %= M;
        }

        

        cout<<"All the tasks is finished..............!"<<endl;

        ofstream fout("./data/enter_aggregator_logs", std::ofstream::out | std::ofstream::app);
        fout.close();

        fout.open("./data/enter_results_N+K="+std::to_string(N+K)+"_bits="+std::to_string(bits), std::ofstream::out | std::ofstream::app);
        fout<<"N+K="<<N+K<<", "<<"Bits="<<bitComplexity/36<<", "<<"Time="<<std::to_string(static_cast<long double>(time_s*1000+time_ms))<<endl;
        fout.close();
        /*close the server*/
        loop_->quit();
    }

    return 4;
}

class Aggregator{
    public:
        Aggregator(socketx::EventLoop *loop, std::string port)
        :loop_(loop), port_(port), stage(1), step_stage3(1),
        server_(std::make_shared<socketx::Server>(loop,port)){
            server_->setHandleConnectionFunc(std::bind(&Aggregator::handleConnection, this, std::placeholders::_1));
            server_->setHandleCloseEvents(std::bind(&Aggregator::handleCloseEvents, this, std::placeholders::_1));
        }

        void start(){
            server_->start();
        }

        void handleConnection(std::shared_ptr<socketx::Connection> conn){
            printf("New connection comes, we are going to set read events!!!\n");
            server_->setHandleReadEvents(std::bind(&Aggregator::handleReadEvents, this,  std::placeholders::_1));
            connectionList.push_back(conn);
        }
        void handleReadEvents(std::shared_ptr<socketx::Connection> conn){
            if(stage==1){
                ofstream fout("./data/enter_aggregator_logs",ofstream::out | ofstream::app);
                fout<<"fd = "<<conn->getFD()<<" "<<"stage = "<<stage<<"\n";
                fout.close();

                stage = stage1(conn);
            }
            else if(stage==2){
                ofstream fout("./data/enter_aggregator_logs",ofstream::out | ofstream::app);
                fout<<"fd = "<<conn->getFD()<<" "<<"stage = "<<stage<<"\n";
                fout.close();

                stage = stage2(conn);
            }
            else if(stage==3){
                ofstream fout("./data/enter_aggregator_logs",ofstream::out | ofstream::app);
                fout<<"fd = "<<conn->getFD()<<" "<<"stage = "<<stage<<"\n";
                fout.close();

                stage = stage3(conn);
            }
            else if(stage==4){
                ofstream fout("./data/enter_aggregator_logs",ofstream::out | ofstream::app);
                fout<<"fd = "<<conn->getFD()<<" "<<"stage = "<<stage<<"\n";
                fout.close();
                stage = stage4(conn, loop_);
            }
        }
        void handleCloseEvents(std::shared_ptr<socketx::Connection> conn){
            printf("Close connection...\n");
            auto it = std::find(connectionList.begin(),connectionList.end(),conn);
            connectionList.erase(it);
            stage1_map.erase(*it);
            stage2_map.erase(*it);
            stage3_map.erase(*it);
        }

    private:
        socketx::EventLoop *loop_;
        std::shared_ptr<socketx::Server> server_;
        std::string port_;

        /*Record stage*/
        int stage;
        int step_stage3;
};


int main(int argc, char **argv){
    if(argc!=2){
        fprintf(stderr,"usage: %s <port>\n", argv[0]);
        exit(0);
    }

    std::string port(argv[1]);

    socketx::EventLoop loop;
    Aggregator server(&loop,port);

    ofstream fout("./data/enter_aggregator_logs");
    fout<<"Start the experiment of "<<N<<" participants. "<<"M = "<<M<<endl;
    fout.close();
    
    /*Start server*/
    server.start(); 
    loop.loop();

    return 0;
}