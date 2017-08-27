/*
 * ServicePool.cpp
 *
 */
#include <glog/logging.h>

#include "../src/profile_config.h"
#include "ServicePool.hpp"
#include "DbService.hpp"

// #include "Reconnection.hpp"
#include "Test.hpp"

//#include "DGetData.hpp"
//#include "DModifyData.hpp"

DB g_db;
vector<DB> g_v_db;

namespace bus
{
boost::shared_ptr<ServicePool> g_spool; //PVP Server的所有数据库操作池
map< int, boost::shared_ptr<ConnectionPool> > g_m_cpool; //所有远程数据库的操作池

ServicePool::ServicePool()
{
}

ServicePool::~ServicePool()
{
	// TODO Auto-generated destructor stub
}

int ServicePool::start()
{
	VLOG(200) << "ServicePool: " << boost::this_thread::get_id();

    _test                   = boost::make_shared<DbTest>(&g_db);

	return 0;
}

int ServicePool::stop()
{
    _test->stop();


	return 0;
}

ConnectionPool::ConnectionPool(const int db_id)
{
	_db_id = db_id;
}

ConnectionPool::~ConnectionPool()
{
}

int ConnectionPool::start()
{
	VLOG(200) << "ConnectionPool " << _db_id << " : " << boost::this_thread::get_id();
    //_get_data = boost::make_shared<DbGetData>(&g_v_db[_db_id]);
    //_modify_data = boost::make_shared<DbModifyData>(&g_v_db[_db_id]);
	return 0;
}

int ConnectionPool::stop()
{
    //_get_data->stop();
    //_modify_data->stop();
	return 0;
}

} /* namespace bus */
