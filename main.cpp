#include "CS_LRU.h"
#include "PIT.h" 
#include "FIB.h"

#include <iostream>

using namespace std;
// 设计三个数据结构 CS PIT 和 FIB
//json 读取

void CSUnitTest(){
    /*
    CSLRU cslru(5);
    string str1 = "/aa/aa/textfile.txt/segment1";
    char *strcontent1 = "teststrlength=16";
    
    string str2 = "/aa/aa/textfile.txt/segment2";
    char *strcontent2 = "testteststrlength=20";

    string str3 = "/aa/aa/textfile.txt/segment3";
    char *strcontent3 = "testtestteststrlaeangth=26";
    
    string str4 = "/aa/aa/textfile.txt/segment4";
    char *strcontent4 = "testtesttestteststrlength=28";
    
    string str5 = "/aa/aa/textfile2.txt/segment1";
    char *strcontent5 = "testtesttesttestteststrlength=32";

    string str6 = "/aa/aa/textfile2.txt/segment2";
    char *strcontent6 = "testtesttesttesttestteststrlength=36";

    string str7 = "/aa/aa/textfile3.txt/segment1";
    char *strcontent7 = "testtesttesttesttesttestteststrlengthh=41";
    
    cslru.putContentNameAndDataAndLength(str1, strcontent1, strlen(strcontent1));
    cslru.printCSLRU();
    cslru.putContentNameAndDataAndLength(str2, strcontent2, strlen(strcontent2));
    cslru.printCSLRU();
    cslru.putContentNameAndDataAndLength(str3, strcontent3, strlen(strcontent3));
    cslru.printCSLRU();
    cslru.putContentNameAndDataAndLength(str4, strcontent4, strlen(strcontent4));
    cslru.printCSLRU();
    cslru.putContentNameAndDataAndLength(str5, strcontent5, strlen(strcontent5));
    cslru.printCSLRU();
    cslru.putContentNameAndDataAndLength(str6, strcontent6, strlen(strcontent6));
    cslru.printCSLRU();
    cslru.putContentNameAndDataAndLength(str5, strcontent5, strlen(strcontent5));
    cslru.printCSLRU();
    cslru.putContentNameAndDataAndLength(str7, strcontent7, strlen(strcontent7));
    cslru.printCSLRU();

    cslru.deleteContentDataAndLength("/aa/aa/textfile.txt/segment4");
    cslru.printCSLRU();
    cslru.putContentNameAndDataAndLength(str7, strcontent7, strlen(strcontent7));
    cslru.printCSLRU();
    cslru.putContentNameAndDataAndLength(str1, strcontent1, strlen(strcontent1));
    cslru.printCSLRU();
    cslru.putContentNameAndDataAndLength(str2, strcontent2, strlen(strcontent2));
    cslru.printCSLRU();
    cslru.deleteContentDataAndLength("/aa/aa/textfile.txt/segment3");
    //cslru.deleteContentDataAndLength("/aa/aa/textfile.txt/segment2");
    cslru.printCSLRU();

    vector<string> ret = cslru.getAllRelatedContentPackage("/aa/aa/textfile.txt/segment3");
    for(auto r : ret) cout << r << " ";
    cout << endl;
    vector<string> ret1 = cslru.getAllRelatedContentPackage("/aa/aa/textfile.txt");
    for(auto r : ret1) cout << r << " ";
    cout << endl;
    vector<string> ret2 = cslru.getAllRelatedContentPackage("/aa/aa/textfile2.txt");
    for(auto r : ret2) cout << r << " ";
    cout << endl;
    vector<string> ret3 = cslru.getAllRelatedContentPackage("/aa/aa/textfile3.txt");
    for(auto r : ret3) cout << r << " ";
    cout << endl;*/
}

