#include "InterestProc.h"
#include "DataProc.h"
#include "InquireProc.h"
#include "VideoProc.h"
#include "MsgProc.h"

#include <pthread.h>
#include <iostream>

using namespace std;

void CSUnitTest(){
    /*
    string str1 = "pku/eecs/file/test1.txt/segment1";
    char *strcontent1 = "teststrlength=16";
    DataPackage package1(str1.c_str(), strcontent1, strlen(strcontent1), 1, 0);

    string str2 = "pku/eecs/file/test1.txt/segment1";
    char *strcontent2 = "testteststrlength=20";
    DataPackage package2(str2.c_str(), strcontent2, strlen(strcontent2), 1, 0);

    string str3 = "pku/eecs/file/test1.txt/segment3";
    char *strcontent3 = "testteststrlength=28";
    DataPackage package3(str3.c_str(), strcontent3, strlen(strcontent3), 1, 0);

    string str4 = "pku/eecs/file/test2.txt/segment4";
    char *strcontent4 = "testteststrlength=22";
    DataPackage package4(str4.c_str(), strcontent4, strlen(strcontent4), 1, 0);

    string str5 = "pku/eecs/file/test2.txt/segment1";
    char *strcontent5 = "testteststrlength=23";
    DataPackage package5(str5.c_str(), strcontent5, strlen(strcontent5), 1, 0);
    
    CSLRU *cslruinstance = CSLRU::GetInstance(3);
    cslruinstance->putContentNameAndDataAndLength(package1);
    cslruinstance->printCSLRU();
    cslruinstance->putContentNameAndDataAndLength(package2);
    cslruinstance->printCSLRU();
    cslruinstance->putContentNameAndDataAndLength(package3);
    cslruinstance->printCSLRU();
    cslruinstance->putContentNameAndDataAndLength(package4);
    cslruinstance->printCSLRU();
    cslruinstance->putContentNameAndDataAndLength(package5);
    cslruinstance->printCSLRU();
    
    string delstr1 = "pku/eecs/file/test10.txt";
    cslruinstance->deleteAllDataPackageByUpperName(delstr1);
    cslruinstance->printCSLRU();
    vector<DataPackage> res1 = cslruinstance->getAllRelatedContentPackage("pku/eecs/file/test1.txt");
    for(auto r : res1) cout << r.contentName << "  " << r.data << endl;
    cout << cslruinstance->IsDataPackageInContentStore(package1) << endl;
    */   
}

void PITUnitTest(){
    /*
    string name1 = "pku/eecs/video/testfile1.txt"; string IP1 = "162.105.85.184"; unsigned short port1 = 20000;
    string name2 = "pku/eecs/video/testfile2.txt"; string IP2 = "162.105.85.63"; unsigned short port2 = 20100;
    string name3 = "pku/eecs/file/testfile1.txt"; string IP3 = "162.105.85.184"; unsigned short port3 = 20040;
    string name4 = "pku/eecs/file/testfile1.txt"; string IP4 = "162.105.85.184"; unsigned short port4 = 20041;
    PIT *pitinstance = PIT::GetInstance();
    pitinstance->insertIpAndPortByContentName(name1, IP1, port1);
    pitinstance->insertIpAndPortByContentName(name2, IP2, port2);
    pitinstance->insertIpAndPortByContentName(name3, IP3, port3);
    pitinstance->insertIpAndPortByContentName(name4, IP4, port4);
    pitinstance->printPIT();
    
    string str = "pku/eecs/video/testfile1.txt/segment1";
    vector<pair<string, unsigned short>> res = pitinstance->getPendingFace(str);
    cout << res.size() << endl;
    if(res.size() > 0){
        for(int i = 0; i < res.size(); i++){
            cout << res[i].first << " " << res[i].second << " ";
        }
        cout << endl;
    }*/
}

