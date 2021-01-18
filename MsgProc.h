#ifndef __MSGPROC_H__
#define __MSGPROC_H__
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
 * MsgProc 用于处理短消息流媒体
 * 短消息流和视频流很相似，同文件流相比他们都不需要本地缓存，直接转发就行
 */
class MsgProc{
private:

    // PIT 表
    PIT* pitInstance;

    //接收Interest包的socket
    UDPSocket udpMsgSocket;

    //绑定接收MsgSocket的接收端口
    unsigned short MsgPort;

    void InitMsgProc();

    vector<pair<string, unsigned short>> getMsgPendingFaceInPIT(string name);

    string getUpperName(string name);

public:
    MsgProc();
    ~MsgProc();

    void procMsgPackage();
};
#endif