#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <string.h>
#include <iostream>
#include <fstream>

#include "json/json.h"
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
 * FIB 表启动的时候，需要使用jsoncpp读取配置文件读取
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
     * 获得上一级（或者可以理解成本地的目录），从而判断请求是不是本地的内容
     * pku/eecs/ICN_EGS_1/ICN_GEO_1/video/testfile.txt/segment1 应该转换成pku/eecs/ICN_EGS_1/ICN_GEO_1 
    */
    string getUpperContent(string name);


public:
    FIB();
    ~FIB();
    /**
     * 初始化FIB表和本地内容表
     * 需要靠读取json文件的形式来初始化，每个节点的json文件是不一样的
     * 分别初始化ContentNameForwardMap（每个内容应该转发的IP+port）和localNames（本地内容表）
     */
    void initFIB();

    /**
     * 判断一个ContentName是不是本地的表里面有
     * 请注意这里应该使用的是最长匹配原则，比如
     * 本地是： pku/eecs/ICN_EGS_1/ICN_GEO_1; 收到pku/eecs/ICN_EGS_1/video/testfile.txt 是不能匹配的，因为video上一级是pku/eecs/ICN_EGS_1是不能匹配的
    */
    bool isMatchLocalNames(string name);
};