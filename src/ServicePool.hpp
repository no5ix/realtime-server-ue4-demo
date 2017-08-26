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
// class DbReconnection;
// class DbFormulaCombine;
// class DbGetUniverseItem;
// class DbGongfaSystem;
// class DbPvpCounterAtk;
// class DbPvpData;
// class DbPvpFight;
// class DbPvpFightRecord;
class DbPvpFightResult;
class DbPvpGetRival;
class DbPvpGetUniverseRival;
// class DbPvpPlunder;
// class DbPvpSpy;
// class DbPvpTeam;
// class DbPvpTreasure;
// class DbRanking;
// class DbSkillLevelUp;

class ServicePool : public boost::enable_shared_from_this<ServicePool>
{
public:
    ServicePool();
    virtual ~ServicePool();
    int stop();
    int start();

    boost::shared_ptr<DbTest>                 _test;
    // boost::shared_ptr<DbReconnection>         _reconnection;
    // boost::shared_ptr<DbFormulaCombine>       _formula_combine;
    // boost::shared_ptr<DbGetUniverseItem>      _get_universe_item;
    // boost::shared_ptr<DbGongfaSystem>         _gongfa_system;
    // boost::shared_ptr<DbPvpCounterAtk>        _pvp_counter_atk;
    // boost::shared_ptr<DbPvpData>              _pvp_data;
    // boost::shared_ptr<DbPvpFight>             _pvp_fight;
    // boost::shared_ptr<DbPvpFightRecord>       _pvp_fight_record;
    boost::shared_ptr<DbPvpFightResult>       _pvp_fight_result;
    boost::shared_ptr<DbPvpGetRival>          _pvp_get_rival;
    boost::shared_ptr<DbPvpGetUniverseRival>  _pvp_get_universe_rival;
    // boost::shared_ptr<DbPvpPlunder>           _pvp_plunder;
    // boost::shared_ptr<DbPvpSpy>               _pvp_spy;
    // boost::shared_ptr<DbPvpTeam>              _pvp_team;
    // boost::shared_ptr<DbPvpTreasure>          _pvp_treasure;
    // boost::shared_ptr<DbRanking>              _ranking;
    // boost::shared_ptr<DbSkillLevelUp>         _skill_levelup;
};


class DbGetData;
class DbModifyData;
class ConnectionPool : public boost::enable_shared_from_this<ConnectionPool>
{
    int _db_id;
public:
    ConnectionPool(const int db_id);
    virtual ~ConnectionPool();
    int start();
    int stop();

    boost::shared_ptr<DbGetData>  _get_data;
    boost::shared_ptr<DbModifyData>  _modify_data;
};

extern boost::shared_ptr<ServicePool> g_spool; //PVP Server的所有数据库操作池
extern map< int, boost::shared_ptr<ConnectionPool> > g_m_cpool; //所有远程数据库的操作池
} /* namespace bus */
#endif /* SERVICEPOOL_HPP_ */
