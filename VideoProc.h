#ifndef __VIDEOPROC_H__
#define __VIDEOPROC_H__
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
 * VideoProc 用于处理视频流
 * 视频流和文件不一样,主要区别为:
 * 1. 本地不进行缓存, 收到直接进行转发
 * 2. 当本地找不到视频流的时候，当PIT中有选项的时候则直接插入，当没有的时候则直接向上级转，而不是转给同级的
 *    (这是因为如果轮询同级的如果有那么最终视频流还是会从源端转发，那么从同级再转过来就浪费资源没有意义)
 * 3. 当用户订阅的时候，则插入这条消息, 如果是第一个消息则还需要向上级转发; 当用户取消订阅的时候只需要在本地删除即可
 *    这是因为发布订阅范式中发布和订阅是空间分离的，发端不需要知道有没有收端接收．ICN接收到发布的数据，如果没有收端则直接丢弃就行
 */
class VideoProc{
private:

    // PIT 表
    PIT* pitInstance;

    // FIB 表
    FIB* fibInstance;

    //接收Interest包的socket
    UDPSocket udpVideoSocket;

    //绑定接收VideoSocket的接收端口
    unsigned short VideoPort;

    void InitVideoProc();

    vector<pair<string, unsigned short>> getVideoPendingFaceInPIT(string name);

    /**
     * judge if the icn is the last layer;
     * only last layer can transmit multicast pacakges
     */
    bool judgeLastLayer();

    bool judegMulCastIP(string IP);

public:
    VideoProc();
    ~VideoProc();

    void procVideoPackage();
};
#endif