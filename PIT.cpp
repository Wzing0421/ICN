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

vector<pair<string, unsigned short>> PIT::getPendingFace(string name){
    
    std::lock_guard<mutex> PITLock(pitmtx);
    /**
     * 有此情况下会用到这个函数：收到Data包之后，会查找PIT表来获得应该转发的端口．　Data包都是以segment包粒度进行传输的
     * 正常情况下，Interest会发送UpperName也就是文件粒度的ContentName, 而Data包则以segmentName为粒度进行传输
     * 先考虑正常情况,也就是Interest包以文件粒度进行传输，Data包以segmentName为粒度进行传输
     */
    vector<pair<string, unsigned short>> ret;
    
    //因为只有接收到DataPackage之后会用到这个函数，所以一定都是都是包粒度的
    string upperName = getUpperName(name);
    if(upperName.empty()){
        cout << "[Error] Invalid Content Name In Data Package, name is: " << name << endl;
        return ret;
    }
    //在map中查找 pku/eecs/file/test1.txt之类的包
    auto it = ContentName2IPPort.find(upperName);
    //直接找到了，说明是类似pku/eecs/video/testfile.txt/segment1这种
    if(it != ContentName2IPPort.end()){
        auto IPPortSet = it->second;
        for(auto itpair = IPPortSet.begin(); itpair != IPPortSet.end(); itpair++){
            ret.push_back(*itpair);
        }
        return ret;
    }
    else{
        //没找到
        cout << "[Info] No Content Name in ContentName2IPPort, name is: " << name << endl;
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