#include "util.h"

std::string hexbyte(char c)
{
    uint8_t ab = (uint8_t) c;
    uint8_t a = ab >> 4;
    uint8_t b = ab & 0x0F;
    std::string ret;
	if (a < 10) ret += '0'+a;
	else ret += 'A'-10+a;
	if (b < 10) ret += '0'+b;
	else ret += 'A'-10+b;
    return ret+" ";
}

