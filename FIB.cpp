#include "FIB.h"

FIB::FIB(){
    initFIB();
}

FIB::~FIB(){

}


FIB* FIB::fibInstance = NULL;
std::mutex fibmtx;

FIB* FIB::GetInstance(){
    if(fibInstance == NULL){
        std::lock_guard<mutex> lck(fibmtx);
        if(fibInstance == NULL){
            fibInstance = new FIB();
        }
    }
    return fibInstance;
}

void FIB::initFIB(){
    Json::Value root;
    Json::Reader reader;

    ifstream ifs("FIBSettings.json");
    if(!reader.parse(ifs, root)){
        printf("[Error] Fail to parse FIBSettings.json");
        return;
    }
    
    //parse local names
    LocalName = root["Name"].asString();
    layer = -1;
    position = -1;
    layer2sz = -1;

    //parse fib structure
    for(int i = 0; i < 3; i++){
        string layerstr = "layer" + to_string(i);
        vector<pair<string, string>> NameIPVec;
        int layersz = root[layerstr].size();
        for(int j = 0; j < layersz; j++){
            Json::Value jsval = root[layerstr][j];
            string name = jsval["name"].asString();
            if(name == LocalName){
                layer = i;
                position = j;
            }
            string ip = jsval["IP"].asString();
            NameIPVec.push_back(make_pair(name, ip));
        }
        FIBstructure.push_back(NameIPVec);
    }
    layer2sz = root["layer2"].size();
    if(layer == -1){
        cout << "[Error] No name match in FIB settings!" << endl;
    }
    if(layer2sz == -1){
        cout << "[Error] layer2 error in FIB settings!" << endl;
    }

    //获得同级转发列表
    formForwardingIPlist(layer, root);

    ifs.close();
}

bool FIB::isMatchLocalNames(string name){

    //为空表示不合法 或者localNames没找到相应的字符串则说明不是本地字符串
    if(name.empty() || localNames.find(name) == localNames.end()){
        return false;
    }
    return true;
}

/**
 * 从 pku/eecs/file/test1.txt/segment1 应该转换成pku/eecs/file/test1.txt再查找应该转发的接口
 * 其实这个函数没用，反正是返回同级的转发逻辑和接口
 */
string FIB::getUpperContent(string name){
    int position = name.find("segment");
    if(position == string::npos) return name;
    return name.substr(0, position - 1);
}

vector<string> FIB::getForwardingFaces(string name){
    return ForwardingIPlist;
}

void FIB::formForwardingIPlist(int layer, Json::Value &root){
    /**
     * 对于第一层，也就是地面的ICN节点，需要所有节点轮询一遍，注意去掉自己
     */
    string layerstr = "layer" + to_string(layer);
    if(layer == 1){
        for(int i = 0; i < root[layerstr].size(); i++){
            Json::Value jsval = root[layerstr][i];
            string name = jsval["name"].asString();
            if(name != LocalName){
                ForwardingIPlist.push_back(jsval["IP"].asString());
            }
        }
    }
    /**
     * 对于第二层，也就是高轨卫星GEO,需要询问其左右两个
     */
    else if(layer == 2){
        int leftpos = (position + layer2sz - 1) % layer2sz; 
        int rightpos = (position + 1) % layer2sz;
        Json::Value jsleftval = root[layerstr][leftpos];
        ForwardingIPlist.push_back(jsleftval["IP"].asString());
        Json::Value jsrightval = root[layerstr][rightpos];
        ForwardingIPlist.push_back(jsrightval["IP"].asString());
    }
    else{
        cout << "[Error] Invalid layer!" << endl;
    }
}

string FIB::getUpperLevelForwardingIP(){
    return FIBstructure[layer - 1][0].second;   
}

string FIB::getLowerLevelForwardingIP(){
    string ret = "";
    if(layer == 2) return ret;
    else return FIBstructure[layer + 1][0].second;
    
}

int FIB::getLayer(){
    return layer;
}