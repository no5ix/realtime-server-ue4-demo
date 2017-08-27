/*
 * ServicePool.cpp
 *
 */

#include <glog/logging.h>
#include "../src/profile_config.h"
#include "../src/Pre.h"
#include "Business.hpp"
#include "ServicePool.hpp"
#include "DbService.hpp"

namespace bus
{

DbService::DbService(DB * db) : _result(0), _row(0)
{
	_connect.thread_id = 0;
	_db = *db;
}

int DbService::db_free_all_res()
{
	do {
		MYSQL_RES * res = mysql_store_result(&_connect);
		if (res) mysql_free_result(res);
	} while (!mysql_next_result(&_connect));
	return 0;
}

int DbService::get_db()
{
	int ret = 0;
	if (_connect.thread_id != 0) { ///如果是已有连接
		if (mysql_errno(&_connect))
			db_close();else {
			timeval t_now;
			gettimeofday(&t_now, 0);
			if (t_now.tv_sec - _time_lastdb.tv_sec >= 30)  ///距最近操作时间大于30秒,则检查心跳
				ret = db_heartbeat(&_connect);
			if (ret == 0) {
				_time_lastdb = t_now;
				return 0;
			} else
				db_close();
		}
	}

	if (0 == db_connect(&_connect, _db.server.c_str(), _db.user.c_str(), _db.passwd.c_str(), _db.db.c_str(), (unsigned int)_db.port)) {
		gettimeofday(&_time_lastdb, 0);
		return 0;
	} else {
		VLOG(1) << "connect to mysql failed: "
				<< mysql_error(&_connect);
		db_close();
	}

	return -1;
}

int DbService::db_close(MYSQL * connect)
{
	db_free_all_res();
	mysql_close(connect);
	mysql_thread_end();
	connect->thread_id = 0;
	return 0;
}

int DbService::db_close()
{
	db_close(&_connect);
	return 0;
}

int DbService::db_heartbeat(MYSQL * connect)
{
	int ret = (mysql_ping(connect) == 0) ? 0 : -1;
	if (ret != 0)
		VLOG(100) << mysql_error(connect);
	return ret;
}

int DbService::stop()
{
	///关闭 mysql
	if (_connect.thread_id != 0) {
		VLOG(100) << "stop mysql: " << _connect.thread_id;
		db_close();
	}

	return 0;
}

int DbService::db_connect(MYSQL * connect, const char * server, const char * user,
						  const char * passwd, const char * db, unsigned int port)
{
	int ret = -1;

	mysql_init(connect); // init

	char tmpset[255] = "";
	int timeout = _db.timeout;
	sprintf(tmpset, "set wait_timeout=%d", timeout);
	mysql_options(connect, MYSQL_INIT_COMMAND, tmpset);
	sprintf(tmpset, "set interactive_timeout=%d", _db.interactive_timeout);
	mysql_options(connect, MYSQL_INIT_COMMAND, tmpset);
	bool auto_cnt = true;
	mysql_options(connect, MYSQL_OPT_RECONNECT, &auto_cnt);
	mysql_options(connect, MYSQL_OPT_READ_TIMEOUT, &timeout);
	mysql_options(connect, MYSQL_OPT_WRITE_TIMEOUT, &timeout);

	if (0 == mysql_real_connect(connect, server, //连接数据库 mysql_real_connect
								user, passwd, db, port, NULL, CLIENT_FOUND_ROWS))

	{
		VLOG(1) << "connect to mysql failed: " << mysql_error(connect);
		return ret = -1;
	}
	VLOG(100) << "mysql id: " << connect->thread_id ;

	if (mysql_set_character_set(connect, "utf8"))
		VLOG(1) << "client character set: " << mysql_character_set_name(connect);
	//打开自动提交
	if (0 != mysql_autocommit(connect, 1)) {
		VLOG(1) << "turn off auto commit failed: "
				<< mysql_error(connect);
		db_close();
		return ret = -1;
	}
	return ret = 0;
}

int DbService::query()
{
	///获取数据库连接
	if (0 != get_db()) {
		VLOG(L_ER) << "get db connection falied!";
		return -1;
	}
	VLOG(L_DT) << "SQL : " << _sql;
	///执行sql查询
	if (0 != mysql_query(&_connect, _sql)) {
		VLOG(L_ER) << _sql;
		VLOG(L_ER) << "query sql exception: " << mysql_error(&_connect);
		return -1;
	}

	///保存查询结果
	if (NULL == (_result = mysql_store_result(&_connect))) {
		VLOG(L_ER) << _sql;
		VLOG(L_ER) << "query result exception: " << mysql_error(&_connect);
		mysql_free_result(_result);
		return -1;
	}

	return 0;
}

int DbService::insert()
{
	if (0 != get_db()) {
		VLOG(L_ER) << "get db connection falied!";
		return -1;
	}

	if (0 != mysql_query(&_connect, _sql)) {
		VLOG(L_ER) << _sql;
		VLOG(L_ER) << "execute sql exception: " << mysql_error(&_connect);
		return -1;
	}

	return 0;
}

int DbService::update()
{
	return insert();
}

int DbService::remove()
{
	return insert();
}

int DbService::exec_proc()
{
	if (0 != get_db()) {
		VLOG(L_ER) << "get db connection falied!";
		return -1;
	}

	if (0 != mysql_query(&_connect, _sql)) {
		VLOG(L_ER) << _sql;
		VLOG(L_ER) << "procedure exec exception: " << mysql_error(&_connect);
		return -1;
	}

	return 0;
}

int DbService::query_proc_result()
{
	if (0 != get_db()) {
		VLOG(L_ER) << "get db connection falied!";
		return -1;
	}

	if (0 != mysql_query(&_connect, _sql)) {
		VLOG(L_ER) << _sql;
		VLOG(L_ER) << "query proc result failed: " << mysql_error(&_connect);
		return -1;
	}

	if (NULL == (_result = mysql_store_result(&_connect))) {
		VLOG(L_ER) << "query result exception: " << mysql_error(&_connect);
		mysql_free_result(_result);
		return -1;
	}

	return 0;
}

int DbService::start_transaction()
{
	if (0 != get_db()) {
		VLOG(L_ER) << "get db connection falied!";
		return -1;
	}

	if (0 != mysql_query(&_connect, "START TRANSACTION")) {
		VLOG(L_ER) << "failed to start transaction: " << mysql_error(&_connect);
		return -1;
	}

	return 0;
}

int DbService::rollback()
{
	if (0 != get_db()) {
		VLOG(L_ER) << "get db connection falied!";
		return -1;
	}

	if (0 != mysql_query(&_connect, "ROLLBACK")) {
		VLOG(L_ER) << "failed to rollback: " << mysql_error(&_connect);
		return -1;
	}

	return 0;
}

int DbService::commit()
{
	if (0 != get_db()) {
		VLOG(L_ER) << "get db connection falied!";
		return -1;
	}

	if (0 != mysql_query(&_connect, "COMMIT")) {
		VLOG(L_ER) << "failed to commit: " << mysql_error(&_connect);
		rollback();
		return -1;
	}

	return 0;
}