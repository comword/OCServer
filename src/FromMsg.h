#pragma once
#ifndef FROMMSG_H
#define FROMMSG_H
#include <string>
#include <unordered_map>

#include <uv.h>

typedef std::unordered_map<std::string, std::string> HashMap;

class FromMsg
{
    private:
        HashMap attributes;
        std::string errorMsg = "";
        char fromVersion = 1;
        int msgCookie;
        int resultCode = 1001;
        std::string serviceCmd;
        int packSeq = -1;
        std::string uin;
        uv_buf_t netBuffer;
        //size_t length = 0;
    public:
        FromMsg( size_t bufsize );
        ~FromMsg();
};
#endif
