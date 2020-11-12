#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <string.h>
#include <iostream>

using namespace std;

class PIT{

private:
    /**
     * 存放一个ContentName和一组IP port之间的关系
     * ContentName表示当前请求的名字，比如pku/eecs/video/textfile
     * IP port标识请求来源的端口
     * 这样当data到来的时候根据data就可以找到源端口进行转发
     */
    unordered_map<string, unordered_set<pair<string, unsigned short>>> ContentName2IPPort;

public:
    PIT();
    ~PIT();

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
};