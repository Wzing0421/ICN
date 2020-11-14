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