#pragma once
#ifndef UVNET_H
#define UVNET_H

#include <uv.h>
#include <list>
#include <unordered_map>
#include "PacketSync.h"
#include "evt_tls.h"
#include "tcpprotocol.h"

extern bool if_exit;
struct ClientConnS {
    typedef void ( *TcpCloseCB )( ClientConnS *, void * );
    typedef void ( *uv_handshake_cb )( ClientConnS *, int );
    typedef void ( *uv_tls_write_cb )( ClientConnS *, int );
    typedef void ( *uv_tls_read_cb )( ClientConnS *, ssize_t, const uv_buf_t * );
    bool need_tls = false;
    size_t clientid;
    uv_tcp_t tcphandle;
    evt_tls_t *tls;
    PacketSync *packet;
    uv_buf_t read_buf;
    void *parent_server;//tcpserver
    TcpCloseCB closedcb = nullptr;
    void *closedcb_userdata = nullptr;
    uv_tls_read_cb tls_rd_cb = nullptr;
    uv_handshake_cb tls_hsk_cb = nullptr;
    uv_tls_write_cb tls_wr_cb = nullptr;
    uint16_t quota = 0xffff;
    bool trust = false;
    void Close();
    static void AfterClientClose( uv_handle_t *handle );
    void SetClosedCB( TcpCloseCB pfun, void *userdata );
    static void Close_tls( evt_tls_t *tls, int status );
};

typedef struct { //the param of uv_write
    uv_write_t write_req;
    uv_buf_t buf;
} write_param;

class uvnet
{
    public:
        //![1/0] xxxxx(v6OnlyT/F)(4/6)(U/T)
        const size_t backlog = 256;
        char net_type = 0;
        std::string bind_addr = "::";
        int port = 9001;
        std::string lasterrmsg;
        uvnet();
        uvnet( std::string bindaddr, int port, bool netProtocol );
        virtual ~uvnet();
        void bind_net();
        char getNetAddrType( std::string saddr );
        bool SetNoDelay( bool enable );
        bool SetKeepAlive( int enable, unsigned int delay );
        size_t set_srvcert( std::string cert, std::string key );
        bool sendinl( const std::string &senddata, ClientConnS *client );
        bool Start();
        void SetProtocol( TCPServerProtocolProcess *pro );
        static void GetPacket( const NetPacket &packethead, const char *packetdata,
                               void *userdata );

    private:
        uv_tcp_t tserver;
        evt_ctx_t tls_ctx;
        uv_mutex_t mutex_cl;
        //uv_udp_t userver;
        sockaddr_in addr4;
        sockaddr_in6 addr6;
        uv_loop_t mloop;
        uv_thread_t HW_start_thread;
        TCPServerProtocolProcess *protocol = nullptr;
        inline static std::string GetUVError( int errcode );
        std::list<ClientConnS *> client_list;
        std::unordered_map<int, ClientConnS *> client_map;
        std::list<write_param *> writeparam_list;
        bool run_loop();
        static void DeleteTcpHandle( uv_handle_t *handle );
        static void RecycleTcpHandle( uv_handle_t *handle );
        static void SubClientClosed( ClientConnS *client, void *userdata );
        static void CloseWalkCB( uv_handle_t *handle, void *arg );
        size_t GetAvailaClientID() const;
        static int uv_tls_write( ClientConnS *stream, uv_buf_t *buf );
        static void on_evt_write( evt_tls_t *tls, int status );
        static int uv_tls_writer( evt_tls_t *t, void *bfr, int sz );
        static std::string getTLSError( size_t errcode );
        static ClientConnS *AllocTcpClientCtx( void *parentserver, size_t suggested_size );
        static void FreeTcpClientCtx( ClientConnS *ctx );
        static write_param *AllocWriteParam( size_t suggested_size );
        static void FreeWriteParam( write_param *param );
        static void on_hd_complete( evt_tls_t *t, int status );
        static void evt_on_rd( evt_tls_t *t, char *bfr, int sz );
        static void uv_rd_cb( ClientConnS *strm, ssize_t nrd, const uv_buf_t *bfr );
        static void AfterSend( uv_write_t *req, int status );
        static void on_new_connection( uv_stream_t *server, int status );
        static void alloc_buffer( uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf );
        static void read_header( uv_stream_t *client, ssize_t nread, const uv_buf_t *buf );
        static void StartThread( void *arg );
};
#endif
