#include "FIB.h"

FIB::FIB(){
    initFIB();
}

FIB::~FIB(){

}

void FIB::initFIB(){
    Json::Value root;
    Json::Reader reader;

    ifstream ifs("FIBSettings.json");
    if(!reader.parse(ifs, root)){
        printf("[Error] Fail to parse FIBSettings.json");
        return;
    }
    
    //parse local content names
    string Location = root["Location"].asString();
    localNames.insert(Location);
    int contentsz = root["LocalContent"].size();
    for(int i = 0; i < contentsz; i++) {
        localNames.insert(root["LocalContent"][i].asString());
    }

    //parse FIB Tables
    int fibsz = root["forwardingTable"].size();
    for(int i = 0; i < fibsz; i++){
        Json::Value jsval = root["forwardingTable"][i];
        string contentName = jsval["LocationName"].asString();
        string IP = jsval["IP"].asString();
        unsigned short port = (unsigned short) jsval["port"].asUInt();

        unordered_set< pair<string, unsigned short>, pair_hash_fib > IPPortSet;
        IPPortSet.insert(make_pair(IP, port));
        ContentNameForwardMap[contentName] = IPPortSet;
    }
    // print FIB table
    cout << "=======================" << endl;
    for(auto it = localNames.begin(); it != localNames.end(); it++){
        cout << *it << " ";
    }
    cout << endl;
    cout << "-----------------------" << endl;
    for(auto it = ContentNameForwardMap.begin(); it != ContentNameForwardMap.end(); it++){
        auto myset = it->second;
        cout << "ContentName: " << it->first << " ";
        for(auto itset = myset.begin(); itset != myset.end(); itset++){
            cout << itset->first << " " << itset->second;
        }
        cout << endl;
    }
    cout << "=======================" << endl;
}

bool FIB::isMatchLocalNames(string name){

    //为空表示不合法 或者localNames没找到相应的字符串则说明不是本地字符串
    if(name.empty() || localNames.find(name) == localNames.end()){
        return false;
    }
    return true;
}

/**
 * 从 pku/eecs/ICN_EGS_1/ICN_GEO_1/video/testfile.txt/segment1 应该转换成pku/eecs/ICN_EGS_1/ICN_GEO_1
 * 我这里ContentName确定一定会有file video msg三种之间的任意一种。所以根据这个找到上级就行
 */
string FIB::getUpperContent(string name){
    int position;
    string ret = "";
    if(string::npos != (position = name.find("file")) ){
        ret = name.substr(0, position - 1);
    }
    else if(string::npos != (position = name.find("video")) ){
        ret = name.substr(0, position - 1);
    }
    else if(string::npos != (position = name.find("msg")) ){
        ret = name.substr(0, position - 1);
    }
    return ret;
}

vector<pair<string, unsigned short>> FIB::getForwardingFaces(string name){
    vector<pair<string, unsigned short>> ret;
    auto pairiter = ContentNameForwardMap.find(name);
    if(pairiter == ContentNameForwardMap.end()){
        return ret;
    }
    unordered_set< pair<string, unsigned short>, pair_hash_fib > pairset = pairiter->second;
    for(auto it = pairset.begin(); it != pairset.end(); it++){
        ret.push_back(*it);
    }
    return ret;
}