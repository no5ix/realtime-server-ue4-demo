/*
 * Heartbeat.cpp
 *
 */

#include <glog/logging.h>
#include "Heartbeat.hpp"
#include "Process.h"

namespace bus {

BusHeartbeat::BusHeartbeat() {}

BusHeartbeat::~BusHeartbeat() {}

// 业务逻辑的处理代码
int BusHeartbeat::process(Process* pro)
{
    VLOG(L_DB) << "HEARTBEAT start";
    _tools = pro;

    _ack.put("ret", 0);
    VLOG(L_DB) << "HEARTBEAT finish";
    _tools->deal_done();
    return 0;
}

} /* namespace bus */
