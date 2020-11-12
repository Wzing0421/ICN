#pragma once
#ifndef UDPSOCKET_H
#define UDPSOCKET_H
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<time.h>
#include<string>
#include<unistd.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<iostream>
using std::string;
class UDPSocket
{
     public:
            UDPSocket();
           ~UDPSocket();
            int create(unsigned short port);//绑定端口
            int create();
            int create(string mcastip,int mcastport);
            int sendbuf(char *buf,int buflen,string destip,unsigned short destport);
            int recvbuf(char *buf,int buflen,string&srcip,unsigned short &srcport);
            int Close();
            int sock;
            int m_sock;
 
};
#endif
