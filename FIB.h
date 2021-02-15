#ifndef __FIB_H__
#define __FIB_H__

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
#include "arpa/inet.h"

using namespace std;

struct pair_hash_fib{
    inline size_t operator()(const pair<string, unsigned short> &p) const{
        unsigned int x = ntohl(inet_addr(p.first.c_str()));
        return (size_t) (x + (unsigned int) p.second);
    }
};

/**
 * FIB是兴趣包转发信息表
 * 对于一个Interest兴趣包的内容信息，根据FIB表将Interest转发至相应的其他ICN节点上
 * 理论上FIB表应该是ICN 节点启动的时候就固定的，但是还是应该留下来能调用的添加和删除的接口，以保证接口的拓展性
 * FIB 表启动的时候，需要使用jsoncpp读取配置文件读取.
 * 请注意，FIB是不需要加锁的，因为本身FIB的网络拓扑应该是已经确定好的，并且所有的操作只涉及到读，不涉及到写。
 */
class FIB{

private:

    /**
     * 前向信息表。
     * 对于一个包含ContentName的Interest兴趣包, 需要定义的是如果本地没找到这个ContentName应该将其转发到那个目的地址上面
     * 这个目的地址不一定是一个IP+port而有可能是一组，所以用unordered_set来定义
     */
    unordered_map<string, unordered_set< pair<string, unsigned short>, pair_hash_fib > > ContentNameForwardMap;

    /**
     * 代表本地所有匹配的内容信息
     * 比如 pku/eecs/ICN_EGS_1/ICN_GEO_1 那么除了这个ContentName之外，它的下级
     * pku/eecs/ICN_EGS_1/ICN_GEO_1/video pku/eecs/ICN_EGS_1/ICN_GEO_1/file pku/eecs/ICN_EGS_1/ICN_GEO_1/msg 也都能作为本节点的目录
     * 只要有任何一个命中，那么都应该认为是本地节点的内容
     */
    unordered_set<string> localNames;

    /**
     * 标识此ICN节点的名称
     */
    string LocalName;

    /**
     * 标识ICN节点在第几层
     */
    int layer;
    
    /**
     * 第二层GEO的总数
     */
    int layer2sz;
    /**
     * 如果在第二层GEO，涉及到环状所以需要需要知道在环里面的位置 
     */
    int position;

    /**
     * FIB转发表的分级结构
     */
    vector<vector<pair<string, string>>> FIBstructure;

    /**
     * 记录当这个ICN节点发现其没有内容之后需要向本级查询的节点列表
     * 在FIB初始化的时候就应该将其填充，避免多次查询影响效率
     */
    vector<string> ForwardingIPlist;

    /**
     * 只支持layer == 1 和　layer == 2
     */
    void formForwardingIPlist(int layer, Json::Value &root);

    FIB();
    static FIB* fibInstance;

public:
    
    ~FIB();
    FIB(FIB &other) = delete;
    void operator=(const FIB &) = delete;

    static FIB* GetInstance();

    /**
     * 初始化FIB表和本地内容表
     * 需要靠读取json文件的形式来初始化，每个节点的json文件是不一样的
     * 分别初始化ContentNameForwardMap（每个内容应该转发的IP+port）和localNames（本地内容表）
     */
    void initFIB();

    /**
     * 判断一个ContentName是不是本地的表里面有
     * 注意：1. 这里的输入应该是getUpperContent调整过的
     *      2. 调整过的输入应该遵循 最长匹配原则，比如
     * 本地是： pku/eecs/ICN_EGS_1/ICN_GEO_1; 收到pku/eecs/ICN_EGS_1/video/testfile.txt 是不能匹配的，因为video上一级是pku/eecs/ICN_EGS_1是不能匹配的
    */
    bool isMatchLocalNames(string name);

    /**
     * 根据最长匹配原则，获得上一级（或者可以理解成本地的目录） 从而判断请求是不是本地的内容
     * pku/eecs/ICN_EGS_1/ICN_GEO_1/video/testfile.txt/segment1 应该转换成pku/eecs/ICN_EGS_1/ICN_GEO_1
     * 先使用本函数调整输入ContentName, 然后根据ContentName来使用isMatchLocalNames函数判断能不能匹配到本地的内容目录 
    */
    string getUpperContent(string name);

    /**
     * 如果不能匹配本地的目录， 则使用下面的函数获得应该转发至的所有接口，将Interest包转发至相应的接口
     * 如果为空，说明不知道应该将其转发至哪里，当前原则是丢弃
     * 
     * 下一步计划，如果不知道转发至哪里，则转发至上级。
     * 比如：高轨卫星转发至地面信关站ICN节点，地面ICN 节点转发至信源端，表示请求源端发送新的文件等内容
     */
    vector<string> getForwardingFaces(string name);

    /**
     *　当本级别没用内容的时候， 获得上一级的转发ICN节点的IP
     */
    string getUpperLevelForwardingIP();

    /**
     * 
        对于组播的视频流或者短消息流,　高层的节点不能向组播转发（避免重复），而应该向其下级节点转发
        所以fib表应该提供寻找下级某节点的功能
    */
    string getLowerLevelForwardingIP();

    int getLayer();

};

#endif