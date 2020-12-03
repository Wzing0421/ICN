#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include <string.h>
#include <iostream>
using namespace std;

/**
 * 添加对Package的定义
 * Package包括两种: Interest和Data
 * Interest表示请求，Data表示返回的数据
 */

/**
 * InterestPackage目前只有这一个参数，表示请求的内容
 */
struct InterestPackage{
    
    /**
     * 请求内容名字
     */
    char contentName[50];
    
    InterestPackage(const char* _contentName){
        strcpy(contentName, _contentName);
    }
    InterestPackage(){}
};

struct DataPackage{
    /**
     * 内容名字
     */
    char contentName[50];
    
    /**
     * 具体数据,我只是初步定于1400字节，因为考虑到IP的MTU是1500字节
     */
    char data[1400];
    
    /**
     * 数据长度
     */
    int datasize;

    /**
     * 包序号，排序用的
     */
    int segmentNum;

    /**
     * 是否为最后一个包
     * 不是：　end = 0
     * 是：　end = 1
     */
    int end;

    DataPackage(const char* _contentName, const char* _data, int _size, int _segmentNum, int _end){
        strcpy(contentName, _contentName);
        strcpy(data, _data);
        datasize = _size;
        segmentNum = _segmentNum;
        end = _end;
    }
    DataPackage(){}
    bool operator==(const DataPackage rhs) const{
        //return (this->segmentNum == rhs.segmentNum && this->datasize == rhs.datasize && strlen(this->contentName) == strlen(rhs.contentName) && strncmp(this->contentName, rhs.contentName, sizeof(this->contentName)));
        return (strlen(this->contentName) == strlen(rhs.contentName) && strncmp(this->contentName, rhs.contentName, sizeof(this->contentName)) == 0);
    }

    //当Interest到来本地发现存在内容的时候，需要排序之后按照顺序发送回Interest请求源端
    bool operator < (const DataPackage rhs) const{
        string contentstr1 = this->contentName;
        string contentstr2 = rhs.contentName;
        return contentstr1 < contentstr2;
    }
};

/**
 * 当一个ICN Node收到了Interest之后首先需要将其交给同级别的其他ICN节点．
 * 显然它不能仍旧占用InterestPackage的处理流程，所以交给一个单独的线程来处理．
 * 这个线程接收到InquirePackage就来查找有无缓存，有的话，通知源端发送Interest包即可．
 * 但是有一个问题，就是对于InterestProc来说，每次接收到Interest之后需要向同级询问，这个应该是异步的，这个实现起来比较繁琐．
 * 所以如果可以的话，应该想一下如何只用Interest来解决这个问题，此为上策．
 * 把同级查询作为InterestProc中的一个中间过程，同步进行，只不过这样性能会有影响．
 */
struct InquirePackage{
    /**
     * 请求内容名字
     */
    char contentName[50];

    /**
     * inquire = 1表示查询
     * inquire = 0表示回复
     */
    int inquire;

    /**
     * answer = 1表示内容存在
     * answer = 0表示内容不存在
     */
    int answer;
    
    InquirePackage(const char* _contentName, int _inquire, int _answer){
        strcpy(contentName, _contentName);
        inquire = _inquire;
        answer = _answer;
    }
    InquirePackage(){}
};

#endif