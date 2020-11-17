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
    ifs.close();

    // bind port by IntestSocket
    udpInterestSocket.create(InterestPort);

    // get Sigleton instance
    cslruInstance = CSLRU::GetInstance();
    pitInstance = PIT::GetInstance();
    fibInstance = FIB::GetInstance();

}

void InterestProc::procInterestPackage(){
    return;
}