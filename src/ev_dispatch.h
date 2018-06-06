/**
 * @file ev_dispatch.h
 * @auther Ge Tong
 */

#pragma once
#ifndef EV_DISPATCH_H
#define EV_DISPATCH_H
#include <uv.h>
#include <string>
#include <queue>

#include "gameproto.h"
class EvDispatch
{
        virtual ~EvDispatch();
    private:
        uv_loop_t mloop;
        uv_thread_t mthread;
        uv_mutex_t mutex_queue;
        std::string lasterrmsg;
        std::queue<FromMsg * > qfmsg;
        uv_async_t async;
    public:
        EvDispatch();
        static void StartThread( void *arg );
        bool Start();
        void postMsg( FromMsg *msg );
    private:
        static void afterpost_cb( uv_async_s *async );
        bool checkQueue();
        void proc_msg( FromMsg *msg );
};

extern EvDispatch *msgq;
#endif

