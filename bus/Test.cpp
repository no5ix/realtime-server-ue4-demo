/*
 * Test.cpp
 *
 */

#include "Test.hpp"
//#include "DGetData.hpp"

namespace bus
{
BusTest::BusTest() {}
BusTest::~BusTest() {}
int BusTest::process(Process * tools)
{
	VLOG(L_DB) << "处理 Test";
	_tools = tools;
	int ret = checkQryParam();
	if (ret)
		goto ERROR_PARAM;
	printQry();
	ret = g_spool->_test->getData(_qry, _ack);
	if (ret)
		goto ERROR_PARAM;
	_ack.put("ret", ret);
	printAck();
	VLOG(L_DB) << "处理 Test 完毕 ";
	_tools->deal_done();
	return 0;

ERROR_PARAM:
	_ack.put("ret", ret);
	VLOG(L_ER) << "处理 Test 失败 ,RET_CODE: " << ret;
	_tools->deal_done();
	return -1;
}

int BusTest::checkQryParam()
{
	if (_qry.get<int>("player_id", -1) < 1)
		return RET_PARAM_ERR;
	return RET_NORMAL;
}

int DbTest::getData(Json &_qry, Json &_ack)
{
	boost::unique_lock<boost::shared_mutex> lock(_mutex);
	        
	int player_id = _qry.get<int>("player_id");
	
	sprintf(_sql, "SELECT player_id, age, sex FROM player_data WHERE player_id = %d", player_id);
	if (0 != query())
		return RET_DB_QUERY_ERR;

	clog << "player_id = " << player_id << endl;

	if (NULL != (_row = mysql_fetch_row(_result)))
	{
		_ack.put("player_id", ATOI(_row[0]));
		_ack.put("age", ATOI(_row[1]));
		_ack.put("sex", _row[2]);
	}
	else
	{
		_ack.put("player_id", -1);
		_ack.put("age", -1);
		_ack.put("sex", "");
	}
	mysql_free_result(_result);

	return RET_NORMAL;
}


}/* namespace bus */
