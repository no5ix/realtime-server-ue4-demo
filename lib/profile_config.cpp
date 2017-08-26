/**
 * @file
 * @brief 启动参数管理
 *
 */

#include <fstream>
#include <iostream>
#include "profile_config.h"

namespace bas{
//using namespace std;
b::shared_ptr<Config> Config::_config;
void Config::help()
{
    std::cout << *_desc_cfg;
}
int Config::clear()
{
    _cfg.reset();
    _cfg = b::shared_ptr<b::program_options::variables_map>(new b::program_options::variables_map);
    _desc_cfg.reset();
    _desc_cfg = b::shared_ptr<b::program_options::options_description>(new b::program_options::options_description);
    _visible_cfg.reset();
    _visible_cfg = b::shared_ptr<b::program_options::options_description>(new b::program_options::options_description);
    _hidden_cfg.reset();
    _hidden_cfg = b::shared_ptr<b::program_options::options_description>(new b::program_options::options_description);
    return 0;
}

int Config::parse(int argc, char* argv[])
{
    try
    {
        clear();
        if(argc!=0 || argv!=0)
        {
            this->argc=argc;
            this->argv=argv;
        }

        std::string brief("PVP Server");

        _desc_cfg->add_options ()
            ("help,h", brief.data())
            ("version,v", "version ")
            ("config-file,c", b::program_options::value<std::string>(),"use config file")
            ("srv.daemon,e",b::program_options::value<bool>()->default_value(true),"run as daemon")
            ("srv.port,p",b::program_options::value<int>()->default_value(4560),"service port")
            ("srv.ip,a", b::program_options::value<std::string>()->default_value("0.0.0.0"),"service ip")
            ("srv.ios,i", b::program_options::value<int>()->default_value(1),"io pool size")
            ("srv.threads,r",b::program_options::value<int>()->default_value(1),"thread pool size")
            ("srv.handlers,j",b::program_options::value<int>()->default_value(1),"preallocated handler number")
            ("srv.buffer,b",b::program_options::value<int>()->default_value(5120),"data buffer size")
            ("srv.timeout,t",b::program_options::value<int>()->default_value(28800),"timeout seconds")
            ("srv.wait,w",b::program_options::value<int>()->default_value(1),"closed wait")

            ("db.realm_cnt", b::program_options::value<int>()->default_value(1), "count of all realms")
            ("db.realm_ids", b::program_options::value<std::string>()->default_value("0"), "all realm ids")
            ("db.servers", b::program_options::value<std::string>()->default_value("localhost"), "all db connections")
            ("db.ports", b::program_options::value<std::string>()->default_value("3306"), "all db ports")
            ("db.dbs", b::program_options::value<std::string>()->default_value("base_td"), "all db databases")
            ("db.users", b::program_options::value<std::string>()->default_value("root"), "all db users")
            ("db.passwds", b::program_options::value<std::string>()->default_value("mobi2us"), "all db passwords")

            ("mysql.server,x", b::program_options::value<std::string>()->default_value("localhost"),"mysql server")
            ("mysql.port,X",b::program_options::value<int>()->default_value(3306),"mysql server port")
            ("mysql.db,D",b::program_options::value<std::string>()->default_value("base_pvp"),"mysql dabase name")
            ("mysql.user,I", b::program_options::value<std::string>()->default_value("root"),"mysql user")
            ("mysql.passwd,P", b::program_options::value<std::string>()->default_value("mobi2us"),"mysql password")
            ("mysql.wait_timeout",b::program_options::value<int>()->default_value(60),"mysql wait timeout")
            ("mysql.interactive_timeout",b::program_options::value<int>()->default_value(30),"mysql interactive timeout")

            ("log.path,l",b::program_options::value<std::string>()->default_value("./"),"log file path")
            ("log.level,L",b::program_options::value<int>()->default_value(700),"log file level")
            ("log.head,g",b::program_options::value<std::string>()->default_value(argv[0]),"log file prefix")
            ("log.term,T",b::program_options::value<bool>()->default_value(false),"log to stderr")
            ("log.max_size,m",b::program_options::value<int>()->default_value(50),"max log size")
            ("log.hex_dump,H",b::program_options::value<bool>()->default_value(false),"log hex dump")

            ("data.sensitive_word",b::program_options::value<std::string>()->default_value("./senword.txt"),"sensitive word list")
             ;

        b::program_options::positional_options_description p;
        p.add("config-file", -1);
        store(
            b::program_options::command_line_parser(this->argc,this->argv).options(*_desc_cfg).positional(p).run(),
            *_cfg);

        notify(*_cfg);

        if ((*_cfg).count("help") || (*_cfg).count("version"))
            return 0;

        if (!(*_cfg).count("config-file"))
        {
            std::cerr << "usage: " << argv[0] << " <config-file> \n";
            exit(EXIT_FAILURE);
        }

        std::ifstream ifs((*_cfg)["config-file"].as<std::string>().c_str());
        store(parse_config_file(ifs, *_desc_cfg), *_cfg);
        ifs.close();
        notify(*_cfg);
    }
    catch(std::exception& e)
    {
        std::cout << e.what() << "\n";
        exit(EXIT_FAILURE);
    }
    return 0;
}

Config& Config::instance()
{
    if (Config::_config.get() == 0)
    {
        Config::_config.reset(new Config);
    }
    return *Config::_config;
}

}

