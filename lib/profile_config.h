/**
 * @file
 * @brief 启动参数管理
 *
 */
#ifndef __PROFILE_CONFIG_H__
#define __PROFILE_CONFIG_H__
#include "Pre.h"
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
namespace bas{
//using namespace boost;
//namespace b = boost;
/// 配置解释器
class Config
{
public:
    static b::shared_ptr<Config> _config;
    int parse(int argc=0, char* argv[]=0);
    static Config& instance();
    int count(const char* key) { return (*_cfg).count(key); }
    int count(const std::string &key) { return (*_cfg).count(key.c_str()); }
    void help();

    template<typename RET>
    RET get(const char* key) { return (*_cfg)[key].as<RET>(); }
    template<typename RET>
    RET get(const std::string &key) { return (*_cfg)[key.c_str()].as<RET>(); }

private:
    b::shared_ptr<b::program_options::variables_map>         _cfg;
    b::shared_ptr<b::program_options::options_description>   _desc_cfg;
    b::shared_ptr<b::program_options::options_description>   _visible_cfg;
    b::shared_ptr<b::program_options::options_description>   _hidden_cfg;
    int argc;
    char** argv;
    int clear();
};

#define CONF bas::Config::instance()
}
#endif //__PROFILE_CONFIG_H__


