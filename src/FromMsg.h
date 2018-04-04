
#include <string>
#include <vector>
#include <unordered_map>

typedef std::vector<uint8_t> Buffer_v;
typedef std::unordered_map<std::string, std::string> HashMap;

class FromMsg {
	HashMap attributes;
	std::string errorMsg = "";
	char fromVersion = 1;
	int msgCookie;
	int resultCode = 1001;
	std::string serviceCmd;
	int packSeq = -1;
	std::string uin;
	Buffer_v netBuffer;
	unsigned int length = 0;
};
