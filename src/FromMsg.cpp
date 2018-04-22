#include "FromMsg.h"

FromMsg::FromMsg( size_t bufsize )
{
    netBuffer.base = ( char * )malloc( bufsize );
    netBuffer.len = bufsize;
}

FromMsg::~FromMsg()
{
    free( netBuffer.base );
}
