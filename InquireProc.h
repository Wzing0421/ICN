#ifndef _INQUIREPROC_H__
#define _INQUIREPROC_H__

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
 * InquireProc用于处理同级别ICN节点的查询
*/
class InquireProc{
private:

    // Content Store 表
    CSLRU* cslruInstance;

    UDPSocket udpInquireSocket;

    //向本级别ICN节点查询的目的端口
    unsigned short InquirePort;

    //向本级别ICN节点查询的源端口
    unsigned short InquireBindPort;

    void InitInquireProc();

    bool isContentStoreDataExist(string name);

public:

    InquireProc();
    ~InquireProc();

    void procInquire();
    
};
#endif