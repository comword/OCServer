/**
 * @file ev_dispatch.cpp
 * @auther Ge Tong
 */
#include "ev_dispatch.h"
#include "debug.h"
#include "uv_net.h"
#include "jImpl.h"

#include <assert.h>

EvDispatch::EvDispatch()
{
    int iret = uv_loop_init( &mloop );
    if( iret ) {
        lasterrmsg = uvnet::GetUVError( iret );
        DebugLog( D_ERROR, D_NETWORK ) << lasterrmsg;
        return;
    }
    iret = uv_mutex_init( &mutex_queue );
    if( iret ) {
        lasterrmsg = uvnet::GetUVError( iret );
        DebugLog( D_ERROR, D_NETWORK ) << lasterrmsg;
    }
    iret = uv_async_init( &mloop , &async, afterpost_cb );
    if( iret ) {
        lasterrmsg = uvnet::GetUVError( iret );
        DebugLog( D_ERROR, D_NETWORK ) << lasterrmsg;
    }
    async.data = this;
}

EvDispatch::~EvDispatch()
{
    uv_close( ( uv_handle_t * ) &async, NULL );
    uv_loop_close( &mloop );
    uv_thread_join( &mthread );
    uv_mutex_destroy( &mutex_queue );
}

bool EvDispatch::Start()
{
    int iret = uv_thread_create( &mthread, StartThread, this );
    if( iret ) {
        this->lasterrmsg = uvnet::GetUVError( iret );
        DebugLog( D_WARNING, D_NETWORK ) << lasterrmsg;
        return false;
    }
    return true;
}

void EvDispatch::StartThread( void *arg )
{
    EvDispatch *theclass = static_cast<EvDispatch *>( arg );
    assert( theclass );
    DebugLog( D_INFO, D_NETWORK ) << "Starting EvDispatch.";
    int iret = uv_run( &theclass->mloop, UV_RUN_DEFAULT );
    if( iret ) {
        theclass->lasterrmsg = uvnet::GetUVError( iret );
        DebugLog( D_WARNING, D_NETWORK ) << theclass->lasterrmsg;
    }
    DebugLog( D_INFO, D_NETWORK ) << "EvDispatch is dead.";
}

void EvDispatch::postMsg( FromMsg *msg )
{
    uv_mutex_lock( &mutex_queue );
    qfmsg.push( msg );
    uv_mutex_unlock( &mutex_queue );
    uv_async_send( &async );
}

void EvDispatch::afterpost_cb( uv_async_t *async )
{
    EvDispatch *theclass = static_cast<EvDispatch *>( async->data );
    assert( theclass );
    while( !theclass->checkQueue() ) {
        uv_mutex_lock( &theclass->mutex_queue );
        FromMsg *msg = theclass->qfmsg.front();
        theclass->proc_msg( msg );
        delete msg;
        theclass->qfmsg.pop();
        uv_mutex_unlock( &theclass->mutex_queue );
    }
}

bool EvDispatch::checkQueue()
{
    uv_mutex_lock( &mutex_queue );
    bool r = qfmsg.empty();
    uv_mutex_unlock( &mutex_queue );
    return r;
}

void EvDispatch::proc_msg( FromMsg *msg )
{
    std::string cmd = msg->serviceCmd;
    //DebugLog(D_DEBUG, D_NETWORK) << "Processing "+cmd;
    jobject srv = jc->getService( cmd );
    if( srv == nullptr ) { //Unhandled message
        DebugLog( D_INFO, D_NETWORK ) << "Unhandled service command: " + cmd;
        delete msg;
        return;
    }
    std::string r;
    bool res = jc->execTransact( srv, msg->netBuffer, r, 0 );
    if( res ) {

    }
    delete msg;
}
