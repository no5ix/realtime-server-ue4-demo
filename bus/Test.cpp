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
	if (_qry.get<int>("pvp_id", -1) < 1)
		return RET_PARAM_ERR;
	return RET_NORMAL;
}

int DbTest::getData(Json &_qry, Json &_ack)
{
	boost::unique_lock<boost::shared_mutex> lock(_mutex);
	        
	int pvp_id = _qry.get<int>("pvp_id");
	
	sprintf(_sql, "SELECT rank_level, player_id, realm_id FROM pvp WHERE pvp_id = %d", pvp_id);
	if (0 != query())
		return RET_DB_QUERY_ERR;
	int rank_level = -1;
	int realm_id = -1;
	int player_id = -1;

	if (NULL != (_row = mysql_fetch_row(_result)))
	{
		rank_level = ATOI(_row[0]);
		player_id = ATOI(_row[1]);
		realm_id = ATOI(_row[2]);
	}
	mysql_free_result(_result);

	if (!g_m_cpool.count(realm_id)) return -1;
	return RET_NORMAL;
}


}/* namespace bus */
