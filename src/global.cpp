#include "base.h"
#include "global.h"


bool SD_card_OK;
int SD_file_No;

uint32_t FC_time_now()
{
    return FC_systick_millis_count;
}

uint32_t FC_elapsed_millis(uint32_t timestamp)
{
    uint32_t now = FC_systick_millis_count;
    if (now>=timestamp)
        return now - timestamp;
    else
        // wrap around
        return (0xFFFFFFFF - timestamp) + now + 1;
}

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

