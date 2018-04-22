#pragma once
#ifndef UVNET_H
#define UVNET_H

#include <uv.h>
#include <list>
//#include <unordered_map>
#include "PacketSync.h"
#include "evt_tls.h"

struct ClientConnS;
typedef void ( *TcpCloseCB )( ClientConnS *conn_s, void *userdata );

extern bool if_exit;

struct ClientConnS {
    uv_tcp_t tcphandle;
    PacketSync *packet;
    uv_buf_t read_buf;
    int clientid;
    void *parent_server;//tcpserver
    TcpCloseCB closedcb = nullptr;
    void *closedcb_userdata = nullptr;
    uint8_t sessionkey[16] = {0};
    //void *parent_acceptclient;//accept client
    void Close();
    static void AfterClientClose( uv_handle_t *handle );
    void SetClosedCB( TcpCloseCB pfun, void *userdata );
};

typedef struct { //the param of uv_write
    uv_write_t write_req;
    uv_buf_t buf;
} write_param;

//class ClientData
//{
//    private:
//        ClientConnS *client_handle;//accept client data
//        int client_id = -1;
//        //uv_loop_t* loop = nullptr;
//        bool isclosed = false;
//        std::string errmsg;
//        //ServerRecvCB recvcb_;
//        //void* recvcb_userdata;
//
//        TcpCloseCB closedcb = nullptr;
//        void *closedcb_userdata = nullptr;
//    public:
//        ClientData( ClientConnS *control, int clientid );
//        virtual ~ClientData();
//
//        //void SetRecvCB(ServerRecvCB pfun, void* userdata);//set recv cb
//        void SetClosedCB( TcpCloseCB pfun, void *userdata ); //set close cb.
//        ClientConnS *GetTcpHandle( void ) const {
//            return client_handle;
//        }
//
//        void Close();
//
//        const char *GetLastErrMsg() const {
//            return errmsg.c_str();
//        };
//
//        static void AfterClientClose( uv_handle_t *handle );
//};

class uvnet
{
        struct uv_tls_s {
            typedef void ( *uv_handshake_cb )( uv_tls_s *, int );
            typedef void ( *uv_tls_write_cb )( uv_tls_s *, int );
            typedef void ( *uv_tls_read_cb )( uv_tls_s *, ssize_t, const uv_buf_t * );
            typedef void ( *uv_tls_close_cb )( uv_tls_s * );
            uv_tcp_t *tcp_hdl;
            evt_tls_t *tls;

            uv_tls_read_cb tls_rd_cb;
            uv_tls_close_cb tls_cls_cb;
            uv_handshake_cb tls_hsk_cb;
            uv_tls_write_cb tls_wr_cb;
        };
    public:
        //![1/0] xxxxx(v6OnlyT/F)(4/6)(U/T)
        const size_t backlog = 256;
        char net_type = 0;
        std::string bind_addr = "::";
        int port = 9001;
        uvnet();
        uvnet( std::string bindaddr, int port, bool netProtocol );
        virtual ~uvnet();
        void bind_net();
        char getNetAddrType( std::string saddr );
        bool SetNoDelay( bool enable );
        bool SetKeepAlive( int enable, unsigned int delay );
        size_t set_srvcert( std::string cert, std::string key );
        bool Start();
        std::string lasterrmsg;
        static std::string getTLSError( size_t errcode );
        static ClientConnS *AllocTcpClientCtx( void *parentserver, size_t suggested_size );
        static void FreeTcpClientCtx( ClientConnS *ctx );
        static write_param *AllocWriteParam( size_t suggested_size );
        static void FreeWriteParam( write_param *param );

    private:
        uv_tcp_t tserver;
        evt_ctx_t tls_ctx;
        //uv_udp_t userver;
        sockaddr_in addr4;
        sockaddr_in6 addr6;
        uv_loop_t mloop;
        uv_thread_t HW_start_thread;
        //uv_mutex_t mutex_cl;
        inline static std::string GetUVError( int errcode );
        //TcpCloseCB closedcb_;
        //void* closedcb_userdata;
        //std::unordered_map<int, ClientData *> client_map;
        std::list<ClientConnS *> client_list;
        std::list<write_param *> writeparam_list; //Avail write_t
        bool run_loop();
        static void DeleteTcpHandle( uv_handle_t *handle );
        static void RecycleTcpHandle( uv_handle_t *handle );
        static void SubClientClosed( ClientConnS *client, void *userdata );
        size_t GetAvailaClientID() const;

    public:
        static void on_new_connection( uv_stream_t *server, int status );
        static void alloc_buffer( uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf );
        static void read_header( uv_stream_t *client, ssize_t nread, const uv_buf_t *buf );
        static void StartThread( void *arg );
        static void GetPacket( const NetPacket &packethead, const unsigned char *packetdata,
                               void *userdata );
};
#endif
