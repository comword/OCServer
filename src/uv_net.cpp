#include "debug.h"
#include "uv_net.h"

#include <assert.h>

uvnet::uvnet()
{
	int iret = uv_loop_init(&mloop);
	if (iret) {
		DebugLog( D_ERROR, D_NETWORK ) << GetUVError(iret);
		return;
	}
//    iret = uv_mutex_init(&mutex_cl);
//    if (iret) {
//    	DebugLog( D_ERROR, D_NETWORK ) << GetUVError(iret);
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
	uv_loop_close(&mloop);
    //uv_stop( mloop );
    uv_thread_join(&HW_start_thread);
	//uv_mutex_destroy(&mutex_clients_);
}

bool uvnet::SetNoDelay(bool enable)
{
    int iret = uv_tcp_nodelay(&tserver, enable ? 1 : 0);
    if (iret) {
    	DebugLog( D_WARNING, D_NETWORK ) <<  GetUVError(iret);
        return false;
    }
    return true;
}

bool uvnet::SetKeepAlive(int enable, unsigned int delay)
{
    int iret = uv_tcp_keepalive(&tserver, enable , delay);
    if (iret) {
    	DebugLog( D_WARNING, D_NETWORK ) <<  GetUVError(iret);
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
            DebugLog( D_WARNING, D_NETWORK ) << "uv_listen error: " << uv_strerror( r );
        }
    } else { //UDP
    	DebugLog( D_WARNING, D_NETWORK ) << "UDP mode not implemented.";
    }
}

void uvnet::on_new_connection( uv_stream_t *server, int status )
{
    uvnet *thisStream = static_cast<uvnet *>( server->data );
    assert(thisStream);
    if( status < 0 ) {
        DebugLog( D_WARNING, D_NETWORK ) << "New connection error" << uv_strerror( status );
        return;
    }

//    uv_tcp_t *client = ( uv_tcp_t * ) malloc( sizeof( uv_tcp_t ) );
//    uv_tcp_init( &thisStream->mloop, client );
//    if( uv_accept( server, ( uv_stream_t * ) client ) == 0 ) {
//        DebugLog( D_INFO, D_NETWORK ) << "New connection, status: " << status;
//        uv_read_start( ( uv_stream_t * ) client, alloc_buffer, read_header );
//    } else {
//        uv_close( ( uv_handle_t * ) client, NULL );
//    }
}

void uvnet::alloc_buffer( uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf )
{
	( void )handle;
    buf->base = ( char * ) malloc( suggested_size );
    buf->len = suggested_size;
}

void uvnet::echo_write( uv_write_t *req, int status )
{
    if( status ) {
        fprintf( stderr, "Write error %s\n", uv_strerror( status ) );
    }
    free( req );
}

void uvnet::read_header( uv_stream_t *client, ssize_t nread, const uv_buf_t *buf )
{
    if( nread < 0 ) {
        if( nread != UV_EOF ) {
            fprintf( stderr, "Read error %s\n", uv_err_name( nread ) );
        }
        uv_close( ( uv_handle_t * ) client, NULL );
    } else if( nread > 0 ) {

//        uv_write_t *req = ( uv_write_t * ) malloc( sizeof( uv_write_t ) );
//        uv_buf_t wrbuf = uv_buf_init( buf->base, nread );
//        uv_write( req, client, &wrbuf, 1, echo_write );
    }
//    printf( "echo_read:%s\n", buf->base );
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
	DebugLog( D_INFO, D_NETWORK )<<"Server is starting...";
    int iret = uv_run(&mloop, UV_RUN_DEFAULT);
    if (iret) {
    	DebugLog( D_WARNING, D_NETWORK ) << GetUVError(iret);
        return false;
    }
    return true;
}

bool uvnet::Start()
{
    //return uv_run( mloop, UV_RUN_DEFAULT );
	int iret = uv_thread_create(&HW_start_thread, StartThread, this);//use thread to wait for start succeed.
	if (iret) {
		DebugLog( D_WARNING, D_NETWORK ) << GetUVError(iret);
		return false;
	}
	return true;
}

ClientConnS* uvnet::AllocTcpClientCtx(void* parentserver, size_t suggested_size)
{
	ClientConnS* ctx = (ClientConnS*)malloc(sizeof(*ctx));
	ctx->packet_ = new FromMsg;
	ctx->read_buf_.base = (char*)malloc(suggested_size);
	ctx->read_buf_.len = suggested_size;
	ctx->parent_server = parentserver;
	return ctx;
}

void uvnet::FreeTcpClientCtx(ClientConnS* ctx)
{

}

write_param* uvnet::AllocWriteParam(void)
{

}

void uvnet::FreeWriteParam(write_param* param)
{

}

void uvnet::StartThread(void* arg)
{
	uvnet* theclass = (uvnet*)arg;
    //theclass->startstatus_ = START_FINISH;
    theclass->run_loop();
    //the server is close when come here
    //theclass->isclosed_ = true;
    DebugLog( D_INFO, D_NETWORK )<<"Server is closed.";
//    if (theclass->closedcb_) {
//        theclass->closedcb_(-1, theclass->closedcb_userdata_);
//    }
}

inline std::string uvnet::GetUVError(int errcode)
{
    if (0 == errcode) {
        return "";
    }
    std::string err;
    auto tmpChar = uv_err_name(errcode);
    if (tmpChar) {
        err = tmpChar;
        err += ":";
    }else{
		err = "Unknown system errcode "+std::to_string((long long)errcode);
		err += ":";
	}
    tmpChar = uv_strerror(errcode);
    if (tmpChar) {
        err += tmpChar;
    }
    return std::move(err);
}
