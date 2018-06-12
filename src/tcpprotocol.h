#ifndef TCP_SERVER_PROTOCOL_PROCESS_H
#define TCP_SERVER_PROTOCOL_PROCESS_H
#include <string>
#include "net_base.h"

struct ClientConnS;
class TCPServerProtocolProcess
{
    public:
        TCPServerProtocolProcess() {}
        virtual ~TCPServerProtocolProcess() {}
        bool need_tls = false;
        std::string srv_crt, srv_key;
        //parse the recv packet, and make the response packet return.
        //packet     : the recv packet
        //buf        : the packet data
        //std::string: the response packet. no response can't return empty string.
        virtual void ParsePacket( ClientConnS &client, const NetPacket &packet, char *buf ) = 0;
};

#endif//TCP_SERVER_PROTOCOL_PROCESS_H
