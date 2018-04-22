/***************************************
* @file     net_base.h
* @brief    网络库基本功能函数
* @details  大小端判断
            32&64位系统判断
            ntohll与htonl的实现
            int32与int64序列/反序列化为char[4],char[8]
            数据包头结构定义
            在win平台测试过，linux平台未测试
* @author   phata, wqvbjhc@gmail.com
* @date     2014-5-16
* @mod      2014-5-21 phata 包定义添加了包头包尾版本和校验位信息
****************************************/
#ifndef NET_BASE_H
#define NET_BASE_H
#include <stdint.h>
#if defined (WIN32) || defined (_WIN32)
#include <WinSock2.h>
#include <stdlib.h>
#ifdef _MSC_VER
#pragma comment(lib,"Ws2_32.lib")
#endif //_MSC_VER
#else
#include <sys/types.h>
#include <netinet/in.h>
#endif

//判断是否小端字节序，是返回true
inline bool IsLittleendian()
{
    int i = 0x1;
    return *( char * )&i == 0x1;
}

//把32位的int保存在char[4]中.先转为网络字节序，然后int的最高位保存为char[0],最低位保存于char[3]
inline bool Int32ToChar( const uint32_t intnum, unsigned char *charnum )
{
    //unsigned long network_byteorder = htonl( intnum ); //转换为网络字节序
    unsigned long network_byteorder = intnum;
    charnum[0] = ( unsigned char )( ( network_byteorder & 0xff000000 ) >> 24 ); //int的最高位
    charnum[1] = ( unsigned char )( ( network_byteorder & 0x00ff0000 ) >> 16 ); //int的次高位
    charnum[2] = ( unsigned char )( ( network_byteorder & 0x0000ff00 ) >> 8 ); //int的次低位;
    charnum[3] = ( unsigned char )( ( network_byteorder & 0x000000ff ) ); //int的最低位;
    return true;
}

//把char[4]转换为32位的int。int的最高位保存为char[0],最低位保存于char[3]，然后转为主机字节序
inline bool CharToInt32( const unsigned char *charnum, uint32_t &intnum )
{
    intnum = ( charnum[0] << 24 ) + ( charnum[1] << 16 ) + ( charnum[2] << 8 ) + charnum[3];
    //intnum = ntohl( intnum );
    return true;
}

//把64位的int保存在char[8]中.先转为网络字节序，然后int的最高位保存为char[0],最低位保存于char[7]
inline bool Int64ToChar( const uint64_t intnum, unsigned char *charnum )
{
    //uint64_t network_byteorder = htonll( intnum ); //转换为网络字节序
    uint64_t network_byteorder = intnum;
    charnum[0] = ( unsigned char )( ( network_byteorder & 0xff00000000000000ULL ) >>
                                    56 ); //int的最高位
    charnum[1] = ( unsigned char )( ( network_byteorder & 0x00ff000000000000ULL ) >>
                                    48 ); //int的次高位
    charnum[2] = ( unsigned char )( ( network_byteorder & 0x0000ff0000000000ULL ) >> 40 );
    charnum[3] = ( unsigned char )( ( network_byteorder & 0x000000ff00000000ULL ) >> 32 );
    charnum[4] = ( unsigned char )( ( network_byteorder & 0x00000000ff000000ULL ) >> 24 );
    charnum[5] = ( unsigned char )( ( network_byteorder & 0x0000000000ff0000ULL ) >> 16 );
    charnum[6] = ( unsigned char )( ( network_byteorder & 0x000000000000ff00ULL ) >>
                                    8 ); //int的次低位;
    charnum[7] = ( unsigned char )( ( network_byteorder & 0x00000000000000ffULL ) ); //int的最低位;
    return true;
}

//把char[8]转换为64位的int。int的最高位保存为char[0],最低位保存于char[7]，然后转为主机字节序
inline bool CharToInt64( const unsigned char *charnum, uint64_t &intnum )
{
    intnum = ( ( uint64_t )charnum[0] << 56 ) + ( ( uint64_t )charnum[1] << 48 ) + ( (
                 uint64_t )charnum[2] << 40 ) + ( ( uint64_t )charnum[3] << 32 ) +
             ( charnum[4] << 24 ) + ( charnum[5] << 16 ) + ( charnum[6] << 8 ) + charnum[7];
    //intnum = ntohll( intnum ); //转换为网络字节序
    return true;
}

#pragma pack(1)//将当前字节对齐值设为1

