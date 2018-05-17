#include <stdlib.h>
#include "PacketSync.h"
#include "debug.h"

PacketSync::PacketSync(): packet_cb_( NULL ), packetcb_userdata_( NULL )
{
    thread_readdata = uv_buf_init( ( char * )malloc( BUFFER_SIZE ),
                                   BUFFER_SIZE );
    thread_packetdata = uv_buf_init( ( char * )malloc( BUFFER_SIZE ),
                                     BUFFER_SIZE );
    truepacketlen = 0;
    headpos = -1;
    headpt = NULL;
    parsetype = PARSE_NOTHING;
    getdatalen = 0;
    HEAD = 0x0C;
    TAIL = 0xEA;
}

PacketSync::~PacketSync()
{
    free( thread_readdata.base );
    free( thread_packetdata.base );
}

bool PacketSync::SetHaT( char packhead, char packtail )
{
    HEAD = packhead;
    TAIL = packtail;
    return true;
}

void PacketSync::recvdata( const unsigned char *data, size_t len )
{
    size_t iret = 0;
    while( iret < len || truepacketlen >= NET_PACKAGE_HEADLEN + 2 ) {
        if( PARSE_NOTHING == parsetype ) {
            if( thread_readdata.len - truepacketlen >= len - iret ) {
                memcpy( thread_readdata.base + truepacketlen, data + iret, len - iret );
                truepacketlen += len - iret;
                iret  = len;
            } else {
                memcpy( thread_readdata.base + truepacketlen, data + iret, thread_readdata.len - truepacketlen );
                iret += thread_readdata.len - truepacketlen;
                truepacketlen = thread_readdata.len;
            }
            headpt = ( char * )memchr( thread_readdata.base, HEAD, truepacketlen );
            if( !headpt ) { //1
                DebugLog( D_INFO, D_NETWORK ) << "Reading " << truepacketlen <<
                                              " data, but packet head is not found.";
                truepacketlen = 0;
                continue;
            }
            headpos = headpt - thread_readdata.base;
            if( truepacketlen - headpos - 1 < NET_PACKAGE_HEADLEN ) {
                if( headpos != 0 ) {
                    DebugLog( D_INFO, D_NETWORK ) << "Reading " << truepacketlen << " found head at " << headpos <<
                                                  "catching...";
                    memmove( thread_readdata.base, thread_readdata.base + headpos, truepacketlen - headpos );
                    truepacketlen -= headpos;
                }
                continue;
            }
            headpt = &thread_readdata.base[headpos + 1];
            CharToNetPacket( ( const unsigned char * )( headpt ), theNexPacket );
            if( theNexPacket.header != HEAD ||
                theNexPacket.tail != TAIL ) {
                memmove( thread_readdata.base, thread_readdata.base + headpos + 1,
                         truepacketlen - headpos - 1 );
                truepacketlen -= headpos + 1;
                continue;
            }
            parsetype = PARSE_HEAD;
            if( thread_packetdata.len < ( size_t )theNexPacket.datalen + 1 ) {
                thread_packetdata.base = ( char * )realloc( thread_packetdata.base, theNexPacket.datalen + 1 );
                thread_packetdata.len = theNexPacket.datalen + 1;
            }
            getdatalen = ( std::min )( ( int )( truepacketlen - headpos - 1 - NET_PACKAGE_HEADLEN ),
                                       ( int )( theNexPacket.datalen + 1 ) );
            if( getdatalen > 0 ) {
                memcpy( thread_packetdata.base, thread_readdata.base + headpos + 1 + NET_PACKAGE_HEADLEN,
                        getdatalen );
            }
        }

        if( getdatalen < theNexPacket.datalen + 1 ) {
            if( getdatalen + ( len - iret ) < theNexPacket.datalen + 1 ) {
                memcpy( thread_packetdata.base + getdatalen, data + iret, len - iret );
                getdatalen += len - iret;
                iret = len;
                return;
            } else {
                memcpy( thread_packetdata.base + getdatalen, data + iret, theNexPacket.datalen + 1 - getdatalen );
                iret += theNexPacket.datalen + 1 - getdatalen;
                getdatalen = theNexPacket.datalen + 1;
            }
        }
        if( ( uint8_t )thread_packetdata.base[theNexPacket.datalen] != TAIL ) {
            if( truepacketlen - headpos - 1 - NET_PACKAGE_HEADLEN >= theNexPacket.datalen +
                1 ) {
                memmove( thread_readdata.base, thread_readdata.base + headpos + 1,
                         truepacketlen - headpos - 1 );
                truepacketlen -= headpos + 1;
            } else {
                if( thread_readdata.len < NET_PACKAGE_HEADLEN + theNexPacket.datalen + 1 ) {
                    thread_readdata.base = ( char * )realloc( thread_readdata.base,
                                           NET_PACKAGE_HEADLEN + theNexPacket.datalen + 1 );
                    thread_readdata.len = NET_PACKAGE_HEADLEN + theNexPacket.datalen + 1;
                }
                memmove( thread_readdata.base, thread_readdata.base + headpos + 1, NET_PACKAGE_HEADLEN );
                truepacketlen = NET_PACKAGE_HEADLEN;
                memcpy( thread_readdata.base + truepacketlen, thread_packetdata.base, theNexPacket.datalen + 1 );
                truepacketlen += theNexPacket.datalen + 1;
            }
            parsetype = PARSE_NOTHING;
            continue;
        }
        if( truepacketlen - headpos - 1 - NET_PACKAGE_HEADLEN >= theNexPacket.datalen +
            1 ) {
            memmove( thread_readdata.base,
                     thread_readdata.base + headpos + NET_PACKAGE_HEADLEN + theNexPacket.datalen + 2,
                     truepacketlen - ( headpos + NET_PACKAGE_HEADLEN + theNexPacket.datalen + 2 ) );
            truepacketlen -= headpos + NET_PACKAGE_HEADLEN + theNexPacket.datalen + 2;
        } else {
            truepacketlen = 0;
        }
        if( this->packet_cb_ ) {
            this->packet_cb_( theNexPacket, ( const char * )thread_packetdata.base,
                              this->packetcb_userdata_ );
        }
        parsetype = PARSE_NOTHING;
    }
}

void PacketSync::SetPacketCB( GetFullPacket pfun, void *userdata )
{
    packet_cb_ = pfun;
    packetcb_userdata_ = userdata;
}

/*
* @brief   Combining data into a binary stream in NetPacket format can be sent directly.
* @param   packet --NetPacket
           data   --The actual data to send
* @return  std::string --The returned binary stream. Start: &string[0] length: string.length()
******************************/
std::string PacketSync::PacketData( NetPacket &packet, const unsigned char *data )
{
    unsigned char packchar[NET_PACKAGE_HEADLEN];
    NetPacketToChar( packet, packchar );

    std::string retstr;
    retstr.append( 1, packet.header );
    retstr.append( ( const char * )packchar, NET_PACKAGE_HEADLEN );
    retstr.append( ( const char * )data, packet.datalen );
    retstr.append( 1, packet.tail );
    return retstr;
}
