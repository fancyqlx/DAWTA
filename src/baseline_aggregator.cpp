#include "EventLoop.hpp"
#include "Server.hpp"
#include "Threadx.hpp"
#include "dawta.hpp"

/* auto config = readConfig();
int N = config.first;
BigInteger M = config.second; */


long double  time_ms; // Milliseconds
time_t  time_s;  // Seconds
struct timespec time_spec;
struct timespec time_spec_;

std::vector<std::shared_ptr<socketx::Connection>> connectionList;
std::map<std::shared_ptr<socketx::Connection>, socketx::Message> stage1_map;
std::map<std::shared_ptr<socketx::Connection>, std::vector<BigInteger>> stage2_num_map;
std::map<std::shared_ptr<socketx::Connection>, std::vector<BigInteger>> stage2_count_map;
std::map<std::shared_ptr<socketx::Connection>, unsigned long long> counter;
size_t bitComplexity = 0;
size_t participants_counter = N;



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

int stage2(std::shared_ptr<socketx::Connection> conn, socketx::EventLoop *loop_){
    socketx::Message msg = conn->recvmsg();
    if(msg.getSize()==0){
        conn->handleClose();
        return 2;
    }

    /*bitComplexity*/
    bitComplexity += msg.getSize() * 8;

    std::string cryptoStr(msg.getData());
    std::ofstream fout("./data/baseline_aggregator_logs", std::ofstream::out | std::ofstream::app);
    fout<<"fd = "<<conn->getFD()<<" "<<"cryptoStr = "<<cryptoStr<<endl;
    fout.close();

    /*Construct vector of each connection for storing number and its count*/
    clock_gettime(CLOCK_REALTIME,&time_spec);
    size_t begin = 0;
    
    auto pos = cryptoStr.find(" ", begin);
    std::string numStr = cryptoStr.substr(begin,pos-begin);
    //cout<<numStr<<endl;
    begin = pos + 1;
    pos = cryptoStr.find(" ", begin);
    std::string countStr = cryptoStr.substr(begin,pos-begin);
    //cout<<countStr<<endl;
    
    if(stage2_num_map.count(conn)){
        stage2_num_map[conn].push_back(stringToBigInteger(numStr));
    }else{
        stage2_num_map[conn] = std::vector<BigInteger>(1,stringToBigInteger(numStr));
    }
    if(stage2_count_map.count(conn)){
        stage2_count_map[conn].push_back(stringToBigInteger(countStr));
    }else{
        stage2_count_map[conn] = std::vector<BigInteger>(1,stringToBigInteger(countStr));
    }

    counter[conn] -= 1;
    clock_gettime(CLOCK_REALTIME,&time_spec_);
    time_s += (time_spec_.tv_sec - time_spec.tv_sec);
    time_ms += static_cast<long double>(time_spec_.tv_nsec - time_spec.tv_nsec)/(1.0e6);
    /*Compute the vector of the sum of num_vec and count_vec*/
    
    if(counter[conn]==0) participants_counter -= 1;
    cout<<participants_counter<<endl;
    if(participants_counter==0){
        clock_gettime(CLOCK_REALTIME,&time_spec);
        std::vector<BigInteger> num_vec(N,BigInteger());
        std::vector<BigInteger> count_vec(N,BigInteger());
        for(int i=0;i<N;++i){
            for(auto it_num_map = stage2_num_map.begin();it_num_map!=stage2_num_map.end();++it_num_map){
                num_vec[i] += (it_num_map->second)[i];
            }
            num_vec[i] %= M;
            cout<<bigIntegerToString(num_vec[i])<<endl;


            for(auto it_count_map = stage2_count_map.begin();it_count_map!=stage2_count_map.end();++it_count_map){
                count_vec[i] += (it_count_map->second)[i];
            }
            count_vec[i] %= M;
            cout<<bigIntegerToString(count_vec[i])<<endl;
        }
        clock_gettime(CLOCK_REALTIME,&time_spec_);
        time_s += (time_spec_.tv_sec - time_spec.tv_sec);
        time_ms += static_cast<long double>(time_spec_.tv_nsec - time_spec.tv_nsec)/(1.0e6);


        fout.open("./data/baseline_results_N="+std::to_string(N)+"_bits="+std::to_string(bits), std::ofstream::out | std::ofstream::app);
        fout<<"N="<<N<<", "<<"Bits="<<bitComplexity/9<<", "<<"Time="<<std::to_string(static_cast<long double>(time_s*1000+time_ms))<<endl;
        fout.close();
        /*close the server*/
        loop_->quit();
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
            connectionList.push_back(conn);
            counter[conn] = numRange;
        }
        void handleReadEvents(std::shared_ptr<socketx::Connection> conn){
            if(stage==1){
                ofstream fout("./data/baseline_aggregator_logs",ofstream::out | ofstream::app);
                fout<<"fd = "<<conn->getFD()<<" "<<"stage = "<<stage<<"\n";
                fout.close();

                stage = stage1(conn);
            }
            else if(stage==2){
                ofstream fout("./data/baseline_aggregator_logs",ofstream::out | ofstream::app);
                fout<<"fd = "<<conn->getFD()<<" "<<"stage = "<<stage<<"\n";
                fout.close();

                stage = stage2(conn,loop_);
            }
        }
        void handleCloseEvents(std::shared_ptr<socketx::Connection> conn){
            printf("Close connection...\n");
            auto it = std::find(connectionList.begin(),connectionList.end(),conn);
            connectionList.erase(it);
            stage1_map.erase(*it);
            stage2_num_map.erase(*it);
            stage2_count_map.erase(*it);
            counter.erase(*it);
        }

    private:
        socketx::EventLoop *loop_;
        std::shared_ptr<socketx::Server> server_;
        std::string port_;

        /*Record stage*/
        int stage;
        unsigned long long numRange = static_cast<unsigned long long>(std::pow(N,2));
};


int main(int argc, char **argv){
    if(argc!=2){
        fprintf(stderr,"usage: %s <port>\n", argv[0]);
        exit(0);
    }

    std::string port(argv[1]);

    socketx::EventLoop loop;
    Aggregator server(&loop,port);

    ofstream fout("./data/baseline_aggregator_logs");
    fout<<"Start the experiment of "<<N<<" participants. "<<"M = "<<M<<endl;
    fout.close();
    
    /*Start server*/
    server.start(); 
    loop.loop();

    return 0;
}