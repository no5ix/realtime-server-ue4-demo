/**
 *  @file Main.cpp
 *
 */
//#include <my_sys.h>
#include <glog/logging.h>
#include <mysql.h>
#include <sys/types.h>
#include <signal.h>
//#include <boost/serialization/singleton.hpp>
//#include <boost/pool/detail/singleton.hpp>
//#include <boost/serialization/extended_type_info_typeid.hpp>
//#include <boost/random.hpp>
#include "Main.h"
#include "../bus/Process.h"
#include "../bus/ServicePool.hpp"
#include "../bus/DbService.hpp"

using namespace bas;
using namespace bus;
using std::clog;
using std::string;
//using namespace std;
//using namespace boost::asio;
//using namespace boost::serialization;
//using boost::details::pool;

#ifndef OPENING_TIME
#define OPENING_TIME (__DATE__ __TIME__)
#endif

#ifndef TEST_TIME
#define TEST_TIME (__DATE__ __TIME__)
#endif

#ifndef PROGRAM_VERSION
#define PROGRAM_VERSION  (__DATE__ __TIME__)
#endif

int* g_argc = 0;
char** g_argv = 0;
bool g_exit = false;

int g_realm_cnt = 0;

/**
 * @brief 加载配置
 * @param reload 是否为重载,默认为否
 * @return OK:成功
 */
int load_conf(bool reload)
{
    /// 检查命令行参数
    CONF.parse(*g_argc, g_argv);

    /// 参数为help则显示帮助后结束
    if (CONF.count("help"))
    {
        CONF.help();
        exit(OK);
    }

    /// 参数为version则显示版本信息后结束
    if (CONF.count("version"))
    {
        clog << "OPENING_TIME: " << OPENING_TIME << endl;
        clog << "TEST_IME: " << TEST_TIME << endl;
        clog << "PROGRAM_VERSION: " << PROGRAM_VERSION << endl;
        exit(OK);
    }

    ///初始化全局变量
    g_db.server = CONF.get<string>("mysql.server");
    g_db.port = CONF.get<int>("mysql.port");
    g_db.db = CONF.get<string>("mysql.db");
    g_db.user = CONF.get<string>("mysql.user");
    g_db.passwd = CONF.get<string>("mysql.passwd");
    g_db.timeout = CONF.get<int>("mysql.wait_timeout");
    g_db.interactive_timeout = CONF.get<int>("mysql.interactive_timeout");
    g_db.realm_id = 0;

    /// 读取GServer数据库配置信息
    if (set_dbs()) {
        clog << "GServer Database Error " << endl;
        exit(OK);
    }

    return OK;
}

/**
 * @brief 读取GServer数据库配置信息
 * @return
 */
int set_dbs()
{
    g_realm_cnt = CONF.get<int>("db.realm_cnt");
    vector<string> realm_ids;
    vector<string> servers;
    vector<string> ports;
    vector<string> dbs;
    vector<string> users;
    vector<string> passwds;
    
    int ret = parse_db_conf("db.realm_ids", &realm_ids);
    if (ret) {
        clog << "error count of realm_ids" << endl;
        return ret;
    }
    ret = parse_db_conf("db.servers", &servers);
    if (ret) {
        clog << "error count of servers" << endl;
        return ret;
    }
    ret = parse_db_conf("db.ports", &ports);
    if (ret) {
        clog << "error count of ports" << endl;
        return ret;
    }
    ret = parse_db_conf("db.dbs", &dbs);
    if (ret) {
        clog << "error count of dbs" << endl;
        return ret;
    }
    ret = parse_db_conf("db.users", &users);
    if (ret) {
        clog << "error count of users" << endl;
        return ret;
    }
    ret = parse_db_conf("db.passwds", &passwds);
    if (ret) {
        clog << "error count of passwds" << endl;
        return ret;
    }
    
    //clog << "realm_cnt = " << g_realm_cnt << endl;
    for (int i = 0; i < g_realm_cnt; ++i)
    {
        DB db_server;
        db_server.realm_id = ATOI(realm_ids[i].c_str());
        db_server.port = ATOI(ports[i].c_str());
        db_server.server = servers[i];
        db_server.db = dbs[i];
        db_server.user = users[i];
        db_server.passwd = passwds[i];
        db_server.timeout = CONF.get<int>("mysql.wait_timeout");
        db_server.interactive_timeout = CONF.get<int>("mysql.interactive_timeout");

        // clog << "-------------------------------------" << endl;
        // clog << "db_server.realm_id = " << db_server.realm_id << endl;
        // clog << "db_server.server   = " << db_server.server << endl;
        // clog << "db_server.port     = " << db_server.port << endl;
        // clog << "db_server.db       = " << db_server.db << endl;
        // clog << "db_server.user     = " << db_server.user << endl;
        // clog << "db_server.passwd   = " << db_server.passwd << endl;
        
        g_v_db.push_back(db_server);
    }
    clog << "-------------------------------------" << endl;
    return 0;
}

int parse_db_conf(const char * path, vector<string> * results)
{
    static const string f = ",";
    size_t pos = 0;
    size_t idx = 0;
    string conf = CONF.get<string>(path);
    while (std::string::npos != (idx = conf.find(f, pos))) {
        results->push_back(conf.substr(pos, idx-pos));
        pos = idx + 1;
    }
    results->push_back(conf.substr(pos));
    if (results->size() != g_realm_cnt) return -1;
    return 0;
}


