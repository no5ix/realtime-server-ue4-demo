/*
 * Test.hpp
 *
 */

#ifndef TEST_HPP_
#define TEST_HPP_


#include "Business.hpp"
#include "ServicePool.hpp"
#include "../src/Command.hpp"
#include "Process.h"

namespace bus
{

class BusTest: public Business<Json, Json>
{

public:
	BusTest();
	virtual ~BusTest();
	virtual int process(Process *tools);

private:
	int checkQryParam();
};

class DbTest: public DbService
{
public:
	DbTest(DB * db) : DbService(db) {}

	int getData(Json &_qry, Json &_ack);
};

} /* namespace bus */


#endif /* TEST_HPP_ */
