/**
 * @file gameproto.h
 * @auther Ge Tong
 */
#pragma once
#ifndef GAMEPROTO_H
#define GAMEPROTO_H
#include <stdint.h>
#include <string>
#include <streambuf>
#include <unordered_map>

#include "tcpprotocol.h"

#define PROTO_APP_FIRST_TAG 4;

typedef std::unordered_map<std::string, std::string> HashMap;

struct membuf: std::streambuf {
    membuf( char *begin, char *end ) {
        this->setg( begin, begin, end );
    }
};

class FromMsg
{
    public:
        HashMap attributes;
        std::string errorMsg = "";
        char fromVersion = 1;
        uint32_t msgCookie = 0;
        uint32_t resultCode = 1000;
        std::string serviceCmd;
        uint32_t packSeq = -1;
        std::string uin;
        std::string netBuffer;
        char packet_type = 0;
        ClientConnS &client;
    public:
        FromMsg( ClientConnS &client ): client( client ) {}
        ~FromMsg() {}
        void ParsePacket( const NetPacket &packethead, const char *packetdata );
};

class ToMsg
{
    public:
        HashMap attributes;
        std::string errorMsg = "";
        char toVersion = 1;
        uint32_t msgCookie;
        uint32_t resultCode;
        std::string serviceCmd;
        uint32_t packSeq = -1;
        std::string uin;
        std::string netBuffer;
        char packet_type = 0;
        uint32_t Timeout;
        ClientConnS &client;
    public:
        ToMsg();
        ~ToMsg();
};

class GameTCPProtocol : public TCPServerProtocolProcess
{
    public:
        GameTCPProtocol() {}
        virtual ~GameTCPProtocol() {}
        void ParsePacket( ClientConnS &client, const NetPacket &packet, char *buf );
        bool sendMsg( ToMsg &msg );
    private:
        bool bufFromMsg( const NetPacket &packet, char *buf, FromMsg &msg );
};
#endif
