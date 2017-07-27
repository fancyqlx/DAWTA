#include "EventLoop.hpp"
#include "Server.hpp"
#include "Threadx.hpp"
#include "dawta.hpp"

std::vector<std::shared_ptr<socketx::Connection>> connectionList;
std::map<std::shared_ptr<socketx::Connection>, socketx::Message> stage1_map;
std::map<std::shared_ptr<socketx::Connection>, std::vector<BigInteger>> stage2_map;
std::mutex mut;

int stage1(std::shared_ptr<socketx::Connection> conn){
    socketx::Message msg = conn->recvmsg();
    if(msg.getSize()==0){
        conn->handleClose();
        return 1;
    }
    /*Send message to the client next to it
    * stage increases to 2 after finishing it.
    */
    mut.lock();
    stage1_map[conn] = msg;
    std::ofstream fout("./data/aggregator_logs", std::ofstream::out | std::ofstream::app);
    fout<<"fd = "<<conn->getFD()<<" "<<"key = "<<msg.getData()<<endl;
    fout.close();

    auto it = std::find(connectionList.begin(),connectionList.end(),conn);
    if(it+1 == connectionList.end() && connectionList.size()==N){
        
        for(auto it_vec=connectionList.begin();it_vec!=connectionList.end();++it_vec){
            if(it_vec==connectionList.begin())
                (*it_vec)->sendmsg(stage1_map[*(connectionList.end()-1)]);
            else
                (*it_vec)->sendmsg(stage1_map[*(it_vec-1)]);
        }
        return 2;
    }
    mut.unlock();
    return 1;
}

int stage2(std::shared_ptr<socketx::Connection> conn){
    socketx::Message msg = conn->recvmsg();
    if(msg.getSize()==0){
        conn->handleClose();
        return 2;
    }
    std::string cryptoStr(msg.getData());
    cout<<cryptoStr<<endl;
    std::ofstream fout("./data/aggregator_logs", std::ofstream::out | std::ofstream::app);
    fout<<"fd = "<<conn->getFD()<<" "<<"cryptoStr = "<<cryptoStr<<endl;
    fout.close();

    /*Construct vector of cryptoStr*/
    std::vector<BigInteger> crypto_vec;
    size_t begin = 0;
    for(int i=0;i<N;++i){
        auto pos = cryptoStr.find(" ", begin);
        std::string str = cryptoStr.substr(begin,pos-begin);
        cout<<str<<endl;
        crypto_vec.push_back(stringToBigInteger(str));
        begin = pos + 1;
        if(begin==cryptoStr.size()) break;
    }
    assert(crypto_vec.size()==N);
    stage2_map[conn] = crypto_vec;

    /*Compute the vector of the sum of crypto_vec*/
    std::vector<BigInteger> sum_vec(N,BigInteger());
    auto it = std::find(connectionList.begin(),connectionList.end(),conn);
    if(it+1 == connectionList.end() && connectionList.size()==N){
        for(int i=0;i<N;++i){
            for(auto it_map = stage2_map.begin();it_map!=stage2_map.end();++it_map){
                sum_vec[i] += (it_map->second)[i];
            }
            cout<<bigIntegerToString(sum_vec[i])<<endl;
        }
        fout.open("./data/aggregator_logs", std::ofstream::out | std::ofstream::app);
        fout<<"fd = "<<conn->getFD()<<" "<<"sum_vec = "<<endl;
        for(int i=0;i<N;++i){
            std::string str = bigIntegerToString(sum_vec[i]);
            fout<<str<<" ";
        }
        fout<<endl;
        fout.close();
        return 3;
    }
    return 2;
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
            else if(stage==2){
                ofstream fout("./data/aggregator_logs",ofstream::out | ofstream::app);
                fout<<"fd = "<<conn->getFD()<<" "<<"stage = "<<stage<<"\n";
                fout.close();

                stage = stage2(conn);
            }
        }
        void handleCloseEvents(std::shared_ptr<socketx::Connection> conn){
            printf("Close connection...\n");
            auto it = std::find(connectionList.begin(),connectionList.end(),conn);
            mut.lock();
            connectionList.erase(it);
            stage1_map.erase(*it);
            stage2_map.erase(*it);
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

    ofstream fout("./data/aggregator_logs");
    fout<<"Start the experiment of "<<N<<" participants.\n";
    fout.close();
    
    /*Start server*/
    server.start(); 
    loop.loop();

    return 0;
}