#include <map>
#include <unordered_map>
#include <list>
#include <vector>
#include <string.h>
#include <mutex>
#include <iostream>

using namespace std;
/**
 * CS表
 */

class CSLRU{

private:

    /**
     * 标识Content Store中最多能有多少个包
     */
    int size; 
    
    //存放的是lru的访问顺序,最近访问的放在list的头，如果删除则删除末尾的，以内容来区分
    list<string> lru;

    //根据名字ContentName找到对应的char*迭代器
    map<string, list<string>::iterator> Name2Itermap;
    
    //根据名字ContentName找到Content具体内容
    unordered_map<string, char*> Name2ContentData;

    //根据名字ContentName找到对应的长度，和Name2Content结合使用能够得到具体长度的字符数组
    unordered_map<string, int> Name2ContentLength;

    CSLRU(int _size = 1000);
    static CSLRU* cslruInstance;

public:

    
    ~CSLRU();

    CSLRU(CSLRU &other) = delete;
    void operator=(const CSLRU &) = delete;

    static CSLRU *getInstance(int _size = 1000);

    /**
     * 根据ContentName获得数据ContentData
     * 我在想这里面应该是返回一个list之类的，而不应该仅仅是一个char*
     * 比如说/video/testfile.txt这个文件，可能先转换成一个list:
     * /video/testfile.txt/s1 ; /video/testfile.txt/s2 等等，然后在转换成所有的char*
     * 也就是我们应该有一个函数来存放从一个文件对应所有文件包的函数 
     */
    /*
     *我目前想法getContentData只负责单条数据的处理，比如说/video/testfile.txt/s2 这种
      然后/video/testfile.txt 拆分成多个本地能找到的缓存交由上层逻辑负责
     * 
    */
    char* getContentData(string name);

    /**
     * 根据ContentName获得长度ContentLength
     */
    int getContentLength(string name);

    /*
    * 将数据内容ContentData 和 长度length存入CS表
    */
    void putContentNameAndDataAndLength(string name, char* data, int length);

    /**
     * 根据ContentName 删除 ContentData
     */
    void deleteContentDataAndLength(string name);

    /*
    * 根据一个ContentName找到所有相关的contentNames
    * 比如说一个请求可能请求的是pku/eecs/video/testfile.txt
    * 根据现有的缓存查找出所有合适的缓存，比如说pku/eecs/video/testfile.txt/s1 pku/eecs/video/testfile.txt/s2等等。
    * 前缀树不是很合适，因为本身前缀树对于删除不是很友好，并且对于插入和每一级的查找都有开销。
    * 所以目前的想法是在Name2ContentData和Name2ContentLength做线性查找，并且进行优化
    * 
    */
    vector<string> getAllRelatedContentPackage(string name);

    //把输入内容按照string中的"/"分割开
    void SplitString(string &s, vector<string> &v);
    
    void printCSLRU();

};