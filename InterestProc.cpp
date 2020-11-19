#include "InterestProc.h"
#include "package.h"

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
    ifs.close();

    // bind port by IntestSocket
    udpInterestSocket.create(InterestPort);

    // get Sigleton instance
    cslruInstance = CSLRU::GetInstance(100);
    pitInstance = PIT::GetInstance();
    fibInstance = FIB::GetInstance();

}

void InterestProc::procInterestPackage(){
    char recvInterestBuf[100];
    string srcip_;
    unsigned short sport_;
    /**
     * 我当前的逻辑是，发送的兴趣包应该是统一的格式，都是 pku/eecs/ICN_EGS_1/ICN_GEO_1/file/testFile.txt/segment1 这类形式， 也就是说都有segment部分
     * 先完成规范格式的开发，对于pku/eecs/ICN_EGS_1/ICN_GEO_1/file/testFile.txt这样的格式交由上层处理，先不用管，都是分成最细的包的形式
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
        vector<string> contentNameVec = getContentStoreNameList(name);
        cout << contentNameVec.size()<<endl;

        // 找到了所需要的包
        if(contentNameVec.size() != 0){
            cout << "[Info]: Name in Content Store!" << endl;
            for(int i = 0; i < contentNameVec.size(); i++){
                //将包封装成datapakcage并发送
                char* data = getContentStoreData(contentNameVec[i]);
                int length = getContentStoreDataLength(contentNameVec[i]);
                DataPackage dataPackage(contentNameVec[i].c_str(), data, length);
                char sendbuffer[1460];
                memcpy(sendbuffer, &dataPackage, sizeof(sendbuffer));
                udpInterestSocket.sendbuf(sendbuffer, sizeof(sendbuffer), srcip_, DataPort);
                cout << "[info] send data to " << srcip_ << ":" << DataPort << " ContentName: " << contentNameVec[i] << " Data: " << dataPackage.data <<endl;
            }
        }
        // 在content Store中没有找到
        else{
            //首先在PIT表中找,如果找到了则更新PIT表并丢弃Interest包
            if(isNameExistInPIT(name)){
                // 这里面port写sport_或者dataPort都行，因为数据转发固定是DataPort不会看存入这个PIT的port是什么
                this->insertIpAndPortByContentName(name, srcip_, sport_);
                cout << "[1]Insert Into PIT: " << name << " IP: " << srcip_ << " Port: " << sport_ << endl;
            }
            //PIT表中没有
            else{
                /**
                 * 首先查询FIB表中有无可以转发的路径
                 * 1）如果是指向本地的请求（前面已经没用命中ContentStore了）或者没能在FIB中找到转发接口则丢弃
                 * 2）在FIB中找到转发接口，则将其转发
                 */
                if(!isMatchLocalNames(name)){
                    vector<pair<string, unsigned short>> pendingFace = getForwardingFaces(name);
                    /*必须是找到了，才能够更新PIT表，否则就直接扔掉了*/
                    if(pendingFace.size() > 0){

                        // 1. 先向PIT表中插入表项   
                        insertIpAndPortByContentName(name, srcip_, sport_);
                        cout << "[2]Insert Into PIT: " << name << " IP: " << srcip_ << " Port: " << sport_ << endl;
                        // 2. 将Interest包转发给应该去的地方
                        for(int i = 0; i < pendingFace.size(); i++){
                            udpInterestSocket.sendbuf(recvInterestBuf, 100, pendingFace[i].first, pendingFace[i].second);
                            cout << "[Info] Sending Interest Package to " << pendingFace[i].first << ":" << pendingFace[i].second << endl;
                        }
                    }
                    else{
                        cout << "No match in FIB" << endl;
                    }
                }
            }
        }
    }
    udpInterestSocket.Close();
}

vector<string> InterestProc::getContentStoreNameList(string name){
    return cslruInstance->getAllRelatedContentPackage(name);
}

char* InterestProc::getContentStoreData(string name){
    return cslruInstance->getContentData(name);
}

int InterestProc::getContentStoreDataLength(string name){
    return cslruInstance->getContentLength(name);
}

bool InterestProc::isNameExistInPIT(string name){
    return pitInstance->isContentNamePending(name);
}

void InterestProc::insertIpAndPortByContentName(string name, string IP, unsigned short port){
    return pitInstance->insertIpAndPortByContentName(name, IP, port);
}

bool InterestProc::isMatchLocalNames(string name){
    string UpperName = fibInstance->getUpperContent(name);
    if(UpperName.empty()){
        //我还没完全确定当名字不合法的处理办法
        cout << "[Error] Name Not Valid!" << endl;
    }
    return fibInstance->isMatchLocalNames(UpperName);
}

vector<pair<string, unsigned short>> InterestProc::getForwardingFaces(string name){
    string UpperName = fibInstance->getUpperContent(name);
    return fibInstance->getForwardingFaces(UpperName);
}