/**
 * @brief 入口函数
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char* argv[])
{
    g_argc = &argc;
    g_argv = argv;

    /// 读取配置
    load_conf();

    /// @todo 配置正确性检查

    ///初始化glog,调试时可用GLOG_log_dir环境变量改变为FLAGS_logtostderr
    //FLAGS_max_log_size google::SetLogDestination
    FLAGS_log_dir = CONF.get<string> ("log.path");
    google::InitGoogleLogging(CONF.get<string> ("log.head").c_str());
    google::InstallFailureSignalHandler();
    FLAGS_v = CONF.get<int> ("log.level");
    FLAGS_alsologtostderr = CONF.get<bool> ("log.term");
    FLAGS_max_log_size = CONF.get<int> ("log.max_size");

    LOG(INFO)<< "JoyServer start...";
    /// 检查是否为daemon运行,并切换运行方式
    if (CONF.get<bool> ("srv.daemon"))
    {
        LOG(INFO) << "Run as daemon.";
        ::daemon(0, 0);
    }

    /// 初始化 mysql库
    if (mysql_library_init(-1, NULL, NULL))
    {
        VLOG(1) << "could not initialize MySQL library";
        exit(KO);
    }

#ifdef WIN32
    bool ret = SetConsoleCtrlHandler((PHANDLER_ROUTINE)signal_handle, true);
    if (ret == false)
    {
        std::clog << "Registery signal failed!" << std::endl;
        exit(KO);
    }
    boost::chrono::milliseconds dura(2000);
    while (!g_exit)
    {
        boost::this_thread::sleep_for(dura);
    }
#else
    signal_handle();
#endif
    LOG(INFO)<< "JoyServer finish." << std::endl;
    /// 停止glog
    google::ShutdownGoogleLogging();

    return OK;
}

void flush_log()
{
    google::FlushLogFiles(0);
}


#ifdef WIN32
/**
 * @brief 中断信号捕捉
 * @param dwCtrlType
 * @return OK:成功
 */
bool signal_handle(DWORD dwCtrlType)
{
    std::clog << "Recive signal...\r\n" << std::endl;

    switch (dwCtrlType)
    {
        case CTRL_C_EVENT: // ctrl + c
        case CTRL_BREAK_EVENT:// ctrl + break
        case CTRL_CLOSE_EVENT:// 关闭控制台
        std::clog << "Prepare to exit service!" << std::endl;
        g_exit = true;
        return OK;
        default:
        return KO;
    }
    return KO;
}

#else
bool signal_handle()
{
    /// 阻塞所有信号,不传递给新创建的线程
    sigset_t new_mask;
    sigfillset(&new_mask);
    sigset_t old_mask;
    pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);

    g_spool = boost::make_shared<bus::ServicePool>();
    g_spool->start();

    for (int i = 0; i < g_realm_cnt; ++i)
    {
        boost::shared_ptr<bus::ConnectionPool> cpool = boost::make_shared<bus::ConnectionPool>(i);
        g_m_cpool.insert(std::pair<int, boost::shared_ptr<bus::ConnectionPool> >(g_v_db[i].realm_id, cpool));
        cpool->start();
    }

    /// 初始化 server.
    GameServer s(CONF.get<string> ("srv.ip").c_str(),
    CONF.get<int> ("srv.port"),
    CONF.get<int> ("srv.ios"),
    CONF.get<int> ("srv.threads"),
    new server_handler_pool(new Worker_Allocator(),
                CONF.get<int> ("srv.handlers"),
                CONF.get<int> ("srv.buffer"),
                CONF.get<int> ("srv.buffer"),
                CONF.get<int> ("srv.timeout"),
                CONF.get<int> ("srv.wait")));

    GameServer *gs = (GameServer*) &s;

    boost::thread_group thr;

    /// 创建线程后台运行
    boost::thread_group* tg = (boost::thread_group*) &thr;
    tg->create_thread(boost::bind(&GameServer::run, gs));

    /// 恢复原来的信号
    pthread_sigmask(SIG_SETMASK, &old_mask, 0);

    /// 等待特定的信号
    sigset_t wait_mask;
    sigemptyset(&wait_mask);
    sigaddset(&wait_mask, SIGINT);
    sigaddset(&wait_mask, SIGQUIT);
    sigaddset(&wait_mask, SIGTERM);
    sigaddset(&wait_mask, SIGUSR2);
    sigaddset(&wait_mask, SIGHUP);
    pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
    int sig = 0;
    int ret = -1;
    while (-1 != (ret = sigwait(&wait_mask, &sig)))
    {
        LOG(INFO)<< "Receive signal. " << sig;
        if (sig == SIGUSR2)
        {
            flush_log();
            continue;
        }
        if (sig == SIGHUP)
        {
            LOG(INFO) << "Receive reload config signal.";
            load_conf(true);
            continue;
        }
        if (sig == SIGTERM || sig == SIGQUIT || sig == SIGINT)
        {
            LOG(INFO) << "Receive stop signal, Exit.";
            g_exit = true;
            break;
        }
    }
    if (ret == -1)
    {
        LOG(INFO)<< "sigwaitinfo() returned err: " << errno
        << "\t" << strerror(errno);
        return false;
    }

    gs->stop();
    tg->join_all();
    g_spool->stop();
    g_spool.reset();
    for (std::map<int, boost::shared_ptr<bus::ConnectionPool> >::iterator it = g_m_cpool.begin(); it != g_m_cpool.end(); ++it)
    {
        it->second->stop();
        it->second.reset();
    }

    return true;
}
#endif

