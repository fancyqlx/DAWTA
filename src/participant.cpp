#include "EventLoop.hpp"
#include "Client.hpp"
#include "Connection.hpp"
#include "BigIntegerLibrary.hh"
#include "dawta.hpp"

class EchoClient{
    public:
        EchoClient(socketx::EventLoop *loop, std::string hostname, std::string port)
        :loop_(loop), hostname_(hostname),port_(port), state(1),
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
            clientConn = conn;
            key1 = bigIntegerToString(generateRandom());
            conn->sendmsg(Message(key1.c_str(),key1.size()));
            printf("Sending key1 %s to server...\n", key1.c_str());
        }
        void handleReadEvents(std::shared_ptr<socketx::Connection> conn){
            if(state==1){
                Message msg = conn.recvmsg();
                key2 = std::string(msg.get)
            }
        }
        void handleCloseEvents(std::shared_ptr<socketx::Connection> conn){
            printf("Close connection...\n");
        }

    private:
        std::shared_ptr<socketx::Connection> stdinConn;
        std::shared_ptr<socketx::Connection> clientConn;
        socketx::EventLoop *loop_;
        std::shared_ptr<socketx::Client> client_;
        std::string hostname_;
        std::string port_;
        /*Two keys, one for client and another is received from other clients*/
        std::string key1;
        std::string key2;
        /*Record the current state*/
        int state;
};


int main(int argc, char **argv){
    if(argc!=3){
        fprintf(stderr,"usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    std::string hostname(argv[1]);
    std::string port(argv[2]);

    BigInteger a = stringToBigInteger(hostname);
    BigInteger b = stringToBigInteger(port);
    std::cout<<bigIntegerToString(a+b)<<std::endl;

    
    std::cout<<hashFunc(hostname,port)<<std::endl;

    socketx::EventLoop loop;
    EchoClient client(&loop,hostname,port);
    client.start();
    loop.loop();

    return 0;
}