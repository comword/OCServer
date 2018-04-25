#pragma once
#ifndef TOMSG_H
#define TOMSG_H
#include <unordered_map>
#include <string>

#include <uv.h>

typedef std::unordered_map<std::string, std::string> HashMap;

class ToMsg
{
    private:
        HashMap attributes;
        std::string errorMsg = "";
        char toVersion = 1;
        int msgCookie;
        int resultCode;
        std::string serviceCmd;
        int packSeq = -1;
        std::string uin;
        std::string netBuffer;
        long Timeout;
    public:
        ToMsg( size_t bufsize );
        ~ToMsg();
};
#endif
