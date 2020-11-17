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

    void InitInterestProc();

public:

    InterestProc();
    ~InterestProc();

    /**
     * 用于处理收到Interst包的函数
     * 详细流程见《信息中心网络》清华大学著 108页
     */
    void procInterestPackage();
};