/**
 * @file gameproto.h
 * @auther Ge Tong
 */

#include <string>

#include "tcpprotocol.h"
#include "FromMsg.h"
#include "ToMsg.h"

class GameTCPProtocol : public TCPServerProtocolProcess
{
    public:
        GameTCPProtocol() {}
        virtual ~GameTCPProtocol() {}
        const std::string &ParsePacket( const NetPacket &packet, const char *buf );
    private:
        std::string pro_packet;
};
