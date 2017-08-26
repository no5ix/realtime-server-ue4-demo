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
// #include "FormulaCombine.hpp"
// #include "GetUniverseItem.hpp"
// #include "GongfaSystem.hpp"
// #include "PvpCounterAtk.hpp"
// #include "PvpData.hpp"
// #include "PvpFight.hpp"
// #include "PvpFightRecord.hpp"
#include "PvpFightResult.hpp"
#include "PvpGetRival.hpp"
#include "PvpGetUniverseRival.hpp"
// #include "PvpPlunder.hpp"
// #include "PvpSpy.hpp"
// #include "PvpTeam.hpp"
// #include "PvpTreasure.hpp"
// #include "Ranking.hpp"
// #include "SkillLevelUp.hpp"

#include "DGetData.hpp"
#include "DModifyData.hpp"

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

    // _formula_combine        = boost::make_shared<DbFormulaCombine>(&g_db);
    // _get_universe_item      = boost::make_shared<DbGetUniverseItem>(&g_db);
    // _gongfa_system          = boost::make_shared<DbGongfaSystem>(&g_db);
    // _pvp_counter_atk        = boost::make_shared<DbPvpCounterAtk>(&g_db);
    // _pvp_data               = boost::make_shared<DbPvpData>(&g_db);
    // _pvp_fight              = boost::make_shared<DbPvpFight>(&g_db);
    // _pvp_fight_record       = boost::make_shared<DbPvpFightRecord>(&g_db);
    _pvp_fight_result       = boost::make_shared<DbPvpFightResult>(&g_db);
    _pvp_get_rival          = boost::make_shared<DbPvpGetRival>(&g_db);
    _pvp_get_universe_rival = boost::make_shared<DbPvpGetUniverseRival>(&g_db);
    // _pvp_plunder            = boost::make_shared<DbPvpPlunder>(&g_db);
    // _pvp_spy                = boost::make_shared<DbPvpSpy>(&g_db);
    // _pvp_team               = boost::make_shared<DbPvpTeam>(&g_db);
    // _pvp_treasure           = boost::make_shared<DbPvpTreasure>(&g_db);
    // _ranking                = boost::make_shared<DbRanking>(&g_db);
    // _reconnection           = boost::make_shared<DbReconnection>(&g_db);
    // _skill_levelup          = boost::make_shared<DbSkillLevelUp>(&g_db);

	return 0;
}

int ServicePool::stop()
{
    _test->stop();

    // _formula_combine->stop();
    // _get_universe_item->stop();
    // _gongfa_system->stop();
    // _pvp_counter_atk->stop();
    // _pvp_data->stop();
    // _pvp_fight->stop();
    // _pvp_fight_record->stop();
    _pvp_fight_result->stop();
    _pvp_get_rival->stop();
    _pvp_get_universe_rival->stop();
    // _pvp_plunder->stop();
    // _pvp_spy->stop();
    // _pvp_team->stop();
    // _pvp_treasure->stop();
    // _ranking->stop();
    // _reconnection->stop();
    // _skill_levelup->stop();

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
    _get_data = boost::make_shared<DbGetData>(&g_v_db[_db_id]);
    _modify_data = boost::make_shared<DbModifyData>(&g_v_db[_db_id]);
	return 0;
}

int ConnectionPool::stop()
{
    _get_data->stop();
    _modify_data->stop();
	return 0;
}

} /* namespace bus */
