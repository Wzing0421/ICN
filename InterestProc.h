#ifndef __INTERESTPROC_H__
#define __INTERESTPROC_H__
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <string.h>
#include <iostream>
#include <mutex>
#include <fstream>

#include "json/json.h"
#include "CS_LRU.h"
#include "FIB.h"
#include "PIT.h"
#include "UDPSocket.h"
#include "package.h"

using namespace std;


/**
 * 用于处理Interest包的类
*/
class InterestProc{
private:

    // Content Store 表
    CSLRU* cslruInstance;

    // FIB 表
    FIB* fibInstance;

    // PIT 表
    PIT* pitInstance;

    //接收Interest包的socket
    UDPSocket udpInterestSocket;

    //绑定接收InterestSocket的接收端口
    unsigned short InterestPort;

    unsigned short DataPort;

    void InitInterestProc();

    vector<string> getContentStoreNameList(string name);

    char* getContentStoreData(string name);

    int getContentStoreDataLength(string name);

    bool isNameExistInPIT(string name);

    void insertIpAndPortByContentName(string name, string IP, unsigned short port);

    /*查询这个Interest请求的name是不是归属于本地*/
    bool isMatchLocalNames(string name);

    /*这个请求不属于本地的情况下，获得这个name应该转发的所有接口*/
    vector<pair<string, unsigned short>> getForwardingFaces(string name);

public:

    InterestProc();
    ~InterestProc();

    /**
     * 用于处理收到Interst包的函数
     * 详细流程见《信息中心网络》清华大学著 108页
     */
    void procInterestPackage();
    

};
#endif