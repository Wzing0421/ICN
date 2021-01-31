#include "VideoProc.h"

VideoProc::VideoProc(){
    InitVideoProc();
}

VideoProc::~VideoProc(){

}

void VideoProc::InitVideoProc(){
    Json::Value root;
    Json::Reader reader;
    ifstream ifs("Settings.json");
    if(!reader.parse(ifs, root)){
        printf("[Error] Fail to parse Settings.json");
        return;
    }
    
    //parse variables
    VideoPort = (unsigned short) root["VideoPort"].asUInt();
    ifs.close();

    // bind port by IntestSocket
    udpVideoSocket.create(VideoPort);
    // get Singleton instance
    pitInstance = PIT::GetInstance();
    fibInstance = FIB::GetInstance();
}

void VideoProc::procVideoPackage(){
    char recvDataBuf[1500];
    string srcip_;
    unsigned short sport_;
    int lenrecv;
    /**
     * 数据包应该都是以包为粒度，也就是包括了segment如
     * pku/eecs/video/test.mp3/segment1 这样的形式
     * 接收到DataPackage的时候，查一下UpperName和对应的对应的转发端口
     * 如果在则转发，如果不在则丢弃
     */
    unsigned long long count = 0;
    while (true)
    {
        count++;
        lenrecv = udpVideoSocket.recvbuf(recvDataBuf, 1500, srcip_, sport_);
        if(lenrecv < 0){
            cout << "[Error] udpVideoSocket recv error" << endl;
            break;
        }
        char* contentName = new char[50];
        memcpy(contentName, recvDataBuf, 50);
        string name = contentName;
        // 请注意这里直接根据流的名字　pku/eecs/video/test.mp3来找转发端口，不需要upperName
        printf("Content Name is: %s, count is: %lld\n", name.c_str(), count);
        vector<pair<string, unsigned short>> pendingFaceVec = getVideoPendingFaceInPIT(name);
        if(!judgeLastLayer()){
            // only the last layer can transmit multiple cast packages
            vector<pair<string, unsigned short>> filteredPendingFaceVec;
            for(int i = 0; i < pendingFaceVec.size(); i++){
                if(!judegMulCastIP(pendingFaceVec[i].first)) filteredPendingFaceVec.push_back(pendingFaceVec[i]);
            }
            if(filteredPendingFaceVec.size() > 0){
                for(int i = 0; i < filteredPendingFaceVec.size(); i++){
                    udpVideoSocket.sendbuf(recvDataBuf, lenrecv, filteredPendingFaceVec[i].first, filteredPendingFaceVec[i].second);
                }
            }
        }
        else{
            //有则直接转发, 没有则直接丢弃
            if(pendingFaceVec.size() > 0){
                for(int i = 0; i < pendingFaceVec.size(); i++){
                    udpVideoSocket.sendbuf(recvDataBuf, lenrecv, pendingFaceVec[i].first, pendingFaceVec[i].second);
                }
            }
        }
        delete [] contentName;
    }
    udpVideoSocket.Close();
}

vector<pair<string, unsigned short>> VideoProc::getVideoPendingFaceInPIT(string name){
    return pitInstance->getVideoPendingFace(name);
}

bool VideoProc::judgeLastLayer(){
    return (fibInstance->getLayer() == 2);
}

bool VideoProc::judegMulCastIP(string IP){
    string IP1 = "224.0.0.0";
    string IP2 = "239.255.255.255";
    return ((IP >= IP1) && (IP <= IP2));
}