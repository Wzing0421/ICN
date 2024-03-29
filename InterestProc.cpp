#include "InterestProc.h"

InterestProc::InterestProc(){
    InitInterestProc();
}
InterestProc::~InterestProc(){

}


void InterestProc::InitInterestProc(){
    Json::Value root;
    Json::Reader reader;
    ifstream ifs("Settings.json");
    if(!reader.parse(ifs, root)){
        printf("[Error] Fail to parse Settings.json");
        return;
    }
    
    //parse variables
    InterestPort = (unsigned short) root["InterestPort"].asUInt();
    DataPort = (unsigned short) root["DataPort"].asUInt();
    InquirePort = (unsigned short) root["InquirePort"].asUInt();
    InquireBindPort = (unsigned short) root["InquireBindPort"].asUInt();
    tv_sec = root["tv_sec"].asInt();
    tv_usec = root["tv_usec"].asInt();
    ifs.close();

    // bind port by IntestSocket
    udpInterestSocket.create(InterestPort);
    // bind port by inquireSocket
    inquireSocket.create(InquireBindPort);

    // get Sigleton instance
    cslruInstance = CSLRU::GetInstance(100);
    fibInstance = FIB::GetInstance();
    pitInstance = PIT::GetInstance();
    

}

void InterestProc::procInterestPackage(){
    char recvInterestBuf[100];
    string srcip_;
    unsigned short sport_;
    /**
     * 我当前的逻辑是，发送的兴趣包应该是统一的格式，都是 pku/eecs/ICN_EGS_1/ICN_GEO_1/file/testFile.txt/segment1 这类形式， 也就是说都有segment部分
     * 先完成规范格式的开发，对于pku/eecs/ICN_EGS_1/ICN_GEO_1/file/testFile.txt这样的格式交由上层处理，先不用管，都是分成最细的包的形式
     * 11月23日修改：
     * 发送的兴趣包的逻辑应该是：正常情况下发送的是：pku/eecs/file/test1.txt 这种．当上层的服务发现有一些包丢失需要重传，也会发送类似于pku/eecs/file/test1.txt/segment1 这种Interest包
     */
    while (true)
    {
        int lenrecv = udpInterestSocket.recvbuf(recvInterestBuf, 100, srcip_, sport_);
        if(lenrecv < 0){
            cout << "[Error] udpInterestSocket recv error" << endl;
            break;
        }
        InterestPackage interestPackage;
        memcpy(&interestPackage, recvInterestBuf, sizeof(interestPackage));
        string name = interestPackage.contentName;
        int subscribe = interestPackage.Subscribe;
        int type = interestPackage.type;
        
        //先获得客户端编号
        string client_num = allocateClientName(srcip_);
        
        if(subscribe == 1){
            //先打印订阅信息
            
            cout << "Client " << client_num << " submits subscription request, request content name is : " << name << endl;
            //订阅操作
            //type == 0 是文件 
            if(type == 0){

                vector<DataPackage> ContentDataVec = getContentStoreDataList(name);

                // 找到了所需要的包
                if(ContentDataVec.size() != 0){
                    cout << "[Info]: Find Package in Content Store!" << endl;
                    for(int i = 0; i < ContentDataVec.size(); i++){
                        char sendbuffer[1500];
                        memcpy(sendbuffer, &ContentDataVec[i], sizeof(sendbuffer));
                        udpInterestSocket.sendbuf(sendbuffer, sizeof(sendbuffer), srcip_, DataPort);
                        cout << "[info] send data to " << srcip_ << ":" << DataPort << " ContentName: " << ContentDataVec[i].contentName << " SegmentNum: " << ContentDataVec[i].segmentNum << " End: " << ContentDataVec[i].end << endl;
                        usleep(10000);
                    }
                }
                //在content Store中没有找到
                else{
                    //首先在PIT表中找这个ContentName是不是已经在等待了,如果找到了则更新PIT表并丢弃Interest包
                    if(isNameExistInPIT(name)){
                        /**1月22日更新:
                         * 对于订阅请求，Interest请求端的端口已经固定成51001, 而由于设定接收端单独订阅的文件和短消息都是在51002端口接收,所以对于subscribe的时候直接源端口加1就可以
                         * 而对于组播类的接收则不是51002而需要从数据库配置中心读取
                         */
                        Subscribe(name, srcip_, sport_ + 1);
                        cout << "[1]Insert Into PIT: " << name << " IP: " << srcip_ << " Port: " << sport_ + 1 << endl;
                    }
                    /**
                    * PIT表中没有，需要查找本级节点有无
                    * 有则转发至本级节点，无则转发至上级默认ICN节点
                    */
                    else{

                        //向PIT表中插入这条转发信息
                        Subscribe(name, srcip_, sport_ + 1);
                        cout << "[2]Insert Into PIT: " << name << " IP: " << srcip_ << " Port: " << sport_ + 1 << endl;

                        vector<string> forwardingFaceList = getForwardingFaces(name);
                
                        //先向本级查询，如果没有结果，则向上级查询
                        string IPForForwarding = getThisLevelIPIfContentExist(forwardingFaceList, interestPackage);
                
                        //如果能在同级别找到，则将Interest包转发到同级别IP上
                        if(!IPForForwarding.empty()){
                            udpInterestSocket.sendbuf(recvInterestBuf, 100, IPForForwarding, InterestPort);
                            cout << "[3]Forwarding Interest Package to same level: " << name << " IP: " << IPForForwarding << " Port: " << InterestPort << endl;
                        }
                        //否则转发到默认的上级ICN节点上
                        else{
                            string upperIP = getUpperLevelIP();
                            udpInterestSocket.sendbuf(recvInterestBuf, 100, upperIP, InterestPort);
                            cout << "[3]Forwarding Interest Package to upper level: " << name << " IP: " << upperIP << " Port: " << InterestPort << endl;
                        }
                    }
                }
            }
            else if(type == 1){
                // type == 1 是视频流
                /**
                 * 订阅视频流的时候需要注意的是: 视频流的订阅，不仅仅需要向本地插入，还要向上级报告
                 * 因为源端发送视频流一定需要找到每一个请求端才行
                 */ 
                //直接插入，有重复的可以去重
                //需要注意的是，对于订阅的视频消息，需要注册在51005端口，因为对于请求的客户端，ICN会统一发送至51005端口，那么就需要用51005注册订阅请求
                Subscribe(name, srcip_, sport_ + 4);    
                cout << "[Video]Insert Into PIT: " << name << " IP: " << srcip_ <<  " Port: " << sport_ + 4 << endl;
                // 向上级ICN节点注册
                string upperIP = getUpperLevelIP();
                udpInterestSocket.sendbuf(recvInterestBuf, lenrecv, upperIP, InterestPort);
            }
            else if(type == 2){
                // type == 2 是短消息流
                //直接插入，有重复的可以去重,注意短消息和文件订阅都是51001所以都需要+1
                Subscribe(name, srcip_, sport_ + 1);    
                cout << "[Msg]Insert Into PIT: " << name << " IP: " << srcip_ << " Port: " << sport_ + 1 << endl;
                // 向上级ICN节点注册
                string upperIP = getUpperLevelIP();
                udpInterestSocket.sendbuf(recvInterestBuf, lenrecv, upperIP, InterestPort);
            }
        }
        else{

            cout << "Client " << client_num << " submits Unsubscription request, request content name is : " << name << endl;
            cout << endl;
            cout << "Before Unsubscribe:" << endl;
            pitInstance->printPIT();
            //取消订阅操作
            if(type == 1){ // video, 51005
                UnSubscribe(name, srcip_, sport_ + 4);
            }
            else{
                UnSubscribe(name, srcip_, sport_ + 1);
            }

            if(type == 1){
                // video unSubscribe
                vector<pair<string, unsigned short>> retvec = pitInstance->getVideoPendingFace(name);
                // 如果本ICN节点没有了订阅节点则向上级汇报
                if(retvec.size() == 0){
                    string upperIP = getUpperLevelIP();
                    udpInterestSocket.sendbuf(recvInterestBuf, 100, upperIP, InterestPort);
                }
            }
            else if(type == 2){
                // Msg unSubscribe
                vector<pair<string, unsigned short>> retvec = pitInstance->getMsgPendingFace(name);
                // 如果本ICN节点没有了订阅节点则向上级汇报
                if(retvec.size() == 0){
                    string upperIP = getUpperLevelIP();
                    udpInterestSocket.sendbuf(recvInterestBuf, 100, upperIP, InterestPort);
                }
            }
            
            cout << "After Unsubscribe:" << endl;
            pitInstance->printPIT();
        }
    }
    udpInterestSocket.Close();
}

