#pragma once
#ifndef UVNET_H
#define UVNET_H

#include <uv.h>
#include <list>
//#include <unordered_map>
#include "FromMsg.h"
#include "ToMsg.h"

//typedef void (*TcpCloseCB)(int clientid, void* userdata);
//class ClientData;
typedef struct {
    uv_tcp_t tcphandle;//data filed store this
    FromMsg* packet_;//userdata filed storethis
    uv_buf_t read_buf_;
    int clientid;
    void* parent_server;//tcpserver
    //void* parent_acceptclient;//accept client
} ClientConnS;

typedef struct { //the param of uv_write
	uv_write_t write_req_;
    uv_buf_t buf_;
    int buf_truelen_;
} write_param;

class uvnet
{
    public:
        //![1/0] xxxxx(v6OnlyT/F)(4/6)(U/T)
        const int backlog = 256;
        char net_type = 0;
        std::string bind_addr = "::";
        int port = 9001;
        uvnet();
        uvnet( std::string bindaddr, int port, bool netProtocol );
        ~uvnet();
        void bind_net();
        char getNetAddrType( std::string saddr );
        bool SetNoDelay(bool enable);
        bool SetKeepAlive(int enable, unsigned int delay);
        bool Start();
        ClientConnS* AllocTcpClientCtx(void* parentserver, size_t suggested_size);
        void FreeTcpClientCtx(ClientConnS* ctx);
        write_param* AllocWriteParam(void);
        void FreeWriteParam(write_param* param);
    private:
        uv_tcp_t tserver;
        //uv_udp_t userver;
        sockaddr_in addr4;
        sockaddr_in6 addr6;
        uv_loop_t mloop;
        uv_thread_t HW_start_thread;
        //uv_mutex_t mutex_cl;
        inline std::string GetUVError(int errcode);
        //TcpCloseCB closedcb_;
        void* closedcb_userdata_;
        //std::unordered_map<int, ClientData*> clients_map;
        std::list<ClientConnS*> client_list;
        bool run_loop();

    public:
        static void on_new_connection( uv_stream_t *server, int status );
        static void alloc_buffer( uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf );
        static void echo_write( uv_write_t *req, int status );
        static void read_header( uv_stream_t *client, ssize_t nread, const uv_buf_t *buf );
        static void StartThread(void* arg);

};
#endif
