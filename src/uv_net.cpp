/**
 * @file uv_net.cpp
 * @auther Ge Tong
 */
#include "debug.h"
#include "uv_net.h"
#include "util.h"

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
    evt_ctx_set_nio( &tls_ctx, NULL, uv_tls_writer );
    iret = uv_mutex_init( &mutex_cl );
    if( iret ) {
        lasterrmsg = uvnet::GetUVError( iret );
        DebugLog( D_ERROR, D_NETWORK ) << lasterrmsg;
    }
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
    uv_thread_join( &HW_start_thread );
    uv_mutex_destroy( &mutex_cl );
    for( auto it = client_list.begin(); it != client_list.end(); ++it ) {
        FreeTcpClientCtx( *it );
    }
    client_list.clear();
    for( auto it = writeparam_list.begin(); it != writeparam_list.end(); ++it ) {
        FreeWriteParam( *it );
    }
    writeparam_list.clear();
    evt_ctx_free( &tls_ctx );
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

int uvnet::uv_tls_writer( evt_tls_t *t, void *bfr, int sz )
{
    ClientConnS *theclass = static_cast<ClientConnS *>( t->data );
    assert( theclass );
    uvnet *parent = static_cast<uvnet *>( theclass->parent_server );
    assert( parent );
    return parent->sendinl( std::string( ( char * )bfr, sz ), theclass );
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
        DebugLog( D_INFO, D_NETWORK ) << "" << thisStream->lasterrmsg;
        return;
    }
    client_tcp->packet->SetPacketCB( GetPacket, client_tcp );
    if( client_tcp->need_tls ) {
        evt_tls_accept( client_tcp->tls, on_hd_complete );
    }
    iret = uv_read_start( ( uv_stream_t * )&client_tcp->tcphandle, alloc_buffer, read_header );
    if( iret ) {
        uv_close( ( uv_handle_t * )&client_tcp->tcphandle, RecycleTcpHandle );
        DebugLog( D_INFO, D_NETWORK ) << "uv_read_start failed." << GetUVError( iret );
        return;
    }
    uv_mutex_lock( &thisStream->mutex_cl );
    thisStream->client_map.insert( std::make_pair( clientid, client_tcp ) ); //add accept client
    uv_mutex_unlock( &thisStream->mutex_cl );
    DebugLog( D_INFO, D_NETWORK ) << "New connection, status: " << status << ". new client.";
    return;
}

size_t uvnet::GetAvailaClientID() const
{
    static int s_id = 0;
    return ++s_id;
}

void uvnet::uv_rd_cb( ClientConnS *strm, ssize_t nrd, const uv_buf_t *bfr )
{
    if( nrd <= 0 ) {
        return;
    }
    std::string tmp;
    util::Buffer2String( bfr->base, nrd, tmp );
    DebugLog( D_INFO, D_NETWORK ) << "Receive: " << tmp;
    strm->packet->recvdata( ( const unsigned char * )bfr->base, nrd );
    //uv_tls_write(strm, (uv_buf_t*)bfr);
}

int uvnet::uv_tls_write( ClientConnS *stream, uv_buf_t *buf )
{
    assert( stream != NULL );
    evt_tls_t *evt = stream->tls;
    assert( evt != NULL );
    return evt_tls_write( evt, buf->base, buf->len, on_evt_write );
}

void uvnet::on_evt_write( evt_tls_t *tls, int status )
{
    assert( tls != NULL );
    ClientConnS *ut = static_cast<ClientConnS *>( tls->data );
    assert( ut != NULL );
    if( ut->tls_wr_cb != NULL ) {
        ut->tls_wr_cb( ut, status );
    }
}

void uvnet::on_hd_complete( evt_tls_t *t, int status )
{
    ClientConnS *ut = static_cast<ClientConnS *>( t->data );
    assert( ut != NULL );
    if( status == 1 ) {
        ut->tls_rd_cb = uv_rd_cb;
        evt_tls_read( ut->tls, evt_on_rd );
    } else {
        DebugLog( D_INFO, D_NETWORK ) << "TLS handshake status " << status << ". Closing.";
        ut->Close();
    }
    //uv_tls_close(ut, (uv_tls_close_cb)free);
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
    uvnet *theclass = static_cast<uvnet *>( userdata );
    DebugLog( D_INFO, D_NETWORK ) << "delete client.";
    //FreeTcpClientCtx( client );
    uv_mutex_lock( &theclass->mutex_cl );
    auto itfind = theclass->client_map.find( client->clientid );
    if( itfind != theclass->client_map.end() ) {
        if( theclass->client_list.size() > theclass->backlog ) {
            FreeTcpClientCtx( client );
        } else {
            theclass->client_list.push_back( client );
        }
        delete itfind->second;
        DebugLog( D_INFO, D_NETWORK ) << "delete client:" << itfind->first;
        theclass->client_map.erase( itfind );
    }
    uv_mutex_unlock( &theclass->mutex_cl );
}

