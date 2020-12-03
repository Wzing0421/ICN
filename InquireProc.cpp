#include "InquireProc.h"

InquireProc::InquireProc(){
    InitInquireProc();
}
InquireProc::~InquireProc(){

}
void InquireProc::InitInquireProc(){
    Json::Value root;
    Json::Reader reader;
    ifstream ifs("Settings.json");
    if(!reader.parse(ifs, root)){
        printf("[Error] Fail to parse Settings.json");
        return;
    }
    
    //parse variables
    InquirePort = (unsigned short) root["InquirePort"].asUInt();
    ifs.close();

    udpInquireSocket.create(InquirePort); 

    // get Sigleton instance
    cslruInstance = CSLRU::GetInstance(100);       
}

void InquireProc::procInquire(){
    char recvInquireBuf[100];
    string srcip_;
    unsigned short sport_;
    int lenrecv;
    while (true)
    {
        lenrecv = udpInquireSocket.recvbuf(recvInquireBuf, 100, srcip_, sport_);
        if(lenrecv < 0){
            cout << "[Error] udpDataSocket recv error" << endl;
            break;
        }
        InquirePackage inquirepack;
        memcpy(&inquirepack, recvInquireBuf, sizeof(InquirePackage));
        
        //只接收询问包
        if(inquirepack.inquire != 1) continue;
        string name = inquirepack.contentName;
 
        if(isContentStoreDataExist(name)){
            InquirePackage answerpack(name.c_str(), 0, 1);
            char sendbuffer[100];
            memcpy(sendbuffer, &answerpack, sizeof(sendbuffer));
            udpInquireSocket.sendbuf(sendbuffer, sizeof(sendbuffer), srcip_, sport_);
        }
        else{
            InquirePackage answerpack(name.c_str(), 0, 0);
            char sendbuffer[100];
            memcpy(sendbuffer, &answerpack, sizeof(sendbuffer));
            udpInquireSocket.sendbuf(sendbuffer, sizeof(sendbuffer), srcip_, sport_);
        }
    }
    udpInquireSocket.Close();
}

bool InquireProc::isContentStoreDataExist(string name){
    return cslruInstance->IsDataPackageInContentStore(name);
}