void PITUnitTest(){
    string name1 = "pku/eecs/video/testfile1.txt"; string IP1 = "162.105.85.184"; unsigned short port1 = 20000;
    string name2 = "pku/eecs/video/testfile2.txt"; string IP2 = "162.105.85.63"; unsigned short port2 = 20100;
    string name3 = "pku/eecs/file/testfile1.txt"; string IP3 = "162.105.85.184"; unsigned short port3 = 20040;
    string name4 = "pku/eecs/file/testfile1.txt"; string IP4 = "162.105.85.184"; unsigned short port4 = 20041;
    string name5 = "pku/eecs/video/testfile1.txt/segment1"; string IP5 = "162.105.85.184"; unsigned short port5 = 20200;
    string name6 = "pku/eecs/video/testfile1.txt/segment2"; string IP6 = "162.105.85.63"; unsigned short port6 = 20200;
    string name7 = "pku/eecs/video/testfile1.txt/segment2"; string IP7 = "162.105.85.184"; unsigned short port7 = 20200;
    PIT pit;
    pit.insertIpAndPortByContentName(name1, IP1, port1);
    pit.insertIpAndPortByContentName(name2, IP2, port2);
    pit.insertIpAndPortByContentName(name3, IP3, port3);
    pit.insertIpAndPortByContentName(name4, IP4, port4);
    pit.insertIpAndPortByContentName(name5, IP5, port5);
    pit.insertIpAndPortByContentName(name6, IP6, port6);
    pit.insertIpAndPortByContentName(name7, IP7, port7);
    pit.printPIT();
    
    string str = "pku/eecs/video/testfile1.txt/segment10";
    vector<pair<string, unsigned short>> res = pit.getPendingFace(str);
    cout << res.size() << endl;
    if(res.size() > 0){
        for(int i = 0; i < res.size(); i++){
            cout << res[i].first << " " << res[i].second << " ";
        }
        cout << endl;
    }    
}

void FIBUnitTest(){
    FIB fib;
    /*string str1 = "pku/eecs/ICN_EGS_1/ICN_GEO_1/file";
    cout << "str1 " << str1 << " "<< fib.isMatchLocalNames(str1) << endl;
    string str2 = "pku/eecs/ICN_GEO_1/file/testfial.txt/segment1";
    cout << "str2 " << str2 << " "<< fib.isMatchLocalNames(str2) << endl;
    string str3 = "pku/eecs/ICN_EGS_1/ICN_GEO_1/msg";
    cout << "str3 " << str3 << " "<<  fib.isMatchLocalNames(str3) << endl;
    string str4 = "pku/eecs/ICN_EGS_1/ICN_GEO_1/fi";
    cout << "str4 " << str4 << " "<< fib.isMatchLocalNames(str4) << endl;
    string str5 = "pku/eecs/ICN_EGS_1/ICN_GEO_1";
    cout << "str5 " << str5 << " "<< fib.isMatchLocalNames(str5) << endl;
    string str6 = "pku/eecs/ICN_EGS_1";
    cout << "str6 " << str6 << " "<< fib.isMatchLocalNames(str6) << endl;
    string str7 = "pku/eecs/ICN_EGS_2";
    cout << "str7 " << str7 << " "<< fib.isMatchLocalNames(str7) << endl;
    string str8 = "pku/eecs";
    cout << "str8 " << str8 << " "<< fib.isMatchLocalNames(str8) << endl;*/
    string str1 = "pku/eecs/ICN_EGS_2/file/tes1.txt/segment1";
    //先获得上级根目录
    string upperContentName = fib.getUpperContent(str1);
    //然后判断是不是符合本地目录的要求
    cout << fib.isMatchLocalNames(upperContentName) << endl;
    //不符合则可以获得转发表
    vector<pair<string, unsigned short>> ret = fib.getForwardingFaces(upperContentName);
    for(int i = 0; i < ret.size(); i++){
        cout << ret[i].first << " " << ret[i].second<< " ";
    }
    cout << endl;
}

int main(){
    //CSUnitTest();
    //PITUnitTest();
    FIBUnitTest();
    return 0;
}