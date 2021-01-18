#include "PIT.h"
//unordered_map<string, unordered_set<pair<string, unsigned short>>> ContentName2IPPort;

PIT::PIT(){}
PIT::~PIT(){
    if(PIT::pitInstance){
        delete PIT::pitInstance;
        PIT::pitInstance = NULL;    
    }
}

PIT* PIT::pitInstance = NULL;
std::mutex pitmtx;

PIT* PIT::GetInstance(){
    if(pitInstance == NULL){
        std::lock_guard<mutex> lck(pitmtx);
        if(pitInstance == NULL){
            pitInstance = new PIT();
        }
    }
    return pitInstance;
}

void PIT::insertIpAndPortByContentName(string name, string IP, unsigned short port){
    std::lock_guard<mutex> InsertPITLock(pitmtx);
    auto it = ContentName2IPPort.find(name);
    if(it == ContentName2IPPort.end()){
        unordered_set<std::pair <string, unsigned short>, pair_hash> IPPortSet;
        IPPortSet.insert(make_pair(IP, port));
        ContentName2IPPort[name] = IPPortSet;
    }
    else{
        ContentName2IPPort[name].insert(make_pair(IP, port));
    }
}

void PIT::deleteIpAndPortByContentName(string name, string IP, unsigned short port){
    std::lock_guard<mutex> DelPITLock(pitmtx);
    auto it = ContentName2IPPort.find(name);
    if(it != ContentName2IPPort.end()){
        it->second.erase(make_pair(IP, port));

        if(it->second.size() == 0){
            ContentName2IPPort.erase(it);
        }
    }
}

vector<pair<string, unsigned short>> PIT::getPendingFace(string name){
    
    std::lock_guard<mutex> PITLock(pitmtx);
    /**
     * 有此情况下会用到这个函数：收到Data包之后，会查找PIT表来获得应该转发的端口．　Data包都是以segment包粒度进行传输的
     * 有两种情况：第一种Interest请求是以文件粒度来发送的，另一种是以segment包粒度发送的
     * 需要把这两种情况所有等待转发的接口一起获得
     * 值得一提的是，ContentName2IPPort 里面包含的segment包粒度的包，当数据来之后，转发回源端口之后应该删除;而文件粒度则不用
     */
    vector<pair<string, unsigned short>> ret;
    unordered_set<pair<string, unsigned>, pair_hash> ret_set;
    //因为只有接收到DataPackage之后会用到这个函数，所以一定都是都是包粒度的
    string upperName = getUpperName(name);
    if(upperName.empty()){
        cout << "[Error] Invalid Content Name In Data Package, name is: " << name << endl;
        return ret;
    }
    // 1. 在map中查找 pku/eecs/file/test1.txt之类的包
    auto it = ContentName2IPPort.find(upperName);
    //直接找到了，说明是类似pku/eecs/video/testfile.txt/segment1这种
    if(it != ContentName2IPPort.end()){    
        auto IPPortSet = it->second;
        for(auto itpair = IPPortSet.begin(); itpair != IPPortSet.end(); itpair++){
            ret_set.insert(*itpair);
        }
    }
    // 2. 还有一种情况，是InterestPackage里面发送的是segment粒度的请求，这些也会在ContentName2IPPort中存储,也需要发送至源端
    it = ContentName2IPPort.find(name);
    if(it != ContentName2IPPort.end()){    
        auto IPPortSet = it->second;
        for(auto itpair = IPPortSet.begin(); itpair != IPPortSet.end(); itpair++){
            ret_set.insert(*itpair);
        }
        ContentName2IPPort.erase(name);
    }
    
    // 3. 还原成vector
    if(ret_set.size() > 0){
        for(auto iterpair = ret_set.begin(); iterpair != ret_set.end(); iterpair++){
            ret.push_back(*iterpair);
        }
    }
    
    if(ret.size() == 0){
        cout << "[Info] No Content Name in ContentName2IPPort, name is: " << name << endl;
    }

    return ret;
}

/**
 * 这里需要注意的一点是：这个name直接是pku/eecs/file/test.mp3 而没有segment. 
 * 这是因为作为视频流，传输效率第一，并不关心包传输的顺序
 * 所以只要是这一个流下的包直接转发即可
 */
vector<pair<string, unsigned short>> PIT::getVideoPendingFace(string name){
    std::lock_guard<mutex> PITVideoLock(pitmtx);
    vector<pair<string, unsigned short>> ret;

    auto it = ContentName2IPPort.find(name);
    if(it != ContentName2IPPort.end()){    
        auto IPPortSet = it->second;
        for(auto itpair = IPPortSet.begin(); itpair != IPPortSet.end(); itpair++){
            ret.push_back(*itpair);
        }
    }
    return ret;
}

vector<pair<string, unsigned short>> PIT::getMsgPendingFace(string name){
    std::lock_guard<mutex> PITMsgLock(pitmtx);
    vector<pair<string, unsigned short>> ret;

    auto it = ContentName2IPPort.find(name);
    if(it != ContentName2IPPort.end()){    
        auto IPPortSet = it->second;
        for(auto itpair = IPPortSet.begin(); itpair != IPPortSet.end(); itpair++){
            ret.push_back(*itpair);
        }
    }
    return ret;
}

bool PIT::isContentNamePending(string name){

    std::lock_guard<mutex> GetContentPITLock(pitmtx);
    return ( ContentName2IPPort.find(name) != ContentName2IPPort.end() ); 
}

string PIT::getUpperContentName(string name){
    for(int i = name.size() - 1; i >= 0; i--){
        if(name[i] == '/'){
            return name.substr(0, i);
        }
    }
    return NULL;
}

void PIT::deleteContentName(string name){
    
    std::lock_guard<mutex> DeletePITLock(pitmtx);
    auto it = ContentName2IPPort.find(name);
    if(it != ContentName2IPPort.end()){
        ContentName2IPPort.erase(it);
    }
    return;
}

string PIT::getUpperName(string name){
    string upperName = "";
    size_t position = name.find("segment");
    if(position == string::npos){
        return upperName;
    }
    return name.substr(0, position - 1);
}
void PIT::printPIT(){
    cout << "========================================="<<endl;
    for(auto iter = ContentName2IPPort.begin(); iter != ContentName2IPPort.end(); iter++){
        auto IPPortSet = iter->second;
        cout << "ContentName: "<<iter->first << "  ";
        for(auto iter2 = IPPortSet.begin(); iter2 != IPPortSet.end(); iter2++){
            cout << "  IP: " << iter2->first << "Port: " << iter2->second;
        }
        cout << endl;
    }
    cout << "========================================="<<endl;
}