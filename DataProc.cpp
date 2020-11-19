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
    cout << "bind success" << endl;
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
     * 我当前的逻辑是，发送的兴趣包应该是统一的格式，都是 pku/eecs/ICN_EGS_1/ICN_GEO_1/file/testFile.txt/segment1 这类形式， 也就是说都有segment部分
     * 先完成规范格式的开发，对于pku/eecs/ICN_EGS_1/ICN_GEO_1/file/testFile.txt这样的格式交由上层处理，先不用管，都是分成最细的包的形式
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
        
        vector<string> contentNameVec = getContentStoreNameList(name);
        cout << contentNameVec.size()<<endl;

        // Content Store中没有这个包才可以进行下面的操作
        if(contentNameVec.size() == 0){

            cout << "[Info]: No Name in Content Store!" << endl;
            vector<pair<string, unsigned short>> pendingFaceVec = getPendingFaceInPIT(name);
            if(pendingFaceVec.size() > 0){
                for(int i = 0; i < pendingFaceVec.size(); i++){
                    udpDataSocket.sendbuf(recvDataBuf, sizeof(recvDataBuf), pendingFaceVec[i].first, DataPort);
                    cout << "[Info] Forwarding data to: dstip " << pendingFaceVec[i].first << " ContentName: " << name << endl;
                }
            }
        }
        // 在content Store中找到，说明之前本地缓存有，已经还给了源端，直接丢弃即可
        else{
            cout << "Exist in Content Store" << endl;   
        }
    }
    udpDataSocket.Close();
}

vector<string> DataProc::getContentStoreNameList(string name){
    return cslruInstance->getAllRelatedContentPackage(name);
}

void DataProc::insertDataInContentStore(string name, char* data, int length){
    return cslruInstance->putContentNameAndDataAndLength(name, data, length);
}

vector<pair<string, unsigned short>> DataProc::getPendingFaceInPIT(string name){
    return pitInstance->getPendingFace(name);
}