/*
 * DbService.hpp
 *
 */

#ifndef DBSERVICE_HPP_
#define DBSERVICE_HPP_

#include <mysql.h>

#include "../src/Pre.h"
#include "Business.hpp"

struct DB
{
    string server;
    string db;
    string user;
    string passwd;
    int realm_id;
    int port;
    int timeout;
    int interactive_timeout;
};
extern DB g_db;
extern vector<DB> g_v_db; //存储所有数据库连接信息

namespace bus
{
//封装数据库的相关操作
//提供数据库的链接，查询，返回结果等
class DbService
{
public:
    DbService(DB * db);
    ~DbService(){ stop(); }
    int stop();


protected:
    int get_db();
    int db_connect(MYSQL *connect, const char* server, const char* user,
                const char* passwd, const char* db, unsigned int port);
    int db_heartbeat(MYSQL *connect);
    int db_close();
    int db_close(MYSQL *connect);
    int db_free_all_res();

    /**
     * 执行sql查询语句
     * 本方法将使用_connect, _sql, _result成员，且未对上述成员加锁，请在调用该方法之前加锁
     * @return 0 执行成功 -1 执行失败
     */
    int query();

    /**
     * 执行在成员 _sql 中定义的insert语句
     * 本方法将使用_connect, _sql, 且未对上述成员加锁
     * 请在调用该方法之前自行加锁
     * @return 0 执行成功 -1 执行失败
     */
    int insert();

    /**
     * 执行在成员 _sql 中定义的update语句
     * 本方法将使用_connect, _sql, 且未对上述成员加锁
     * 请在调用该方法之前自行加锁
     * @return 0 执行成功 -1 执行失败
     */
    int update();

    /**
     * 执行在成员 _sql 中定义的delete语句
     * 本方法将使用_connect, _sql, 且未对上述成员加锁
     * 请在调用该方法之前自行加锁
     * @return 0 执行成功 -1 执行失败
     */
    int remove();

    /**
     * 执行在成员 _sql 中定义的存储过程
     * 本方法将使用_connect, _sql, 且未对上述成员加锁
     * 请在调用该方法之前自行加锁
     * @return 0 执行成功 -1 执行失败
     */
    int exec_proc();

    /**
     * 执行在成员 _sql 中定义的存储过程结果查询语句
     * 本方法将使用_connect, _sql, 且未对上述成员加锁
     * 请在调用该方法之前自行加锁
     * @return 0 执行成功 -1 执行失败
     */
    int query_proc_result();

    int start_transaction();

    int rollback();

    int commit();

protected:
    MYSQL _connect; ///< 当前在用的mysql连接 连接的句柄
    timeval _time_lastdb; ///< 最近一次的数据操作时间
    MYSQL_RES * _result; ///< mysql结果集
    MYSQL_ROW _row; ///< mysql行
    char _sql[102400]; ///< 存放sql语句
    boost::shared_mutex _mutex; // 互斥锁
    DB _db;//保存当前connection信息
};

} /* namespace bus */

#endif /* DBSERVICE_HPP_ */