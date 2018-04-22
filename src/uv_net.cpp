#include "debug.h"
#include "uv_net.h"

#include <assert.h>
#include <chrono>
#include <thread>

uvnet::uvnet()
{
    int iret = uv_loop_init( &mloop );
    if( iret ) {
        lasterrmsg = GetUVError( iret );
        DebugLog( D_ERROR, D_NETWORK ) << lasterrmsg;
        return;
    }
    iret = evt_ctx_init( &tls_ctx );
    if( iret ) {
        size_t err = ERR_get_error();
        DebugLog( D_ERROR, D_NETWORK ) << "evt_ctx_init error: " + std::to_string(
                                           err ) + " (" + ERR_error_string( err, NULL ) + ")";
    }
    //    iret = uv_mutex_init( &mutex_cl );
    //    if( iret ) {
    //        DebugLog( D_ERROR, D_NETWORK ) << GetUVError( iret );
    //    }
}

uvnet::uvnet( std::string bindaddr, int port, bool netProtocol ) //t,udp;f,tcp
    : net_type( netProtocol ), bind_addr( bindaddr ), port( port )
{
    uvnet();
    char addrT = getNetAddrType( bindaddr );
    if( addrT != -1 ) {
        this->net_type |= addrT;
    }
}

uvnet::~uvnet()
{
    uv_loop_close( &mloop );
    //uv_stop( mloop );
    uv_thread_join( &HW_start_thread );
    //uv_mutex_destroy( &mutex_cl );
    for( auto it = client_list.begin(); it != client_list.end(); ++it ) {
        FreeTcpClientCtx( *it );
    }
    client_list.clear();
    for( auto it = writeparam_list.begin(); it != writeparam_list.end(); ++it ) {
        FreeWriteParam( *it );
    }
    writeparam_list.clear();
}

bool uvnet::SetNoDelay( bool enable )
{
    int iret = uv_tcp_nodelay( &tserver, enable ? 1 : 0 );
    if( iret ) {
        lasterrmsg = GetUVError( iret );
        DebugLog( D_WARNING, D_NETWORK ) << lasterrmsg;
        return false;
    }
    return true;
}

bool uvnet::SetKeepAlive( int enable, unsigned int delay )
{
    int iret = uv_tcp_keepalive( &tserver, enable, delay );
    if( iret ) {
        lasterrmsg = GetUVError( iret );
        DebugLog( D_WARNING, D_NETWORK ) << lasterrmsg;
        return false;
    }
    return true;
}

void uvnet::bind_net()
{
    if( ( net_type & 1 ) == 0 ) { //TCP
        DebugLog( D_INFO, D_NETWORK ) << "Binding to TCP address " << bind_addr << ":" << port;
        uv_tcp_init( &mloop, &tserver );
        tserver.data = this;
        if( ( net_type & 2 ) == 0 ) { //IPv6
            uv_ip6_addr( bind_addr.c_str(), port, &addr6 );
            if( ( net_type & 4 ) == 1 ) { //IPv6 Only
                uv_tcp_bind( &tserver, ( const struct sockaddr * )&addr6, UV_TCP_IPV6ONLY );
            } else {
                uv_tcp_bind( &tserver, ( const struct sockaddr * )&addr6, 0 );
            }
        } else {
            uv_ip4_addr( bind_addr.c_str(), port, &addr4 );
            uv_tcp_bind( &tserver, ( const struct sockaddr * )&addr4, 0 );
        }
        int r = uv_listen( ( uv_stream_t * ) &tserver, backlog, on_new_connection );
        if( r ) {
            lasterrmsg = GetUVError( r );
            DebugLog( D_WARNING, D_NETWORK ) << "uv_listen error: " << lasterrmsg;
        }
    } else { //UDP
        DebugLog( D_WARNING, D_NETWORK ) << "UDP mode not implemented.";
    }
}

