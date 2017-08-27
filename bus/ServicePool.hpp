/*
 * ServicePool.hpp
 *
 */

#ifndef SERVICEPOOL_HPP_
#define SERVICEPOOL_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/random.hpp>

#include "../src/Pre.h"
#include "DbService.hpp"
#include "Process.h"

namespace bus {

class DbTest;

class ServicePool : public boost::enable_shared_from_this<ServicePool>
{
public:
    ServicePool();
    virtual ~ServicePool();
    int stop();
    int start();

    boost::shared_ptr<DbTest>                 _test;
};


//class DbGetData;
//class DbModifyData;
class ConnectionPool : public boost::enable_shared_from_this<ConnectionPool>
{
    int _db_id;
public:
    ConnectionPool(const int db_id);
    virtual ~ConnectionPool();
    int start();
    int stop();

    //boost::shared_ptr<DbGetData>  _get_data;
    //boost::shared_ptr<DbModifyData>  _modify_data;
};

extern boost::shared_ptr<ServicePool> g_spool; // Server的所有数据库操作池
extern map< int, boost::shared_ptr<ConnectionPool> > g_m_cpool; //所有远程数据库的操作池
} /* namespace bus */
#endif /* SERVICEPOOL_HPP_ */