vector<DataPackage> InterestProc::getContentStoreDataList(string name){
    return cslruInstance->getAllRelatedContentPackage(name);
}

bool InterestProc::isNameExistInPIT(string name){
    return pitInstance->isContentNamePending(name);
}

// The key method for subscribing 
void InterestProc::Subscribe(string name, string IP, unsigned short port){
    return pitInstance->insertIpAndPortByContentName(name, IP, port);
}

// The key method for unsubscribing
void InterestProc::UnSubscribe(string name, string IP, unsigned short port){
    return pitInstance->deleteIpAndPortByContentName(name, IP, port);
}

bool InterestProc::isMatchLocalNames(string name){
    string UpperName = fibInstance->getUpperContent(name);
    if(UpperName.empty()){
        //我还没完全确定当名字不合法的处理办法
        cout << "[Error] Name Not Valid!" << endl;
    }
    return fibInstance->isMatchLocalNames(UpperName);
}

bool InterestProc::isVideoStream(string name){
    return name.find("video") == name.npos; 
}

/**
 * 目前来看这个入参没什么意义，因为只要确定了层数，转发的接口是固定的
 */
vector<string> InterestProc::getForwardingFaces(string name){
    string UpperName = fibInstance->getUpperContent(name);
    return fibInstance->getForwardingFaces(UpperName);
}

