#include "CS_LRU.h"
#include <iostream>

using namespace std;
// 设计三个数据结构 CS PIT 和 FIB
//json 读取

void UnitTest(){
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

    //这里有问题
    //第一，需要确定有没有这个请求，比如说/aa/aa/textfile.txt/segment3 这个就是没有的内容，但是还是显示出来了
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
    cout << endl;
}

int main(){
    UnitTest();
    return 0;
}