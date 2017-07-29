#include "EventLoop.hpp"
#include "Server.hpp"
#include "Threadx.hpp"
#include "dawta.hpp"

/* auto config = readConfig();
int N = config.first;
BigInteger M = config.second; */


long  time_ms; // Milliseconds
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

    /*Send message to the client next to it
    * stage increases to 2 after finishing it.
    */
    stage1_map[conn] = msg;
    /* std::ofstream fout("./data/aggregator_logs", std::ofstream::out | std::ofstream::app);
    fout<<"fd = "<<conn->getFD()<<" "<<"key = "<<msg.getData()<<endl;
    fout.close(); */

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
    return 1;
}

int stage2(std::shared_ptr<socketx::Connection> conn){
    socketx::Message msg = conn->recvmsg();
    if(msg.getSize()==0){
        conn->handleClose();
        return 2;
    }

    std::string cryptoStr(msg.getData());
    std::ofstream fout("./data/aggregator_logs", std::ofstream::out | std::ofstream::app);
    fout<<"fd = "<<conn->getFD()<<" "<<"cryptoStr = "<<cryptoStr<<endl;
    fout.close();

    /*Construct vector of cryptoStr*/
    std::vector<BigInteger> crypto_vec;
    size_t begin = 0;
    while(begin!=cryptoStr.size()){
        auto pos = cryptoStr.find(" ", begin);
        std::string str = cryptoStr.substr(begin,pos-begin);
        cout<<str<<endl;
        crypto_vec.push_back(stringToBigInteger(str));
        begin = pos + 1;
    }

    assert(crypto_vec.size()==N);

    stage2_map[conn] = crypto_vec;

    /*Compute the vector of the sum of crypto_vec*/
    

    auto it = std::find(connectionList.begin(),connectionList.end(),conn);
    if(it+1 == connectionList.end() && connectionList.size()==N){


        std::vector<BigInteger> sum_vec(N,BigInteger());
        for(int i=0;i<N;++i){
            for(auto it_map = stage2_map.begin();it_map!=stage2_map.end();++it_map){
                sum_vec[i] += (it_map->second)[i];
            }
            sum_vec[i] %= M;
            cout<<bigIntegerToString(sum_vec[i])<<endl;
        }

        /* fout.open("./data/aggregator_logs", std::ofstream::out | std::ofstream::app);
        fout<<"fd = "<<conn->getFD()<<" "<<"sum_vec = "<<endl;
        for(int i=0;i<N;++i){
            std::string str = bigIntegerToString(sum_vec[i]);
            fout<<str<<" ";
        }
        fout<<endl;
        fout.close(); */

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
    /* fout.open("./data/aggregator_logs", std::ofstream::out | std::ofstream::app);
    fout<<"fd = "<<conn->getFD()<<" "<<"size of parameter_vec = "<<parameter_vec.size()<<endl;
        for(auto it=parameter_vec.begin();it!=parameter_vec.end();++it){
            fout<<*it<<" ";
        }
    fout<<endl;
    fout.close(); */


    /*Compute the vector of the sum of crypto_vec*/
    auto it = std::find(connectionList.begin(),connectionList.end(),conn);
    if(it+1 == connectionList.end() && connectionList.size()==N){

        /*anti_aggregator has no bitComplexity and timeComplexity in this step*/
        anti_recursion(range,stage3_map,0,static_cast<unsigned long long>(std::pow(N,5)),N);

        std::string rangeStr = "";
        for(auto it_vec=range.begin();it_vec!=range.end();++it_vec){
            rangeStr += std::to_string(it_vec->first) + " " + std::to_string(it_vec->second) + " ";
        }
        /* cout<<"rangeStr: "<<rangeStr<<endl;

        fout.open("./data/aggregator_logs", std::ofstream::out | std::ofstream::app);
        fout<<"rangeStr = "<<rangeStr<<endl;
        fout.close(); */

        msg = socketx::Message(const_cast<char *>(rangeStr.c_str()),rangeStr.size()+1);
        for(auto it_list = connectionList.begin();it_list!=connectionList.end();++it_list){
            (*it_list)->sendmsg(msg);
        }
        cout<<"Sending rangeStr to clients..."<<endl;
        return 4;
    }
    return 3;
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
    if(it+1 == connectionList.end() && connectionList.size()==N){

        clock_gettime(CLOCK_REALTIME,&time_spec);

        /*Begin to simulate stage4*/
        finalResults = simulateStage4(stage3_map, stage4_map, bitComplexity);

        assert(finalResults.size() == N);
        

        cout<<"All the tasks is finished..............!"<<endl;

        clock_gettime(CLOCK_REALTIME,&time_spec_);
        time_s += time_spec_.tv_sec - time_spec.tv_sec;
        time_ms += round((time_spec_.tv_nsec - time_spec.tv_nsec)/1.0e6);

        ofstream fout("./data/aggregator_logs", std::ofstream::out | std::ofstream::app);
        fout<<"finalResults = ";
            for(auto it=finalResults.begin();it!=finalResults.end();++it){
                fout<<bigIntegerToString(*it)<<" ";
            }
        fout<<endl;
        fout.close();

        fout.open("./data/anti_results_N="+std::to_string(N)+"_bits="+std::to_string(bits), std::ofstream::out | std::ofstream::app);
        fout<<"N="<<N<<", "<<"Bits="<<bitComplexity/9<<", "<<"Time="<<std::to_string(time_s*1000+time_ms)<<endl;
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
            else if(stage==3){
                ofstream fout("./data/aggregator_logs",ofstream::out | ofstream::app);
                fout<<"fd = "<<conn->getFD()<<" "<<"stage = "<<stage<<"\n";
                fout.close();

                stage = stage3(conn);
            }
            else if(stage==4){
                ofstream fout("./data/aggregator_logs",ofstream::out | ofstream::app);
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

    ofstream fout("./data/aggregator_logs");
    fout<<"Start the experiment of "<<N<<" participants. "<<"M = "<<M<<endl;
    fout.close();
    
    /*Start server*/
    server.start(); 
    loop.loop();

    return 0;
}