//size_t uvnet::GetAvailaClientID() const
//{
//    static size_t s_id = 0;
//    return ++s_id;
//}

void uvnet::evt_on_rd( evt_tls_t *t, char *bfr, int sz )
{
    uv_buf_t data;
    ClientConnS *theclass = static_cast<ClientConnS *>( t->data );
    assert( theclass );
    data.base = bfr;
    data.len = sz;

    assert( theclass->tls_rd_cb != NULL );
    DebugLog( D_INFO, D_NETWORK ) << "tls_rd_cb " << sz;
    theclass->tls_rd_cb( theclass, sz, &data );
}

void uvnet::read_header( uv_stream_t *client, ssize_t nread, const uv_buf_t *buf )
{
    ClientConnS *theclass = static_cast<ClientConnS *>( client->data );
    assert( theclass );
    if( nread < 0 ) { /* Error or EOF */
        if( nread == UV_EOF ) {
            DebugLog( D_INFO, D_NETWORK ) << "client eof: " << theclass->clientid;
        } else if( nread == UV_ECONNRESET ) {
            DebugLog( D_INFO, D_NETWORK ) << "client connection reset: " << theclass->clientid;
        } else {
            DebugLog( D_WARNING, D_NETWORK ) << "client: " << theclass->clientid << " " << GetUVError(
                                                 nread );
        }
        theclass->Close();
        return;
    } else if( 0 == nread )  { /* Everything OK, but nothing read. */

    } else {
        if( theclass->need_tls ) {
            evt_tls_feed_data( theclass->tls, buf->base, nread );
        } else {
            theclass->packet->recvdata( ( const unsigned char * )buf->base, nread );
        }
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
    DebugLog( D_INFO, D_NETWORK ) << "Server is started.";
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
    assert( this->protocol != nullptr ); //the protocol must be set.
    int iret = uv_thread_create( &HW_start_thread, StartThread,
                                 this ); //use thread to wait for start succeed.
    if( iret ) {
        this->lasterrmsg = GetUVError( iret );
        DebugLog( D_WARNING, D_NETWORK ) << lasterrmsg;
        return false;
    }
    return true;
}

ClientConnS *uvnet::getClientByID( int cid )
{
    ClientConnS *res = nullptr;
    uv_mutex_lock( &mutex_cl );
    if( !( client_map.find( cid ) == client_map.end() ) ) {
        res = client_map[cid];
    }
    uv_mutex_unlock( &mutex_cl );
    return res;
}

void uvnet::GetPacket( const NetPacket &packethead, char *packetdata,
                       void *userdata )
{
    DebugLog( D_INFO, D_NETWORK ) << "Get packet type " << std::to_string( packethead.type );
    assert( userdata );
    ClientConnS *theclass = ( ClientConnS * )userdata;
    uvnet *parent = ( uvnet * )theclass->parent_server;
    //    std::string tmp;
    //    util::Buffer2String( ( char * )packetdata, packethead.datalen, tmp );
    //    DebugLog( D_INFO, D_NETWORK ) << tmp;
    //    assert( parent->protocol != nullptr );
    //const std::string &senddata =
    parent->protocol->ParsePacket( *theclass, packethead, packetdata );
    //    if( !senddata.empty() ) {
    //        parent->sendinl( senddata, theclass );
    //    }
}

void uvnet::SetProtocol( TCPServerProtocolProcess *pro )
{
    this->protocol = pro;
}

bool uvnet::sendinl( const std::string &senddata, ClientConnS *client )
{
    if( senddata.empty() ) {
        DebugLog( D_INFO, D_NETWORK ) << "send empty data.";
        return true;
    }
    write_param *writep = NULL;
    if( writeparam_list.empty() ) {
        writep = AllocWriteParam( 10240 );
    } else {
        writep = writeparam_list.front();
        writeparam_list.pop_front();
    }
    if( writep->buf.len < senddata.length() ) {
        writep->buf.base = ( char * )realloc( writep->buf.base, senddata.length() );
        writep->buf.len = senddata.length();
    }
    memcpy( writep->buf.base, senddata.data(), senddata.length() );
    writep->buf.len = senddata.length();
    writep->write_req.data = client;
    int iret = uv_write( ( uv_write_t * )&writep->write_req, ( uv_stream_t * )&client->tcphandle,
                         &writep->buf, 1, AfterSend );
    if( iret ) {
        writeparam_list.push_back( writep );
        DebugLog( D_INFO, D_NETWORK ) << "client(" << client << ") send error:" << GetUVError( iret );
        return false;
    }
    return true;
}

void uvnet::AfterSend( uv_write_t *req, int status )
{
    ClientConnS *theclass = static_cast<ClientConnS *>( req->data );
    assert( theclass );
    uvnet *parent = static_cast<uvnet *>( theclass->parent_server );
    assert( parent );
    if( parent->writeparam_list.size() > parent->backlog ) { //Maxlist
        FreeWriteParam( ( write_param * )req );
    } else {
        parent->writeparam_list.push_back( ( write_param * )req );
    }
    if( status < 0 ) {
        DebugLog( D_INFO, D_NETWORK ) << "send data error:" << GetUVError( status );
    }
}

ClientConnS *uvnet::AllocTcpClientCtx( void *parentserver, size_t suggested_size )
{
    ClientConnS *ctx = ( ClientConnS * )malloc( sizeof( *ctx ) );
    uvnet *ps = static_cast<uvnet *>( parentserver );
    assert( ps );
    ctx->need_tls = ps->protocol->need_tls;
    if( ctx->need_tls ) {
        ctx->tls = evt_ctx_get_tls( &ps->tls_ctx );
        assert( ctx->tls != NULL );
        ctx->tls->data = ctx;
    }
    ctx->packet = new PacketSync;
    ctx->read_buf.base = ( char * )malloc( suggested_size );
    ctx->read_buf.len = suggested_size;
    ctx->parent_server = ps;
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
    uvnet *theclass = static_cast<uvnet *>( arg );
    assert( theclass );
    //theclass->startstatus_ = START_FINISH;
    theclass->run_loop();
    //the server is close when come here
    uv_mutex_lock( &theclass->mutex_cl );
    for( auto it = theclass->client_map.begin(); it != theclass->client_map.end(); ++it ) {
        auto data = it->second;
        data->Close();
    }
    uv_walk( &theclass->mloop, CloseWalkCB, theclass ); //close all handle in loop
    DebugLog( D_INFO, D_NETWORK ) << "Server is closed.";
    if_exit = true;
}

void uvnet::CloseWalkCB( uv_handle_t *handle, void *arg )
{
    uvnet *theclass = static_cast<uvnet *>( arg );
    assert( theclass );
    if( !uv_is_closing( handle ) ) {
        uv_close( handle, nullptr );
    }
}

std::string uvnet::GetUVError( int errcode )
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

void ClientConnS::Close_tls( evt_tls_t *tls, int status )
{
    ( void )status;
    ClientConnS *theclass = static_cast<ClientConnS *>( tls->data );
    assert( theclass );
    uv_close( ( uv_handle_t * )&theclass->tcphandle, AfterClientClose );
}

void ClientConnS::Close()
{
    if( need_tls ) {
        evt_tls_close( tls, Close_tls );
    } else {
        uv_close( ( uv_handle_t * )&tcphandle, AfterClientClose );
    }
    DebugLog( D_INFO, D_NETWORK ) << "client close";
}

void ClientConnS::AfterClientClose( uv_handle_t *handle )
{
    ClientConnS *theclass = static_cast<ClientConnS *>( handle->data );
    assert( theclass );
    if( theclass->need_tls ) {
        evt_tls_free( theclass->tls );
    }
    if( handle == ( uv_handle_t * )&theclass->tcphandle ) {
        //theclass->isclosed = true;
        if( theclass->closedcb ) { //notice tcpserver the client had closed
            theclass->closedcb( theclass, theclass->closedcb_userdata );
        }
    }
    //free( handle );
}

void ClientConnS::SetClosedCB( TcpCloseCB pfun, void *userdata )
{
    //AfterRecv trigger this cb
    closedcb = pfun;
    closedcb_userdata = userdata;
}

