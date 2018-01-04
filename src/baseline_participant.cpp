#include "EventLoop.hpp"
#include "Client.hpp"
#include "Connection.hpp"
#include "BigIntegerLibrary.hh"
#include "dawta.hpp"

/* auto config = readConfig();
int N = config.first;
BigInteger M = config.second; */

std::string stage1(std::shared_ptr<socketx::Connection> conn){
    socketx::Message msg = conn->recvmsg();
    std::string key2 = std::string(msg.getData());
    printf("Received key2 %s ...\n", key2.c_str());

    /* ofstream fout("./data/participants_logs",ofstream::out | ofstream::app);
    fout<<"fd = "<<conn->getFD()<<" "<<"key2 = "<<key2<<"\n";
    fout.close(); */

    return key2;
}

unsigned long long int stage2(std::shared_ptr<socketx::Connection> conn, std::string key1, std::string key2){
    /*Generate random number between [1,n^5]*/
    std::random_device rd;
    std::mt19937_64 e2(rd());
    auto numRange = static_cast<unsigned long long>(std::pow(N,2));
    std::uniform_int_distribution<unsigned long long int> dist(static_cast<unsigned long long>(1),numRange);
    unsigned long long int randomNum = dist(e2);
    cout<<"RandomNum: "<<randomNum<<endl;

    /*Encrypt the vector by hash code*/
    std::string hashCode1 = hashFunc(key1,std::to_string(N));
    std::string hashCode2 = hashFunc(key2,std::to_string(N));
    BigInteger F = (hexToDecimal(hashCode1)-hexToDecimal(hashCode2));
    
    
    
    for(unsigned long long i=0;i<numRange;++i){
        std::string cryptoStr = "";
        if(i==numRange){
            cryptoStr += bigIntegerToString((F+1)%M) + " ";
            cryptoStr += bigIntegerToString((F+1)%M) + " ";
        }
        else{
            cryptoStr += bigIntegerToString(F%M) + " ";
            cryptoStr += bigIntegerToString(F%M) + " ";
        }
        /* ofstream fout("./data/participants_logs",ofstream::out | ofstream::app);
        fout<<"fd = "<<conn->getFD()<<" "<<"cryptoStr = "<<cryptoStr<<endl;
        fout.close(); */
        /*Send messages to the server*/
        socketx::Message msg(const_cast<char*>(cryptoStr.c_str()),cryptoStr.size()+1);
        conn->sendmsg(msg);
        printf("Sending crypto string to the server\n");         
    }

    ofstream fout("./data/baseline_participants_logs",ofstream::out | ofstream::app);
    fout<<"fd = "<<conn->getFD()<<" "<<"stage = "<<2<<" "<<"Participant has finished its work!\n";
    fout.close();

    return randomNum;
}

class EchoClient{
    public:
        EchoClient(socketx::EventLoop *loop, std::string hostname, std::string port)
        :loop_(loop), hostname_(hostname),port_(port), stage(1),
        client_(std::make_shared<socketx::Client>(loop,hostname,port)){
            client_->setHandleConnectionFunc(std::bind(&EchoClient::handleConnection, this, std::placeholders::_1));
            client_->setHandleCloseEvents(std::bind(&EchoClient::handleCloseEvents, this, std::placeholders::_1));
        }

        void start(){
            client_->start();
        }

        void handleConnection(std::shared_ptr<socketx::Connection> conn){
            printf("New connection comes, we are going to set read events!!!\n");
            client_->setHandleReadEvents(std::bind(&EchoClient::handleReadEvents, this, std::placeholders::_1));

            key1 = bigIntegerToString(generateRandom());
            
            conn->sendmsg(socketx::Message(const_cast<char*>(key1.c_str()),key1.size()+1));

            printf("Sending key1 %s to server...\n", key1.c_str());
            ofstream fout("./data/baseline_participants_logs",ofstream::out | ofstream::app);
            fout<<"fd = "<<conn->getFD()<<" "<<"key1 = "<<key1<<"\n";
            fout.close();
        }

        void handleReadEvents(std::shared_ptr<socketx::Connection> conn){
            if(stage==1){
                ofstream fout("./data/baseline_participants_logs",ofstream::out | ofstream::app);
                fout<<"fd = "<<conn->getFD()<<" "<<"stage = "<<stage<<"\n";
                fout.close();

                key2 = stage1(conn);
                ++stage;
                randomNum = stage2(conn,key1,key2);
                ++stage;
            }else{
                socketx::Message msg = conn->recvmsg();
                if(msg.getSize()==0){
                    conn->handleClose();
                }
            }
        }
        
        void handleCloseEvents(std::shared_ptr<socketx::Connection> conn){
            printf("Participant closes connection...\n");
            loop_->quit();
        }

    private:
        socketx::EventLoop *loop_;
        std::shared_ptr<socketx::Client> client_;
        std::string hostname_;
        std::string port_;
        /*Two keys, one for client and another is received from other clients*/
        std::string key1;
        std::string key2;
        /*Record the current stage*/
        int stage;
        /*Random number */
        unsigned long long int randomNum;
};


int main(int argc, char **argv){
    if(argc!=3){
        fprintf(stderr,"usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    std::string hostname(argv[1]);
    std::string port(argv[2]);
    std::vector<std::shared_ptr<EchoClient>> clientList;

    ofstream fout("./data/baseline_participants_logs",ofstream::out);
    fout<<"Start the experiment of "<<N<<" participants. "<<"M = "<<M<<endl;
    fout.close();

    socketx::EventLoop loop;
    /*Create N client*/
    for(int i=0;i<N;++i){
        printf("Create new client\n");
        std::shared_ptr<EchoClient> client = std::make_shared<EchoClient>(&loop,hostname,port);
        clientList.push_back(client);
    }
    /*Start N client*/
    for(int i=0;i<N;++i){
        printf("Start new client\n");
        clientList[i]->start();
    }
    loop.loop();

    return 0;
}