void FIBUnitTest(){
    
    FIB *fibinstance = FIB::GetInstance();
    /*string str1 = "pku/eecs/ICN_EGS_1/ICN_GEO_1/file";
    cout << "str1 " << str1 << " "<< fibinstance->isMatchLocalNames(str1) << endl;
    string str2 = "pku/eecs/ICN_GEO_1/file/testfial.txt/segment1";
    cout << "str2 " << str2 << " "<< fibinstance->isMatchLocalNames(str2) << endl;
    string str3 = "pku/eecs/ICN_EGS_1/ICN_GEO_1/msg";
    cout << "str3 " << str3 << " "<<  fibinstance->isMatchLocalNames(str3) << endl;
    string str4 = "pku/eecs/ICN_EGS_1/ICN_GEO_1/fi";
    cout << "str4 " << str4 << " "<< fibinstance->isMatchLocalNames(str4) << endl;
    string str5 = "pku/eecs/ICN_EGS_1/ICN_GEO_1";
    cout << "str5 " << str5 << " "<< fibinstance->isMatchLocalNames(str5) << endl;
    string str6 = "pku/eecs/ICN_EGS_1";
    cout << "str6 " << str6 << " "<< fibinstance->isMatchLocalNames(str6) << endl;
    string str7 = "pku/eecs/ICN_EGS_2";
    cout << "str7 " << str7 << " "<< fibinstance->isMatchLocalNames(str7) << endl;
    string str8 = "pku/eecs";
    cout << "str8 " << str8 << " "<< fibinstance->isMatchLocalNames(str8) << endl;
    string str9 = "pku/eecs/ICN_EGS_2/file/tes1.txt/segment1";
    //先获得上级根目录
    string upperContentName = fibinstance->getUpperContent(str9);
    //然后判断是不是符合本地目录的要求
    cout << fibinstance->isMatchLocalNames(upperContentName) << endl;
    //不符合则可以获得转发表
    vector<pair<string, unsigned short>> ret = fibinstance->getForwardingFaces(upperContentName);
    for(int i = 0; i < ret.size(); i++){
        cout << ret[i].first << " " << ret[i].second<< " ";
    }
    cout << endl;
    */
}

void *processingInterestPackage(void*){
    InterestProc interestProc;
    interestProc.procInterestPackage();
}

void *processingDataPackage(void*){
    DataProc dataProc;
    dataProc.procDataPackage();
}

void *processingInquirePackage(void*){
    InquireProc inquireProc;
    inquireProc.procInquire();
}

void *processingVideoPackage(void*){
    VideoProc videoProc;
    videoProc.procVideoPackage();
}

void *processingMsgPackage(void*){
    MsgProc msgProc;
    msgProc.procMsgPackage();
}

int main(){
    //CSUnitTest();
    //PITUnitTest();
    //FIBUnitTest();
    
    pthread_t thid_Interest, thid_Data, thid_Inquire, thid_Video, thid_Msg;
    if(pthread_create(&thid_Interest, NULL, processingInterestPackage, NULL) != 0){
        cout << "processingInterestPackage thread create error" << endl;
        return -1;
    }
    if(pthread_create(&thid_Data, NULL, processingDataPackage, NULL) != 0){
        cout << "processingDataPackage thread create error" << endl;
        return -1;
    }
    if(pthread_create(&thid_Inquire, NULL, processingInquirePackage, NULL) != 0){
        cout << "processingInquirePackage thread create error" << endl;
        return -1;
    }
    if(pthread_create(&thid_Video, NULL, processingVideoPackage, NULL) != 0){
        cout << "processingVideoPackage thread create error" << endl;
        return -1;
    }
    if(pthread_create(&thid_Msg, NULL, processingMsgPackage, NULL) != 0){
        cout << "processingMsgPackage thread create error" << endl;
        return -1;
    }
    pthread_join(thid_Interest, NULL);
    pthread_join(thid_Data, NULL);
    pthread_join(thid_Inquire, NULL);
    pthread_join(thid_Video, NULL);
    pthread_join(thid_Msg, NULL);
    return 0;
}