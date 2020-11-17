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
    
    vector<pair<string, unsigned short>> ret;
    //需要分两种情况：
    // 比如说 pku/eecs/video/testfile.txt/segment1 这类具体包数据应该直接能够找到
    auto it = ContentName2IPPort.find(name);
    //直接找到了，说明是类似pku/eecs/video/testfile.txt/segment1这种
    if(it != ContentName2IPPort.end()){
        auto IPPortSet = it->second;
        for(auto itpair = IPPortSet.begin(); itpair != IPPortSet.end(); itpair++){
            ret.push_back(*itpair);
        }
        return ret;
    }
    else{
        //没找到，说明需要向上级寻找,将pku/eecs/video/testfile.txt/segment1 转换成 pku/eecs/video/testfile.txt 再次寻找
        string UpperStr = getUpperContentName(name);

        //理论上不会出现字符串不合规的情况，但是一旦出现要进行处理
        if(UpperStr.empty()) return ret;
        
        auto uppiter = ContentName2IPPort.find(UpperStr);
        //如果经过上级寻找找到了
        if(uppiter != ContentName2IPPort.end()){
            auto IPPortSet = uppiter->second;
            for(auto itpair = IPPortSet.begin(); itpair != IPPortSet.end(); itpair++){
                ret.push_back(*itpair);
            }
        }
        return ret;
    }
    return ret;
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
    auto it = ContentName2IPPort.find(name);
    if(it != ContentName2IPPort.end()){
        ContentName2IPPort.erase(it);
    }
    return;
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