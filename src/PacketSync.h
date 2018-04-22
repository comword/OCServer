#ifndef PACKET_SYNC_H
#define PACKET_SYNC_H
#include <algorithm>
#include <uv.h>
#include <string>
#include <string.h>
//#include <openssl/md5.h>
#include "net_base.h"

#ifdef _MSC_VER
#ifdef NDEBUG
#pragma comment(lib,"libeay32.lib")
#else
#pragma comment(lib,"libeay32d.lib")
#endif
#endif
typedef void ( *GetFullPacket )( const NetPacket &packethead, const unsigned char *packetdata,
                                 void *userdata );

#ifndef BUFFER_SIZE
#define BUFFER_SIZE (1024*10)
#endif

class PacketSync
{
    public:
        PacketSync();
        virtual ~PacketSync();
        bool SetHaT( char packhead, char packtail );
    public:
        void recvdata( const unsigned char *data, size_t len );
        void SetPacketCB( GetFullPacket pfun, void *userdata );
        static std::string PacketData( NetPacket &packet, const unsigned char *data );
    private:
        GetFullPacket packet_cb_;//回调函数
        void         *packetcb_userdata_;//回调函数所带的自定义数据
        enum {
            PARSE_HEAD,
            PARSE_NOTHING,
        };
        int parsetype;
        size_t getdatalen;
        uv_buf_t  thread_readdata;//负责从circulebuffer_读取数据
        uv_buf_t  thread_packetdata;//负责从circulebuffer_读取packet 中data部分
        size_t truepacketlen;//readdata有效数据长度
        size_t headpos;//找到头位置
        char *headpt;//找到头位置
        unsigned char HEAD;//包头
        unsigned char TAIL;//包尾
        NetPacket theNexPacket;
        //unsigned char md5str[MD5_DIGEST_LENGTH];
    private:// no copy
        PacketSync( const PacketSync & );
        PacketSync &operator = ( const PacketSync & );
};

//客户端或服务器关闭的回调函数
//服务器：当clientid为-1时，表现服务器的关闭事件
//客户端：clientid无效，永远为-1

//TCPServer接收到新客户端回调给用户
typedef void ( *NewConnectCB )( int clientid, void *userdata );

//TCPServer接收到客户端数据回调给用户
typedef void ( *ServerRecvCB )( int clientid, const NetPacket &packethead, const unsigned char *buf,
                                void *userdata );

//TCPClient接收到服务器数据回调给用户
typedef void ( *ClientRecvCB )( const NetPacket &packethead, const unsigned char *buf,
                                void *userdata );

//网络事件类型
typedef enum {
    NET_EVENT_TYPE_RECONNECT = 0,  //与服务器自动重连成功事件
    NET_EVENT_TYPE_DISCONNECT      //与服务器断开事件
} NET_EVENT_TYPE;
//TCPClient断线重连函数
typedef void ( *ReconnectCB )( NET_EVENT_TYPE eventtype, void *userdata );

#endif//PACKET_SYNC_H
