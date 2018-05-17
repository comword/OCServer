/**
 * @file FromMsg.h
 * @auther Ge Tong
 */
#pragma once
#ifndef FROMMSG_H
#define FROMMSG_H
#include <string>
#include <unordered_map>
#include "net_base.h"
#include "uv_net.h"

typedef std::unordered_map<std::string, std::string> HashMap;

class FromMsg
{
    private:
        HashMap attributes;
        std::string errorMsg = "";
        char fromVersion = 1;
        int msgCookie = 0;
        int resultCode = 1000;
        std::string serviceCmd;
        int packSeq = -1;
        std::string uin;
        std::string netBuffer;
        char packet_type = 0;
        ClientConnS *client;
    public:
        FromMsg( ClientConnS *client ): client( client ) {}
        ~FromMsg() {}
        void ParsePacket( const NetPacket &packethead, const char *packetdata );
};
#endif