#define NET_PACKAGE_VERSION 0x01
typedef struct _NetPacket { //传输自定义数据包头结构
    uint8_t version;        //packet version :0
    unsigned char header;   //packet header，eg. 0x02 :1
    unsigned char tail;     //packet tail，eg. 0x03 :2
    unsigned char check[16]; //Authentication hash :3-18
    uint8_t type;           //packet type :19
    uint32_t uid;           //user id :20-23
    uint32_t datalen;       //packet data length :24-27
} NetPacket;
#define NET_PACKAGE_HEADLEN sizeof(NetPacket)

#pragma pack()//将当前字节对齐值设为默认值(通常是4)

inline bool NetPacketToChar( const NetPacket &package, unsigned char *chardata )
{
    chardata[0] = package.version;
    chardata[1] = package.header;
    chardata[2] = package.tail;
    memcpy( chardata + 3, package.check, sizeof( package.check ) );
    chardata[19] = package.type;
    if( !Int32ToChar( ( uint32_t )package.uid, chardata + 20 ) ) {
        return false;
    }
    if( !Int32ToChar( ( uint32_t )package.datalen, chardata + 24 ) ) {
        return false;
    }
    return true;
}

inline bool CharToNetPacket( const unsigned char *chardata, NetPacket &package )
{
    uint32_t tmp32;
    package.version = chardata[0];
    package.header = chardata[1];
    package.tail = chardata[2];
    memcpy( package.check, chardata + 3, sizeof( package.check ) );
    package.type = chardata[19];
    if( !CharToInt32( chardata + 20, tmp32 ) ) {
        return false;
    }
    package.uid = tmp32;
    if( !CharToInt32( chardata + 24, tmp32 ) ) {
        return false;
    }
    package.datalen = tmp32;
    return true;
}
#endif//NET_BASE_H

////测试用例
//#include <stdio.h>
//#include "common/net/net_base.h"
//int main(int argc, char **argv)
//{
//  printf("islittleendian %d\n",IsLittleendian());
//  printf("IsSystem32 %d\n",IsSystem32());
//  uint32_t intnum = 0x1234567A;
//  unsigned char charnum[4];
//  Int32ToChar(intnum,&charnum[0]);
//  printf("Int32ToChar-int=0x%x, %d, char=%x,%x,%x,%x\n",intnum,intnum,charnum[0],charnum[1],charnum[2],charnum[3]);
//  CharToInt32(&charnum[0],intnum);
//  printf("CharToInt32-int=0x%x, %d, char=%x,%x,%x,%x\n",intnum,intnum,charnum[0],charnum[1],charnum[2],charnum[3]);
//
//  uint64_t int64num = 0x123456789ABCDEF0;
//  unsigned char char8num[8];
//  Int64ToChar(int64num,char8num);
//  printf("Int64ToChar-int=0x%I64x, %I64d, char=%x,%x,%x,%x,%x,%x,%x,%x\n",int64num,int64num,char8num[0],char8num[1],char8num[2],
//      char8num[3],char8num[4],char8num[5],char8num[6],char8num[7]);
//  CharToInt64(char8num,int64num);
//  printf("CharToInt64-int=0x%I64x, %I64d, char=%x,%x,%x,%x,%x,%x,%x,%x\n",int64num,int64num,char8num[0],char8num[1],char8num[2],
//      char8num[3],char8num[4],char8num[5],char8num[6],char8num[7]);
//
//  printf("sizeof NetPackage=%d\n",sizeof(NetPacket));
//  unsigned char packagechar[NET_PACKAGE_HEADLEN];
//  NetPacket package;
//  package.type = intnum;
//  package.reserve = intnum + 1;
//  package.datalen = int64num;
//  memset(packagechar,0,NET_PACKAGE_HEADLEN);
//  NetPacketToChar(package,packagechar);
//  printf("NetPackageToChar -- package data (type=%d,reserve=%d,datalen=%d), char=",package.type,package.reserve,package.datalen);
//  for (int i=0; i<NET_PACKAGE_HEADLEN; ++i) {
//      printf("%x,",packagechar[i]);
//  }
//  printf("\n");
//  memset(&package,0,NET_PACKAGE_HEADLEN);
//  CharToNetPacket(packagechar,package);
//  printf("CharToNetPackage -- package data (type=%d,reserve=%d,datalen=%d), char=",package.type,package.reserve,package.datalen);
//  for (int i=0; i<NET_PACKAGE_HEADLEN; ++i) {
//      printf("%x,",packagechar[i]);
//  }
//  printf("\n");
//  memset(packagechar,0,NET_PACKAGE_HEADLEN);
//  NetPacketToChar(package,packagechar);
//  printf("NetPackageToChar -- package data (type=%d,reserve=%d,datalen=%d), char=",package.type,package.reserve,package.datalen);
//  for (int i=0; i<NET_PACKAGE_HEADLEN; ++i) {
//      printf("%x,",packagechar[i]);
//  }
//  printf("\n");
//  return 0;
//}
