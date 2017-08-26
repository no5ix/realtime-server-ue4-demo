/**
 * @file BuildJson.hpp
 * @brief 封装json生成
 */

#ifndef BUILDJSON_HPP_
#define BUILDJSON_HPP_
#define BOOST_SPIRIT_THREADSAFE
#define BOOST_ALL_DYN_LINK
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <utility>
#include <iostream>
#define Pair(X,Y) std::make_pair(X,Y)
namespace Js = boost::property_tree;
using boost::property_tree::ptree;
class Json {
public:
    template<typename F>
    Json& operator<<(F a){
        pt.put(a.first, a.second);
        return *this;
    }

    template<typename T>
    T get(const Js::ptree::path_type & path){
        try{
           return pt.get<T>(path);
        }catch(boost::property_tree::ptree_error &e){
           throw e;
        }
    }
    // 节点不存在 返回默认值
    template<typename T>
    T get(const Js::ptree::path_type & path, const T & default_value){
        return pt.get(path, default_value);
    }

    template<typename T>
    void put(const char* path, const T &value){
        pt.put(path, value);
    }
    
     template<typename T>
    void push_back(const T &value){
    	pt.push_back(value);
    }
    
    void clear(){
        pt.clear();
    }

    const Js::ptree& get_child(const Js::ptree::path_type &  path) const{
        return pt.get_child(path);
    }

    const Js::ptree& add_child(const Js::ptree::path_type &path, const Js::ptree &value){
        return pt.add_child(path, value);
    }

    std::string Str(){
        try{
            std::ostringstream ss;
            Js::write_json (ss, pt, false);
            return ss.str();
        }catch(...){
            return "";
        }
    }
    bool ReadJson(const char* str){
        try{
            std::stringstream ss(str);
            Js::read_json(ss, pt);
        }catch(...){
            std::cerr << "Parse Json failed" << std::endl;
            return false;
        }
        return true;
    }
    Js::ptree& Pt(){
        return pt;
    }

private:
    Js::ptree pt;
};

#endif /* BUILDJSON_HPP_ */