void uvnet::on_new_connection( uv_stream_t *server, int status )
{
    uvnet *thisStream = static_cast<uvnet *>( server->data );
    assert( thisStream );
    if( status < 0 ) {
        thisStream->lasterrmsg = GetUVError( status );
        DebugLog( D_WARNING, D_NETWORK ) << "New connection error" << thisStream->lasterrmsg;
        return;
    }
    ClientConnS *client_tcp = nullptr;
    if( thisStream->client_list.empty() ) {
        client_tcp = AllocTcpClientCtx( thisStream, 10240 );
    } else {
        client_tcp = thisStream->client_list.front();
        thisStream->client_list.pop_front();
        //client_tcp->parent_acceptclient = nullptr;
    }
    int iret = uv_tcp_init( &thisStream->mloop, &client_tcp->tcphandle );
    if( iret ) {
        thisStream->client_list.push_back( client_tcp ); //Recycle
        thisStream->lasterrmsg = GetUVError( iret );
        DebugLog( D_WARNING, D_NETWORK ) << thisStream->lasterrmsg;
        return;
    }
    client_tcp->tcphandle.data = client_tcp;

    auto clientid = thisStream->GetAvailaClientID();
    client_tcp->clientid = clientid;
    client_tcp->SetClosedCB( uvnet::SubClientClosed, thisStream );
    iret = uv_accept( server, ( uv_stream_t * )&client_tcp->tcphandle );
    if( iret ) {
        //thisStream->client_list.push_back( client_tcp ); //Recycle
        uv_close( ( uv_handle_t * ) &client_tcp->tcphandle, RecycleTcpHandle );
        thisStream->lasterrmsg = GetUVError( iret );
        DebugLog( D_WARNING, D_NETWORK ) << thisStream->lasterrmsg;
        return;
    }
    client_tcp->packet->SetPacketCB( GetPacket, client_tcp );
    //  client_tcp->packet->Start(tcpsock->packet_head, tcpsock->packet_tail);
    //    if( uv_tls_init() < 0 ) {
    //        uv_close( ( uv_handle_t * )&client_tcp->tcphandle, RecycleTcpHandle );
    //        thisStream->lasterrmsg = GetUVError( iret );
    //        DebugLog( D_WARNING, D_NETWORK ) << thisStream->lasterrmsg;
    //        return;
    //    }
    //    iret = uv_read_start( ( uv_stream_t * )&client_tcp->tcphandle, alloc_buffer, read_header );
    //    if( iret ) {
    //        uv_close( ( uv_handle_t * )&client_tcp->tcphandle, RecycleTcpHandle );
    //        thisStream->lasterrmsg = GetUVError( iret );
    //        DebugLog( D_WARNING, D_NETWORK ) << thisStream->lasterrmsg;
    //        return;
    //    }
    //    ClientData *cdata = new ClientData( client_tcp, clientid ); //delete on SubClientClosed
    //    client_tcp->parent_acceptclient = cdata;
    //    cdata->SetClosedCB( uvnet::SubClientClosed, thisStream );
    //    uv_mutex_lock( &thisStream->mutex_cl );
    //    thisStream->client_map.insert( std::make_pair( clientid, cdata ) ); //add accept client
    //    uv_mutex_unlock( &thisStream->mutex_cl );
    DebugLog( D_INFO, D_NETWORK ) << "New connection, status: " << status << ". new client id=" <<
                                  clientid;
    return;
}

void uvnet::alloc_buffer( uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf )
{
    ( void )handle;
    buf->base = ( char * ) malloc( suggested_size );
    buf->len = suggested_size;
}

void uvnet::DeleteTcpHandle( uv_handle_t *handle )
{
    ClientConnS *theclass = static_cast<ClientConnS *>( handle->data );
    assert( theclass );
    FreeTcpClientCtx( theclass );
}

void uvnet::RecycleTcpHandle( uv_handle_t *handle )
{
    //the handle on TcpClient had closed.
    ClientConnS *theclass = static_cast<ClientConnS *>( handle->data );
    assert( theclass );
    uvnet *parent = static_cast<uvnet *>( theclass->parent_server );
    assert( parent );
    if( parent->client_list.size() > parent->backlog ) {
        FreeTcpClientCtx( theclass );
    } else {
        parent->client_list.push_back( theclass );
    }
}

void uvnet::SubClientClosed( ClientConnS *client, void *userdata )
{
    //uvnet *theclass = static_cast<uvnet *>( userdata );
    DebugLog( D_INFO, D_NETWORK ) << "delete client:" << client->clientid;
    FreeTcpClientCtx( client );
    //    uvnet *theclass = static_cast<uvnet *>( userdata );
    //    uv_mutex_lock( &theclass->mutex_cl );
    //    auto itfind = theclass->client_map.find( clientid );
    //    if( itfind != theclass->client_map.end() ) {
    //        //if (theclass->closedcb) {
    //        //    theclass->closedcb(clientid, theclass->closedcb_userdata);
    //        //}
    //        if( theclass->client_list.size() > theclass->backlog ) {
    //            FreeTcpClientCtx( itfind->second->GetTcpHandle() );
    //        } else {
    //            theclass->client_list.push_back( itfind->second->GetTcpHandle() );
    //        }
    //        delete itfind->second;
    //        DebugLog( D_INFO, D_NETWORK ) << "delete client:" << itfind->first;
    //        theclass->client_map.erase( itfind );
    //    }
    //    uv_mutex_unlock( &theclass->mutex_cl );
}

