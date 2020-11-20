#ifndef __PIT_H__
#define __PIT_H__

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <string.h>
#include <iostream>
#include <mutex>

#include "arpa/inet.h"

using namespace std;

struct pair_hash{
    inline size_t operator()(const pair<string, unsigned short> &p) const{
        unsigned int x = ntohl(inet_addr(p.first.c_str()));
        return (size_t) (x + (unsigned int) p.second);
    }
};

/** 
 * PIT是等待信息表。
 * 请注意，因为Interest对其有写操作，Data对其也有写操作，所以需要加锁。
 */
class PIT{

private:
    /**
     * 存放一个ContentName和一组IP port之间的关系
     * ContentName表示当前请求的名字，比如pku/eecs/video/textfile
     * IP port标识请求来源的端口
     * 这样当data到来的时候根据data就可以找到源端口进行转发
     */
    unordered_map<string, unordered_set< pair<string, unsigned short>, pair_hash > > ContentName2IPPort;

    //单例模式
    PIT();
    static PIT* pitInstance;

public:
    
    ~PIT();

    PIT(PIT &other) = delete;
    void operator=(const PIT &) = delete;

    static PIT* GetInstance();

    /**
     * 将ContentName对应的源端IP和port记录在map里面
     */
    void insertIpAndPortByContentName(string name, string IP, unsigned short port);


    /**
     * 根据一个ContentName获得所有的源端请求的IP + port
     * 注意请求插入的时候ContentName可能是 /pku/video/frstfile.txt
     * 而数据包到来的时候ContentName可能是 /pku/video/frstfile.txt/segment1
     * 当前想法是所有数据包不可能是 单纯的/pku/video/frstfile.txt 必须分包，也就是必须分成/pku/video/frstfile.txt/segment1
     * 这样根据最后一个/的上一级进行匹配就可以转发了
     */
    vector<pair<string, unsigned short>> getPendingFace(string name); 


    /**
     * 查看当前的内容ContentName是否已经有别人在等待了
     * 当前函数只支持带segment后缀的名称
     */
    bool isContentNamePending(string name);

    /**
     * 
     * 从当前的ContentName获取上级的ContentName
     * 比如说从 /pku/video/frstfile.txt/segment1 转换成 /pku/video/frstfile.txt
     * 因为本身第一次请求Interest来的时候可能是一整个文件，但是之后数据包过来就是具体的分包的数据了，所以需要根据上级的名字来获取源端的信息
     */
    string getUpperContentName(string name);

  
    /**
     * 根据ContentName来删除这个名字对应的转发源信息
     * 这个接口目前还没用到，比如说有转发信息表有超时时间，超时的时候可以调用这个接口 
     */
    void deleteContentName(string name);

    void printPIT();
};
#endif