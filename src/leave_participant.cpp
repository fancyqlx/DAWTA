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
    std::uniform_int_distribution<unsigned long long int> dist(static_cast<unsigned long long>(1),static_cast<unsigned long long>(std::pow(N,5)));
    unsigned long long int randomNum = dist(e2);
    cout<<"RandomNum: "<<randomNum<<endl;


    /*Send a responds message to client*/
    std::string responds = "stage3";
    socketx::Message msg(const_cast<char*>(responds.c_str()),responds.size()+1);
    conn->sendmsg(msg);
    printf("Sending responds string to the server\n");
    return randomNum;
}

/*Send randomNum, key1, key2 to server*/
int stage3(std::shared_ptr<socketx::Connection> conn, unsigned long long int randomNum, std::string key1, std::string key2){
    socketx::Message msg = conn->recvmsg();
    if(msg.getSize()==0){
        conn->handleClose();
        return 3;
    }
    assert(std::string(msg.getData())=="stage3");

    std::string str = std::to_string(randomNum)  + " ";
    str += key1 + " " + key2 + " ";
    msg = socketx::Message(const_cast<char *>(str.c_str()),str.size()+1);
    conn->sendmsg(msg);
    return 4;
}

int stage4(std::shared_ptr<socketx::Connection> conn, unsigned long long int randomNum){
    socketx::Message msg = conn->recvmsg();
    if(msg.getSize()==0){
        conn->handleClose();
        return -1;
    }
    std::string responds(msg.getData());
    /* std::ofstream fout("./data/aggregator_logs", std::ofstream::out | std::ofstream::app);
    fout<<"fd = "<<conn->getFD()<<" "<<"rangeStr = "<<rangeStr<<endl;
    fout.close(); */

    /* Simulate generating IDs*/

    return -1;
}

class EchoClient{
    public:
        EchoClient(socketx::EventLoop *loop, std::string hostname, std::string port)
        :loop_(loop), hostname_(hostname),port_(port), stage(1), ID(-1),
        client_(std::make_shared<socketx::Client>(loop,hostname,port)){
            client_->setHandleConnectionFunc(std::bind(&EchoClient::handleConnection, this, std::placeholders::_1));
            client_->setHandleCloseEvents(std::bind(&EchoClient::handleCloseEvents, this, std::placeholders::_1));
        }

        void start(){
            client_->start();
        }

        void setID(int id){
            this->ID = id;
        }

        void handleConnection(std::shared_ptr<socketx::Connection> conn){
            printf("New connection comes, we are going to set read events!!!\n");
            client_->setHandleReadEvents(std::bind(&EchoClient::handleReadEvents, this, std::placeholders::_1));

            key1 = bigIntegerToString(generateRandom());
            
            conn->sendmsg(socketx::Message(const_cast<char*>(key1.c_str()),key1.size()+1));

            printf("Sending key1 %s to server...\n", key1.c_str());
            ofstream fout("./data/leave_participants_logs",ofstream::out | ofstream::app);
            fout<<"fd = "<<conn->getFD()<<" "<<"key1 = "<<key1<<"\n";
            fout.close();
        }

        void handleReadEvents(std::shared_ptr<socketx::Connection> conn){
            if(stage==1){
                ofstream fout("./data/leave_participants_logs",ofstream::out | ofstream::app);
                fout<<"fd = "<<conn->getFD()<<" "<<"stage = "<<stage<<"\n";
                fout.close();

                key2 = stage1(conn);
                ++stage;
                randomNum = stage2(conn,key1,key2);
                ++stage;
            }
            else if(stage==3){
                ofstream fout("./data/leave_participants_logs",ofstream::out | ofstream::app);
                fout<<"fd = "<<conn->getFD()<<" "<<"stage = "<<stage<<"\n";
                fout.close();

                stage = stage3(conn,randomNum,key1,key2);
            }
            else if(stage==4){
                ofstream fout("./data/leave_participants_logs",ofstream::out | ofstream::app);
                fout<<"fd = "<<conn->getFD()<<" "<<"stage = "<<stage<<"\n";
                fout.close();

                stage4(conn,randomNum);
                std::string str = std::to_string(ID);
                socketx::Message msg(const_cast<char*>(str.c_str()),str.size()+1);
                conn->sendmsg(msg);
                cout<<"Sending ID to the server......"<<endl;

                fout.open("./data/leave_participants_logs",ofstream::out | ofstream::app);
                fout<<"fd = "<<conn->getFD()<<" "<<"ID = "<<ID<<"\n";
                fout.close();

                cout<<"Participant has finished its work...........!"<<endl;
            }
        }
        
        void handleCloseEvents(std::shared_ptr<socketx::Connection> conn){
            printf("Close connection...\n");
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
        int ID;
};


int main(int argc, char **argv){
    if(argc!=3){
        fprintf(stderr,"usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    std::string hostname(argv[1]);
    std::string port(argv[2]);
    std::vector<std::shared_ptr<EchoClient>> clientList;

    ofstream fout("./data/leave_participants_logs",ofstream::out);
    fout<<"Start the experiment of "<<N<<" participants. "<<"K = "<<K<<". M = "<<M<<endl;
    fout.close();

    socketx::EventLoop loop;
    /*Create N client*/
    for(int i=0;i<N-K;++i){
        printf("Create new client\n");
        std::shared_ptr<EchoClient> client = std::make_shared<EchoClient>(&loop,hostname,port);
        client->setID(i+1);
        clientList.push_back(client);
    }
    /*Start N client*/
    for(int i=0;i<N-K;++i){
        printf("Start new client\n");
        clientList[i]->start();
    }
    loop.loop();

    return 0;
}