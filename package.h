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
    char contentName[60];
    
    InterestPackage(const char* _contentName){
        strcpy(contentName, _contentName);
    }
    InterestPackage(){}
};

struct DataPackage{
    /**
     * 内容名字
     */
    char contentName[60];
    
    /**
     * 具体数据,我只是初步定于1400字节，因为考虑到IP的MTU是1500字节
     */
    char data[1400];
    
    /**
     * 数据长度
     */
    int datasize;

    DataPackage(const char* _contentName, const char* _data, int _size){
        strcpy(contentName, _contentName);
        strcpy(data, _data);
        datasize = _size;
    }
};

#endif