size_t uvnet::GetAvailaClientID() const
{
    static size_t s_id = 0;
    return ++s_id;
}

void uvnet::read_header( uv_stream_t *client, ssize_t nread, const uv_buf_t *buf )
{
    ClientConnS *theclass = static_cast<ClientConnS *>( client->data );
    assert( theclass );
    if( nread < 0 ) { /* Error or EOF */
        if( nread == UV_EOF ) {
            DebugLog( D_INFO, D_NETWORK ) << "client(" << theclass->clientid << ")eof";
        } else if( nread == UV_ECONNRESET ) {
            DebugLog( D_INFO, D_NETWORK ) << "client(" << theclass->clientid << ")conn reset";
        } else {
            DebugLog( D_WARNING, D_NETWORK ) << "client(" << theclass->clientid << ")：" << GetUVError(
                                                 nread );
        }
        //ClientData *acceptclient = ( ClientData * )theclass->parent_acceptclient;
        //acceptclient->Close();
        theclass->Close();
        return;
    } else if( 0 == nread )  { /* Everything OK, but nothing read. */

    } else {
        theclass->packet->recvdata( ( const unsigned char * )buf->base, nread );
    }

    if( buf->base ) {
        free( buf->base );
    }
}

char uvnet::getNetAddrType( std::string saddr )
{
    struct sockaddr_in6 ipv6address;
    struct sockaddr_in ipv4address;
    int r = uv_ip6_addr( saddr.c_str(), port, &ipv6address );
    if( r == 0 )
        //if(ipv6address.sin6_scope_id != 0)
    {
        return 0;    //IPv6
    }
    r = uv_ip4_addr( saddr.c_str(), port, &ipv4address );
    if( r == 0 ) {
        return 2;    //IPv4
    }
    return -1;//bad address
}

bool uvnet::run_loop()
{
    DebugLog( D_INFO, D_NETWORK ) << "Server is starting...";
    int iret = uv_run( &mloop, UV_RUN_DEFAULT );
    if( iret ) {
        lasterrmsg = GetUVError( iret );
        DebugLog( D_WARNING, D_NETWORK ) << lasterrmsg;
        return false;
    }
    return true;
}

size_t uvnet::set_srvcert( std::string cert, std::string key )
{
    assert( tls_ctx.ctx );
    DebugLog( D_INFO, D_NETWORK ) << "Loading server certification path: " << cert << " key path: " <<
                                  key;
    return evt_ctx_set_crt_key( &tls_ctx, cert.c_str(), key.c_str() );
}

std::string uvnet::getTLSError( size_t errcode )
{
    std::string res = "SSL error code:[" + std::to_string( errcode ) + "](" + ERR_error_string( errcode,
                      NULL ) + ")\n";
    const char *reason = ERR_reason_error_string( errcode );
    if( reason != nullptr ) {
        res += "Error reason is" + std::string( reason );
    }
    return res;
}

bool uvnet::Start()
{
    //return uv_run( mloop, UV_RUN_DEFAULT );
    int iret = uv_thread_create( &HW_start_thread, StartThread,
                                 this ); //use thread to wait for start succeed.
    if( iret ) {
        this->lasterrmsg = GetUVError( iret );
        DebugLog( D_WARNING, D_NETWORK ) << lasterrmsg;
        return false;
    }
    return true;
}

void uvnet::GetPacket( const NetPacket &packethead, const unsigned char *packetdata,
                       void *userdata )
{
    DebugLog( D_INFO, D_NETWORK ) << "Get packet type " << std::to_string( packethead.type );
    //fprintf(stdout, "Get packet type %d\n", packethead.type);
    assert( userdata );
    ClientConnS *theclass = ( ClientConnS * )userdata;
    //uvnet* parent = (uvnet*)theclass->parent_server;
    DebugLog( D_INFO, D_NETWORK ) << packetdata;
    //const std::string& senddata = parent->protocol_->ParsePacket(packethead, packetdata);
    //parent->sendinl(senddata, theclass);
    return;
}

