#include "ToMsg.h"

ToMsg::ToMsg( size_t bufsize )
{
    netBuffer.base = ( char * )malloc( bufsize );
    netBuffer.len = bufsize;
}

ToMsg::~ToMsg()
{
    free( netBuffer.base );
}
