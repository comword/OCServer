/**
 * @file gameproto.cpp
 * @auther Ge Tong
 */

#include "gameproto.h"
#include "proto.h"
#include "debug.h"
#include "ev_dispatch.h"
#include "uv_net.h"

void GameTCPProtocol::ParsePacket( ClientConnS &client, const NetPacket &packet, char *buf )
{
    FromMsg *m = new FromMsg( client );
    m->fromVersion = packet.version;
    m->packet_type = packet.type;
    m->netBuffer = std::string( buf, packet.datalen );
    bool pack_ok = bufFromMsg( packet, buf, *m );
    if( !pack_ok ) {
        delete m;
    } else {
        msgq->postMsg( m );
    }
}

bool GameTCPProtocol::bufFromMsg( const NetPacket &packet, char *buf, FromMsg &msg )
{
    membuf sbuf( buf, buf + packet.datalen );
    std::istream b( &sbuf );
    ProtoIn pi( b );
    uint32_t UID;
    std::string cmd;
    uint32_t packSeq;
    HashMap attrib;
    try {
        pi.read( UID ); //0
        pi.read( cmd ); //1
        pi.read( packSeq ); //2
        pi.read( attrib ); //3
    } catch( ProtoError &e ) {
        DebugLog( D_INFO, D_PROTO ) << "Unparseable message: " << e.what();
        return false;
    }
    // build the FromMsg
    msg.uin = std::to_string( UID );
    msg.serviceCmd = cmd;
    msg.packSeq = packSeq;
    msg.attributes = attrib;
    return true;
}

bool GameTCPProtocol::sendMsg( ToMsg &msg )
{
    uvnet *parent = static_cast<uvnet *>( msg.client.parent_server );
    return parent->sendinl( msg.netBuffer, &msg.client );
}
