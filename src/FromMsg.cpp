#include "FromMsg.h"
void FromMsg::ParsePacket( const NetPacket &packethead, const char *packetdata )
{
    fromVersion = packethead.version;
    packet_type = packethead.type;
    if( packet_type == 1 ) {
        serviceCmd = "SRV_LOGIN";
    }
    uin = packethead.uid;
    netBuffer = std::string( packetdata, packethead.datalen );
}