ClientConnS *uvnet::AllocTcpClientCtx( void *parentserver, size_t suggested_size )
{
    ClientConnS *ctx = ( ClientConnS * )malloc( sizeof( *ctx ) );
    ctx->packet = new PacketSync;
    ctx->read_buf.base = ( char * )malloc( suggested_size );
    ctx->read_buf.len = suggested_size;
    ctx->parent_server = parentserver;
    return ctx;
}

void uvnet::FreeTcpClientCtx( ClientConnS *ctx )
{
    delete ctx->packet;
    free( ctx->read_buf.base );
    free( ctx );
}

write_param *uvnet::AllocWriteParam( size_t suggested_size )
{
    write_param *param = ( write_param * )malloc( sizeof( write_param ) );
    param->buf.base = ( char * )malloc( suggested_size );
    param->buf.len = suggested_size;
    return param;
}

void uvnet::FreeWriteParam( write_param *param )
{
    free( param->buf.base );
    free( param );
}

void uvnet::StartThread( void *arg )
{
    uvnet *theclass = ( uvnet * )arg;
    //theclass->startstatus_ = START_FINISH;
    theclass->run_loop();
    //the server is close when come here
    //theclass->isclosed_ = true;
    DebugLog( D_INFO, D_NETWORK ) << "Server is closed.";
    if_exit = true;
    //    if (theclass->closedcb_) {
    //        theclass->closedcb_(-1, theclass->closedcb_userdata_);
    //    }
}

inline std::string uvnet::GetUVError( int errcode )
{
    if( 0 == errcode ) {
        return "";
    }
    std::string err;
    auto tmpChar = uv_err_name( errcode );
    if( tmpChar ) {
        err = tmpChar;
        err += ":";
    } else {
        err = "Unknown system errcode " + std::to_string( ( long long )errcode );
        err += ":";
    }
    tmpChar = uv_strerror( errcode );
    if( tmpChar ) {
        err += tmpChar;
    }
    return err;
}

void ClientConnS::Close()
{
    uv_close( ( uv_handle_t * )&tcphandle, AfterClientClose );
    DebugLog( D_INFO, D_NETWORK ) << "client(" << clientid << ")close";
}

void ClientConnS::AfterClientClose( uv_handle_t *handle )
{
    ClientConnS *theclass = static_cast<ClientConnS *>( handle->data );
    assert( theclass );
    if( handle == ( uv_handle_t * )&theclass->tcphandle ) {
        //theclass->isclosed = true;
        if( theclass->closedcb ) { //notice tcpserver the client had closed
            theclass->closedcb( theclass, theclass->closedcb_userdata );
        }
    }
}

void ClientConnS::SetClosedCB( TcpCloseCB pfun, void *userdata )
{
    //AfterRecv trigger this cb
    closedcb = pfun;
    closedcb_userdata = userdata;
}

//ClientData::ClientData( ClientConnS *control, int clientid )
//    : client_handle( control )
//    , client_id( clientid ), isclosed( false )
//{}
//
//ClientData::~ClientData()
//{
//    Close();
//    //while will block loop.
//    //the right way is new AcceptClient and delete it on SetClosedCB'cb
//    while( !isclosed ) {
//        std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
//    }
//}
//
//void ClientData::Close()
//{
//    if( isclosed ) {
//        return;
//    }
//    client_handle->tcphandle.data = this;
//    //send close command
//    uv_close( ( uv_handle_t * )&client_handle->tcphandle, AfterClientClose );
//    DebugLog( D_INFO, D_NETWORK ) << "client(" << client_id << ")close";
//}
//
//void ClientData::AfterClientClose( uv_handle_t *handle )
//{
//    ClientData *theclass = static_cast<ClientData *>( handle->data );
//    assert( theclass );
//    if( handle == ( uv_handle_t * )&theclass->client_handle->tcphandle ) {
//        theclass->isclosed = true;
//        if( theclass->closedcb ) { //notice tcpserver the client had closed
//            theclass->closedcb( theclass->client_id, theclass->closedcb_userdata );
//        }
//    }
//}
//
//void ClientData::SetClosedCB( TcpCloseCB pfun, void *userdata )
//{
//    //AfterRecv trigger this cb
//    closedcb = pfun;
//    closedcb_userdata = userdata;
//}
