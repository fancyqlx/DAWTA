#include "EventLoop.hpp"
#include "Server.hpp"
#include "Threadx.hpp"
#include "dawta.hpp"

std::vector<std::shared_ptr<socketx::Connection>> connectionList;
std::map<std::shared_ptr<socketx::Connection>, socketx::Message> msgMap;
std::mutex mut;

int stage1(std::shared_ptr<socketx::Connection> conn){
    socketx::Message msg = conn->recvmsg();
    if(msg.getSize()==0){
        conn->handleClose();
        return 1;
    }else
    /*Send message to the client next to it
    * stage increases to 2 after finishing it.
    */
    mut.lock();
    msgMap[conn] = msg;
    std::ofstream fout("./data/aggregator_logs", std::ofstream::out | std::ofstream::app);
    fout<<"fd = "<<conn->getFD()<<" "<<"key = "<<msg.getData()<<"\n";
    fout.close();
    auto it = std::find(connectionList.begin(),connectionList.end(),conn);
    if(it+1 == connectionList.end() && connectionList.size()==N){
        
        for(auto it_vec=connectionList.begin();it_vec!=connectionList.end();++it_vec){
            if(it_vec==connectionList.begin())
                (*it_vec)->sendmsg(msgMap[*(connectionList.end()-1)]);
            else
                (*it_vec)->sendmsg(msgMap[*(it_vec-1)]);
        }
        return 2;
    }
    mut.unlock();
    return 1;
}

class Aggregator{
    public:
        Aggregator(socketx::EventLoop *loop, std::string port)
        :loop_(loop), port_(port), stage(1),
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
            mut.lock();
            connectionList.push_back(conn);
            mut.unlock();
        }
        void handleReadEvents(std::shared_ptr<socketx::Connection> conn){
            if(stage==1){
                ofstream fout("./data/aggregator_logs",ofstream::out | ofstream::app);
                fout<<"fd = "<<conn->getFD()<<" "<<"stage = "<<stage<<"\n";
                fout.close();

                stage = stage1(conn);
            }
        }
        void handleCloseEvents(std::shared_ptr<socketx::Connection> conn){
            printf("Close connection...\n");
            auto it = std::find(connectionList.begin(),connectionList.end(),conn);
            mut.lock();
            connectionList.erase(it);
            msgMap.erase(*it);
            mut.unlock();
        }

    private:
        socketx::EventLoop *loop_;
        std::shared_ptr<socketx::Server> server_;
        std::string port_;

        /*Record stage*/
        int stage;
};


int main(int argc, char **argv){
    if(argc!=2){
        fprintf(stderr,"usage: %s <port>\n", argv[0]);
        exit(0);
    }

    std::string port(argv[1]);
    socketx::EventLoop loop;
    Aggregator server(&loop,port);

    std::string test;
    while(std::cin>>test){
        cout<<bigIntegerToString(hexToDecimal(test))<<endl;
    }

    ofstream fout("./data/aggregator_logs",ofstream::out);
    fout<<"Start the experiment of "<<N<<" participants.\n";
    fout.close();
    
    /*Start server*/
    server.start(); 
    loop.loop();

    return 0;
}