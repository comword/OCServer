#include <string>
#include <vector>

class util
{
    public:
        static void int8_to_buf( char *buf, int i, int value );
        static void int16_to_buf( char *buf, int i, int value );
        static void int32_to_buf( char *buf, int i, int value );
        static void int64_to_buf( char *buf, int i, uint64_t value );
        static int String2Buffer( char *src, int srclen, char *dest );
        static int Buffer2String( char *src, int srclen, char *dest );
        static int Buffer2String( char *src, int srclen, std::string &dest );
        static int String2Buffer( std::string src, std::string &dest );
        static int Buffer2String( std::string src, std::string &dest );
        static int String2Buffer( std::string src, std::vector<char> &dest );
        static int Buffer2String( std::vector<char> src, std::string &dest );
};
