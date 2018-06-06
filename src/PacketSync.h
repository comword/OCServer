#ifndef PACKET_SYNC_H
#define PACKET_SYNC_H
#include <algorithm>
#include <uv.h>
#include <string>
#include <string.h>
#include "net_base.h"

#ifdef _MSC_VER
#ifdef NDEBUG
#pragma comment(lib,"libeay32.lib")
#else
#pragma comment(lib,"libeay32d.lib")
#endif
#endif
typedef void ( *GetFullPacket )( const NetPacket &packethead, char *packetdata,
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
        GetFullPacket packet_cb_;
        void         *packetcb_userdata_;
        enum {
            PARSE_HEAD,
            PARSE_NOTHING,
        };
        int parsetype;
        size_t getdatalen;
        uv_buf_t  thread_readdata;
        uv_buf_t  thread_packetdata;
        size_t truepacketlen;
        size_t headpos;
        char *headpt;
        unsigned char HEAD;
        unsigned char TAIL;
        NetPacket theNexPacket;
    private:// no copy
        PacketSync( const PacketSync & );
        PacketSync &operator = ( const PacketSync & );
};

typedef void ( *NewConnectCB )( int clientid, void *userdata );

typedef void ( *ServerRecvCB )( int clientid, const NetPacket &packethead, const unsigned char *buf,
                                void *userdata );

typedef void ( *ClientRecvCB )( const NetPacket &packethead, const unsigned char *buf,
                                void *userdata );

#endif
