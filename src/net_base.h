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
typedef struct { //传输自定义数据包头结构
    uint8_t version;        //packet version :0
    unsigned char header;   //packet header :1
    unsigned char tail;     //packet tail :2
    uint8_t type;           //packet type :3
    uint32_t uid;           //user id :4-7
    uint32_t datalen;       //packet data length :8-11
} NetPacket;
#define NET_PACKAGE_HEADLEN sizeof(NetPacket)

#pragma pack()

inline bool NetPacketToChar( const NetPacket &package, unsigned char *chardata )
{
    chardata[0] = package.version;
    chardata[1] = package.header;
    chardata[2] = package.tail;
    chardata[3] = package.type;
    if( !Int32ToChar( ( uint32_t )package.uid, chardata + 4 ) ) {
        return false;
    }
    if( !Int32ToChar( ( uint32_t )package.datalen, chardata + 8 ) ) {
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
    package.type = chardata[3];
    if( !CharToInt32( chardata + 4, tmp32 ) ) {
        return false;
    }
    package.uid = tmp32;
    if( !CharToInt32( chardata + 8, tmp32 ) ) {
        return false;
    }
    package.datalen = tmp32;
    return true;
}
#endif//NET_BASE_H
