/**
 * @file net_base.h
 * @auther Ge Tong
 * @brief This file contain the definition of the structure of network packet.
 */
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

inline bool Int32ToChar( const uint32_t intnum, unsigned char *charnum )
{
    //unsigned long network_byteorder = htonl( intnum );
    unsigned long network_byteorder = intnum;
    charnum[0] = ( unsigned char )( ( network_byteorder & 0xff000000 ) >> 24 );
    charnum[1] = ( unsigned char )( ( network_byteorder & 0x00ff0000 ) >> 16 );
    charnum[2] = ( unsigned char )( ( network_byteorder & 0x0000ff00 ) >> 8 );
    charnum[3] = ( unsigned char )( ( network_byteorder & 0x000000ff ) );
    return true;
}

inline bool CharToInt32( const unsigned char *charnum, uint32_t &intnum )
{
    intnum = ( charnum[0] << 24 ) + ( charnum[1] << 16 ) + ( charnum[2] << 8 ) + charnum[3];
    //intnum = ntohl( intnum );
    return true;
}

inline bool Int64ToChar( const uint64_t intnum, unsigned char *charnum )
{
    //uint64_t network_byteorder = htonll( intnum );
    uint64_t network_byteorder = intnum;
    charnum[0] = ( unsigned char )( ( network_byteorder & 0xff00000000000000ULL ) >>
                                    56 );
    charnum[1] = ( unsigned char )( ( network_byteorder & 0x00ff000000000000ULL ) >>
                                    48 );
    charnum[2] = ( unsigned char )( ( network_byteorder & 0x0000ff0000000000ULL ) >> 40 );
    charnum[3] = ( unsigned char )( ( network_byteorder & 0x000000ff00000000ULL ) >> 32 );
    charnum[4] = ( unsigned char )( ( network_byteorder & 0x00000000ff000000ULL ) >> 24 );
    charnum[5] = ( unsigned char )( ( network_byteorder & 0x0000000000ff0000ULL ) >> 16 );
    charnum[6] = ( unsigned char )( ( network_byteorder & 0x000000000000ff00ULL ) >>
                                    8 );
    charnum[7] = ( unsigned char )( ( network_byteorder & 0x00000000000000ffULL ) );
    return true;
}

inline bool CharToInt64( const unsigned char *charnum, uint64_t &intnum )
{
    intnum = ( ( uint64_t )charnum[0] << 56 ) + ( ( uint64_t )charnum[1] << 48 ) + ( (
                 uint64_t )charnum[2] << 40 ) + ( ( uint64_t )charnum[3] << 32 ) +
             ( charnum[4] << 24 ) + ( charnum[5] << 16 ) + ( charnum[6] << 8 ) + charnum[7];
    return true;
}

#pragma pack(1)

#define NET_PACKAGE_VERSION 0x01
typedef struct {
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
