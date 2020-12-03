#ifndef __CSLRU_H__
#define __CSLRU_H__

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <vector>
#include <string.h>
#include <mutex>
#include <algorithm>
#include <iostream>

#include "package.h"
using namespace std;
/**
 * 对CS表的设计如下：
 * 按照文件名作为lru的最小粒度进行缓存．比如一个pku/eecs/file/test1.txt 此文件，当其要被替换掉的时候，它的下面的所有的segment都要被替换掉．
 * pku/eecs/file/test1.txt下面有　segment1, segment2到segment10那么这10个包作为一个数组都存起来
 */

//按照DataPackage里面的
struct package_hash_cs{
    inline size_t operator()(const DataPackage &datapack) const{
        string tmp = datapack.contentName;
        return std::hash<string>()(tmp);
        //return (size_t)(datapack.segmentNum + datapack.end);
    }
};

class CSLRU{

private:

    /**
     * 标识Content Store中最多能有多少个包
     */
    int size; 
    
    /**
     * 存放的是lru的访问顺序,最近访问的放在list的头，如果删除则删除末尾的，以内容来区分
     * lru的单位是文件名，不是包．比如　pku/eecs/file/test1.txt
     */
    list<string> lru;

    //根据名字ContentName找到对应的char*迭代器
    map<string, list<string>::iterator> Name2Itermap;
    
    /**根据名字ContentName找到Content具体内容
     * 返回值应该是一组vector<DataPackage>表示一组数据包segment1, segment2等等 
     */
    unordered_map<string, unordered_set< DataPackage, package_hash_cs>> Name2ContentData;
    /**
     * 这个应该用不到了，因为DataPackage里面包含了每一个segment的信息
     * 根据名字ContentName找到对应的长度，和Name2Content结合使用能够得到具体长度的字符数组
     */
    unordered_map<string, int> Name2ContentLength;

    CSLRU(int _size = 100);
    static CSLRU* cslruInstance;

    /**
     * 从DataPackage中获得上级目录的名称，也就是文件名称，比如
     * pku/eecs/file/test1.txt/segment1 -> pku/eecs/file/test1.txt
     * 从而确定是哪个文件，这个文件是不是在lru中
     */
    string getUpperName(string name);

    /**
     * 根据一个UpperName获得所有这个UpperName下面的包
     * 比如说　pku/eecs/file/test1.txt 下面所有的segment包比如segment1 segment2等
     */
    vector<DataPackage> getAllDataPackageByUpperName(string upperName);

    /**
     * 根据一个segment粒度的InterestPackage中的name获得对应的Data Package
     * 比如说根据　pku/eecs/file/test1.txt/segment1获得这个数据包 
     * 理论上这个数据包只有１个，但是对于上层来说看到的还是vector只不过是只有一个元素的vector
     */
    vector<DataPackage> getDataPackageBySegmentName(string name);

public:

    
    ~CSLRU();

    CSLRU(CSLRU &other) = delete;
    void operator=(const CSLRU &) = delete;

    static CSLRU *GetInstance(int _size = 100);

    /*
    * 将Data包存放入Content Store表中
    * 注意根据规则，Data包中ContentName必须含有segment,比如pku/eecs/file/test1.txt/segment1 那么取/segment1前面的字符串就可以判断其是不是在lru中出现过了
    * 并且DataPackage里面包括序列号和end方便进行排序
    */
    void putContentNameAndDataAndLength(DataPackage datapack);

    /**
     * 根据UpperName删除这个UpperName对应的所有的 Data Package
     */
    void deleteAllDataPackageByUpperName(string name);

    /*
    * 根据一个ContentName找到所有相关的contentNames
    * 比如说一个请求可能请求的是pku/eecs/video/testfile.txt
    * 根据现有的缓存查找出所有合适的缓存，比如说pku/eecs/video/testfile.txt/s1 pku/eecs/video/testfile.txt/s2等等。
    * 前缀树不是很合适，因为本身前缀树对于删除不是很友好，并且对于插入和每一级的查找都有开销。
    * 所以目前的想法是在Name2ContentData和Name2ContentLength做线性查找，并且进行优化
    * 
    */
    vector<DataPackage> getAllRelatedContentPackage(string name);

    
    /**
     * 接收到的DataPackage需要首先判断是不是在ContentStore中
     * 这个的判断粒度是segment级别，因为收到的所有DataPackage都是这个级别的
    */
    bool IsDataPackageInContentStore(DataPackage datapack);
    
    void printCSLRU();

    /*
    * 将数据内容ContentData 和 长度length存入CS表
    * 注意上面那个函数是有锁的，这个是无锁的，这个函数在真正开发完成之后会被删除．
    * 这个函数也是向CS中添加东西，是为了方便测试用的．
    */
    void putContentNameAndDataAndLengthNoLock(DataPackage datapack);

};
#endif