#include "DataProc.h"

DataProc::DataProc(){
    InitDataProc();
}

DataProc::~DataProc(){

}

void DataProc::InitDataProc(){
    Json::Value root;
    Json::Reader reader;
    ifstream ifs("Settings.json");
    if(!reader.parse(ifs, root)){
        printf("[Error] Fail to parse Settings.json");
        return;
    }
    
    //parse variables
    InterestPort = (unsigned short) root["InterestPort"].asUInt();
    DataPort = (unsigned short) root["DataPort"].asUInt();
    ifs.close();

    // bind port by IntestSocket
    udpDataSocket.create(DataPort);
    // get Sigleton instance
    cslruInstance = CSLRU::GetInstance(100);
    pitInstance = PIT::GetInstance();
    fibInstance = FIB::GetInstance();
}

void DataProc::procDataPackage(){
    char recvDataBuf[1500];
    string srcip_;
    unsigned short sport_;
    /**
     * 数据包应该都是以包为粒度，也就是包括了segment如
     * pku/eecs/file/test1.txt/segment1 这样的形式
     * 接收到DataPackage的时候，先查找内容缓存，查一下UpperName和对应的这个segmentName是不是都在
     * 如果在则丢弃，如果不在则查找PIT表将其转发至等待源端口，并将其插入Content Store中
     * 详见<清华大学　信息中心网络>　108页
     */
    while (true)
    {
        int lenrecv = udpDataSocket.recvbuf(recvDataBuf, 1500, srcip_, sport_);
        if(lenrecv < 0){
            cout << "[Error] udpDataSocket recv error" << endl;
            break;
        }
        DataPackage dataPackage;
        memcpy(&dataPackage, recvDataBuf, sizeof(DataPackage));
        string name = dataPackage.contentName;
        
        // mesage data 和　file Data共用一个51002数据端口，　两者的区别是一个本地缓存另一个不本地缓存
        if(judgeFile(name)){
            // Content Store中没有这个DataPackage才可以进行下面的操作
            if(!IsDataPackageInContentStore(dataPackage)){

                cout << "[Info]: No DataPackage in Content Store! Package Name: " << dataPackage.contentName << " SegmentNum: " << dataPackage.segmentNum << " End: " << dataPackage.end << endl;

                /**
                * 先将 DataPackage 存入 Content Store 不论PIT中有没有数据都先存放
                * 这里涉及到发布的逻辑: 
                * 即使发布的时候没有订阅者也应该在本地缓存以供后续订阅者订阅
                */
                insertDataInContentStore(dataPackage);
            
                vector<pair<string, unsigned short>> pendingFaceVec = getPendingFaceInPIT(name);
                if(pendingFaceVec.size() > 0){
                    
                    //然后向源端口转发
                    for(int i = 0; i < pendingFaceVec.size(); i++){
                        udpDataSocket.sendbuf(recvDataBuf, sizeof(recvDataBuf), pendingFaceVec[i].first, pendingFaceVec[i].second);
                        cout << "[Info] Forwarding data package to: dstip " << pendingFaceVec[i].first << " ContentName: " << name << " Dstport: " << pendingFaceVec[i].second << endl;
                    }
                }
            }
            // 在content Store中找到，说明之前本地缓存有，已经还给了源端，直接丢弃即可
            else{
                cout << "Exist in Content Store" << endl;   
            }
        }
        else if(judgeMsg(name)){
            // 直接转发
            cout << "[Info]: Get Msg Datapacakge, Pacakge Name: " << dataPackage.contentName << " Data : " << dataPackage.data << endl;
            string upperName = getUpperName(name);
            vector<pair<string, unsigned short>> pendingFaceVec = getMsgPendingFaceInPIT(upperName);
            
            // if not the last layer, needs to filter the NON-multicast IP
            if(!judgeLastLayer()){
                // only the last layer can transmit multiple cast packages
                vector<pair<string, unsigned short>> filteredPendingFacaVec;
                for(int i = 0; i < pendingFaceVec.size(); i++){
                    if(!judegMulCastIP(pendingFaceVec[i].first)) filteredPendingFacaVec.push_back(pendingFaceVec[i]);
                }
                if(filteredPendingFacaVec.size() > 0){
                    //然后向源端口转发
                    for(int i = 0; i < filteredPendingFacaVec.size(); i++){
                        udpDataSocket.sendbuf(recvDataBuf, sizeof(recvDataBuf), filteredPendingFacaVec[i].first, filteredPendingFacaVec[i].second);
                        cout << "[Info] Forwarding data package to: dstip " << filteredPendingFacaVec[i].first << " ContentName: " << name  << " Dstport: " << filteredPendingFacaVec[i].second << endl;
                    }
                }
            }
            else{ // the last layer
                if(pendingFaceVec.size() > 0){        
                    //然后向源端口转发
                    for(int i = 0; i < pendingFaceVec.size(); i++){
                        udpDataSocket.sendbuf(recvDataBuf, sizeof(recvDataBuf), pendingFaceVec[i].first, pendingFaceVec[i].second);
                        cout << "[Info] Forwarding data package to: dstip " << pendingFaceVec[i].first << " ContentName: " << name  << " Dstport: " << pendingFaceVec[i].second << endl;
                    }
                }
            }
        }
        
    }
    udpDataSocket.Close();
}

void DataProc::insertDataInContentStore(DataPackage datapack){
    return cslruInstance->putContentNameAndDataAndLength(datapack);
}

vector<pair<string, unsigned short>> DataProc::getPendingFaceInPIT(string name){
    return pitInstance->getPendingFace(name);
}

vector<pair<string, unsigned short>> DataProc::getMsgPendingFaceInPIT(string name){
    return pitInstance->getMsgPendingFace(name);
}

bool DataProc::IsDataPackageInContentStore(DataPackage datapack){
    return cslruInstance->IsDataPackageInContentStore(datapack);
}

bool DataProc::judgeFile(string name){
    return name.find("file") != name.npos;
}

bool DataProc::judgeVideo(string name){
    return name.find("video") != name.npos;
}

bool DataProc::judgeMsg(string name){
    return name.find("msg") != name.npos;
}

string DataProc::getUpperName(string name){
    string upperName = "";
    size_t position = name.find("segment");
    if(position == string::npos){
        return upperName;
    }
    return name.substr(0, position - 1);
}

bool DataProc::judgeLastLayer(){
    return (fibInstance->getLayer() == 2);
}

bool DataProc::judegMulCastIP(string IP){
    string IP1 = "224.0.0.0";
    string IP2 = "239.255.255.255";
    return ((IP >= IP1) && (IP <= IP2));
}

void DataProc::printInfo(DataPackage datapack){
    cout << "===========DataPackage===========" << endl;
    cout << "[contentName] : " << datapack.contentName << endl;
    cout << "[data]" << datapack.data << endl;
    cout << "[datasize]" << datapack.datasize << endl;
    cout << "[segmentNum]" << datapack.segmentNum << endl;
    cout << "[end]" << datapack.end << endl;
    cout << "=================================" << endl;
}