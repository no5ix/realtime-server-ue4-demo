/*
 * Bussines.h
 *
 */

#ifndef BUSINESS_H_
#define BUSINESS_H_
#define BOOST_DATE_TIME_SOURCE
//#define BOOST_DATE_TIME_NO_LIB
#include <mysql.h>
#include <string.h>
#include <glog/logging.h>
#include "../src/Code.h"
#include "../src/profile_config.h"
#include "../src/Json.hpp"


#include <boost/date_time/posix_time/posix_time.hpp>

using namespace bas;

namespace bus {

const int base_honor[] = { 100, 200, 250, 300, 350, 400 }; //各段位基础荣誉值
const float beauty_honor_add[] = { 0.05, 0.09, 0.13, 0.17, 0.21, 0.25, 0.3 };//妖姬加成  TODO改为读表
const int drum_id = 34023; //战鼓道具ID
const int mirror_id = 34020; //昆仑镜道具ID
const int treasure_id = 36000; //稀世珍宝道具ID

const bool b_log_ingot = true;//CONF.get<bool>("log.ingot"); //是否记录元宝变更

enum RET_CODE { //定义各种返回值
	RET_NORMAL				= 0,	//正常
	RET_DB_QUERY_ERR		= 10,	//数据库查询异常
	RET_DB_UPDATE_ERR,				//数据库更新异常
	RET_DB_INSERT_ERR,				//数据库插入异常
	RET_DB_DELETE_ERR,				//数据库删除异常
	RET_DB_EXEC_PROC_ERR,			//执行存储过程异常
	RET_DB_QUERY_PROC_ERR,			//执行存储过程异常
	RET_DB_NULL_ROW,				//数据库查询返回空行
	RET_DB_MULTI_ROW,				//数据库查询返回多行
	RET_EXISTS				= 20,	//存在
	RET_NOT_EXISTS,					//不存在
	RET_EXISTS_MULTI,				//存在多个
	RET_PARAM_ERR			= 30,	//发送包参数错误
	RET_CSERVER_ERR,				//客户端通信异常
	RET_PROTOCOL_ERR,				//协议操作异常
	RET_UNKNOWN_REALM,              //未知的分区
	RET_CHECK_ERR			= 40,	//服务器端数据与客户端数据校验失败

	RET_LOGIN_OTHER         = 60,   //别处登录
	RET_RECONN_PROTOCOL     = 61,   //同一个协议同时登录
	RET_CHECK_ERR_RELOGIN   = 62,   //资源校验失败,客户端要求重新启动
	
	RET_CDKEY_ERR           = 70,   //CDKEY不合法
	RET_CDKEY_ACTIVATED     = 71,   //CDKEY已激活
	RET_CDKEY_ACTIVATED_PRE = 72,   //已激活过同类型的CDKEY
	
	RET_BOSS_TIME_INVALID   = 80,   //世界boss时间未到


	RET_MAX_CODE
};
// 奖励事件对应的事件类型 对应策划表svn目录：14_福利系统/事件类型表
// 调用函数如下函数来更新事件的参数 这里的事件参数分为叠加的和直接更新的 比如：次数+1 战斗力更新为500：
// 特别注意：这里一些次数是直接累加的 所以函数调用者需要确定事件成功了和不重复提交
// int  updateEventTimes(int player_id, int event_type, int event_args);

enum EVENT_TYPE
{
	
	TEST_EVENT          = 1333, // 每日观想次数
};
       
class Process;
class BusInf
{
    public:
        virtual void* getQry() = 0;
        virtual void* getAck() = 0;
        virtual int process(Process*) = 0;
        virtual void db_done(const int, void *);
        virtual ~BusInf(){};
        BusInf():_tools(0){};
        void peer_addr(const char *ip, const int port){
            if (ip == 0) return ;
            strncpy(_peer_ip, ip, sizeof(_peer_ip));
            _peer_port = port;
        }
        void cmd(int command){
            _cmd = command;
        }

        Process *_tools;

    protected:
        char _peer_ip[16];
        int  _peer_port;
        int  _cmd;
};

template<typename Qry, typename Ack>
class Business : public BusInf
{
    public:
        virtual void* getQry() {return &_qry;}
        virtual void* getAck() {return &_ack;}
        virtual int process(Process*){return 0;}

        virtual ~Business(){
            std::string strTime = boost::posix_time::to_iso_string(
                    boost::posix_time::second_clock::local_time());
            _js << Pair("log_time", strTime) <<
                    Pair("ip", _peer_ip) <<
                    Pair("port", _peer_port) <<
                    Pair("cmd", _cmd) << 
                    Pair("sub_cmd_id", 0); //default sub_cmd_id
            VLOG(L_RP) << _js.Str();
        }
        Business(){}
        void printQry() { VLOG(L_DT) << "Qry Body >>>>> " << _qry.Str(); }
        void printAck() { VLOG(L_DT) << "Ack Body >>>>> " << _ack.Str(); }

    protected:
        Qry _qry;
        Ack _ack;
        Json _js;
};

} /* namespace BUS */
#endif /* BUSINESS_H_ */
