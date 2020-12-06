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
#include <jsoncpp/json/json.h>

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

    /**
     * 询问本级ICN节点是否有请求内容的Socket
     * 这个inquireSocket是InterestProc下的私有成员，和InquireProc.cpp下的Socket不是同一个
     */
    UDPSocket inquireSocket;

    //绑定接收InterestSocket的接收端口
    unsigned short InterestPort;

    unsigned short DataPort;

    //向本级别ICN节点查询的目的端口
    unsigned short InquirePort;

    //向本级别ICN节点查询的源端口
    unsigned short InquireBindPort;

    /**
     * 设置本级查询超时时间
     */
    int tv_sec;
    int tv_usec;

    void InitInterestProc();

    vector<DataPackage> getContentStoreDataList(string name);

    bool isNameExistInPIT(string name);

    void insertIpAndPortByContentName(string name, string IP, unsigned short port);

    /**
     * 查询这个Interest请求的name是不是归属于本地
     */
    bool isMatchLocalNames(string name);

    /**
     * 这个请求不属于本地的情况下，获得这个name应该转发的所有接口
     */
    vector<string> getForwardingFaces(string name);

    /**
     * 当发现本级别的ICN节点存在想要的内容，则返回这个ICN节点的IP
     */
    string getThisLevelIPIfContentExist(vector<string> &forwardingFaceList, InterestPackage &interestpack);

    /**
     * 获得应该转发到上一级的ICN节点的IP
    */
    string getUpperLevelIP();

    /**
     * 打印调试信息
    */
    void printInfo(InterestPackage interestpack);

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