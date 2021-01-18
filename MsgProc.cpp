#include "MsgProc.h"

MsgProc::MsgProc(){
    InitMsgProc();
}

MsgProc::~MsgProc(){

}

void MsgProc::InitMsgProc(){
    Json::Value root;
    Json::Reader reader;
    ifstream ifs("Settings.json");
    if(!reader.parse(ifs, root)){
        printf("[Error] Fail to parse Settings.json");
        return;
    }
    
    //parse variables
    MsgPort = (unsigned short) root["MsgPort"].asUInt();
    ifs.close();

    // bind port by IntestSocket
    udpMsgSocket.create(MsgPort);
    // get Singleton instance
    pitInstance = PIT::GetInstance();
}

void MsgProc::procMsgPackage(){
    char recvDataBuf[1500];
    string srcip_;
    unsigned short sport_;
    int lenrecv;
    /**
     * 数据包应该都是以包为粒度，也就是包括了segment如
     * pku/eecs/Hangzhen/Msg/segment1 这样的形式
     * 接收到DataPackage的时候，查一下UpperName和对应的对应的转发端口
     * 如果在则转发，如果不在则丢弃
     */
    while (true)
    {
        lenrecv = udpMsgSocket.recvbuf(recvDataBuf, 1500, srcip_, sport_);
        if(lenrecv < 0){
            cout << "[Error] udpMsgSocket recv error" << endl;
            break;
        }
        DataPackage dataPackage;
        memcpy(&dataPackage, recvDataBuf, sizeof(DataPackage));
        string name = dataPackage.contentName;

        string upperName = getUpperName(name);
        if(upperName == ""){
            cout << "[Error]: Msg ContentName Invalid: Name is: " << name << endl;
            continue;
        }
        printf("Msg Content Name is: %s\n", name.c_str());
        vector<pair<string, unsigned short>> pendingFaceVec = getMsgPendingFaceInPIT(upperName);
        
        //有则直接转发, 没有则直接丢弃
        if(pendingFaceVec.size() > 0){
            for(int i = 0; i < pendingFaceVec.size(); i++){
                udpMsgSocket.sendbuf(recvDataBuf, lenrecv, pendingFaceVec[i].first, MsgPort);
            }
        }
    }
    udpMsgSocket.Close();
}

vector<pair<string, unsigned short>> MsgProc::getMsgPendingFaceInPIT(string name){
    return pitInstance->getMsgPendingFace(name);
}

string MsgProc::getUpperName(string name){
    string upperName = "";
    size_t position = name.find("segment");
    if(position == string::npos){
        return upperName;
    }
    return name.substr(0, position - 1);
}
