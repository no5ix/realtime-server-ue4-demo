/*
 * Heartbeat.hpp
 *
 */

#ifndef HEARTBEAT_HPP_
#define HEARTBEAT_HPP_

#include "Business.hpp"
#include "ServicePool.hpp"
#include "../src/Command.hpp"
namespace bus {

class BusHeartbeat : public Business<Json, Json>
{
public:
    BusHeartbeat();
    virtual ~BusHeartbeat();

    virtual int process(Process *tools);
private:
    int checkQryParam();
};

} /* namespace bus */
#endif /* HEARTBEAT_HPP_ */
