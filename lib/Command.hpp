

#ifndef __COMMAND_HPP__
#define __COMMAND_HPP__

#include "Protocol.h"

enum {
	HEART_BEAT                   = 33, // 心跳
	HEART_BEAT_ACK               = 34,

	TEST                       = 67,
	TEST_ACK                   = 68,

	MAX_PROTOCOL_CMD
};


#endif /* __COMMAND_HPP__*/