string InterestProc::getThisLevelIPIfContentExist(vector<string> &forwardingFaceList, InterestPackage &interestpack){
    string ret = "";
    struct timeval tv;
    fd_set readfds;
    int lenrecv;
    string srcip_;
    unsigned short sport_;
    char sendbuffer[100];
    char recvbuffer[1500];
    
    //將InterestPackage轉換成InquirePackage
    InquirePackage inquirepack(interestpack.contentName, 1, 0);
    memcpy(sendbuffer, &inquirepack, sizeof(inquirepack));
    for(int i = 0; i < forwardingFaceList.size(); i++){
        FD_ZERO(&readfds);
        FD_SET(inquireSocket.sock, &readfds);
        tv.tv_sec = tv_sec;
        tv.tv_usec = tv_usec;
        
        inquireSocket.sendbuf(sendbuffer, sizeof(sendbuffer), forwardingFaceList[i], InquirePort);
        select(inquireSocket.sock + 1, &readfds, NULL, NULL, &tv);
        if(FD_ISSET(inquireSocket.sock, &readfds)){
            if((lenrecv = inquireSocket.recvbuf(recvbuffer, sizeof(recvbuffer), srcip_, sport_)) >= 0){
                InquirePackage inquirepack1;
                memcpy(&inquirepack1, recvbuffer, sizeof(InquirePackage));
                
                //return srcip_ if there exists content data
                if(inquirepack1.inquire == 0 && inquirepack1.answer == 1){
                    cout << "[Info]: Inquirement get response from " << srcip_ << " Content Name is " << inquirepack1.contentName << endl;
                    return srcip_;
                }
                
            }
        }
        else{
            cout << "[Info] Timeout from " << forwardingFaceList[i] << endl;
        }
    }
    return ret;
}

string InterestProc::allocateClientName(string IP){
    if(clientName.find(IP) != clientName.end()){
        return to_string(clientName[IP]);
    }
    else{
        for(int i = 1; i < clientName.size(); i++){
            bool find = false;
            for(auto iter = clientName.begin(); iter != clientName.end(); iter++){
                if(iter->second == i){
                    find = true;
                    break;
                }
            }
            if(!find){
                clientName[IP] = i;
                return to_string(i);
            }
        }
        clientName[IP] = clientName.size() + 1;
        return to_string(clientName[IP]);
    }
}

string InterestProc::getUpperLevelIP(){
    return fibInstance->getUpperLevelForwardingIP();
}

void InterestProc::printInfo(InterestPackage interestpack){
    cout << "================Interest Package================" << endl;
    cout << "[contentName] : " << interestpack.contentName << endl;
    cout << "================================================" << endl;
}