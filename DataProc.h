#ifndef __DATAPROC_H__
#define __DATAPROC_H__
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

class DataProc{
private:

    // Content Store 表
    CSLRU* cslruInstance;

    // FIB 表
    FIB* fibInstance;

    // PIT 表
    PIT* pitInstance;

    //接收Interest包的socket
    UDPSocket udpDataSocket;

    //绑定接收InterestSocket的接收端口
    unsigned short InterestPort;

    unsigned short DataPort;

    void InitDataProc();

    void insertDataInContentStore(DataPackage datapack);

    vector<pair<string, unsigned short>> getPendingFaceInPIT(string name);

    bool IsDataPackageInContentStore(DataPackage datapack);

    void printInfo(DataPackage datapack);

public:
    DataProc();
    ~DataProc();

    /**
     * 用于处理收到Data包的函数
     * 详细流程见《信息中心网络》清华大学著 109页
     */
    void procDataPackage();
};
#endif