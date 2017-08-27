/**
 * Protocol.h
 * @file
 * @brief 传协议的封装及解释声明
 *
 */

#ifndef PROTOCAL_H_
#define PROTOCAL_H_

#include <cstddef>
#include <string.h>
#include <sstream>
#include <streambuf>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>

#ifndef __MTK__
#include <arpa/inet.h>
#define hton(x) htonl(x)
#define ntoh(x) ntohl(x)
#define MEMCPY memcpy
#else
#define hton(x) HTONL(x)
#define ntoh(x) NTOHL(x)
#endif

#include "Json.hpp"
#include "xxtea.h"

/**
 * @brief 数据包的数据结构
 */
struct Pack {
    unsigned int len; ///< 包体总长
    unsigned int cmd; ///< 命令码
};

/**
 * @brief 定义各种用到的常量值
 */
enum {
    MAXLEN = 102400, ///< 缓冲区的最大长度
    HEADLEN = sizeof(Pack), ///< 包头长度
};

class mybuf : public std::streambuf{
public:
    mybuf(char* begin, char* cur, char* end){
        this->setg((char*)begin, (char*)cur, (char*)end);
    }
    mybuf(char* begin, char* end){
        this->setp(begin, end);
    }
};

class Protocol {
public:
    enum {
        bad, good
    } stat;

    Protocol():
            stat(bad),
            _head(0),
            _data(0),
            _cmd(0),
            _data_len(0),
            result_len(0) {}

    template<typename T>
    Protocol(T *buf,
            const unsigned int len = HEADLEN):
                                            stat(good),
                                            _head(0),
                                            _data(0),
                                            _cmd(0),
                                            _data_len(len),
                                            result_len(0) {
        init(buf, len);
    }

    template<typename T>
    int init(T *buf, const unsigned int len = HEADLEN) {
        if (len < HEADLEN || buf == 0 || len > MAXLEN) stat = bad;
        _head     = (Pack*) buf;
        _data     = buf + HEADLEN;
        _data_len = len;
        stat      = good;
        memset(tmp, 0, sizeof(tmp));

        memset(result, 0, sizeof(result));

        return stat;
    }

    int Compress(const char* data, const int len){
        mybuf dat((char*)data, (char*)data, (char*)data + len);
        std::istream is(&dat);
        boost::iostreams::filtering_streambuf<boost::iostreams::input> steam;
        steam.push(boost::iostreams::bzip2_compressor());
        steam.push(is);
        mybuf out((char*)result, (char*)result+_data_len);
        std::streamsize slen = boost::iostreams::copy(steam, out);

        string private_key = "b5cd81794a627789b3c03df969e97241";
        unsigned char* encrypt_data = xxtea_encrypt(
            (unsigned char*)result,
            slen,
            (unsigned char*)private_key.c_str(),
            private_key.size(),
            &result_len
        );
        memcpy(_data, encrypt_data, result_len);

        free(encrypt_data);
        return result_len;
    }

    int Decompress(){
        string private_key = "b5cd81794a627789b3c03df969e97241";
        unsigned char* decrypt_data = xxtea_decrypt(
            (unsigned char*)_data,
            _data_len,
            (unsigned char*)private_key.c_str(),
            private_key.size(),
            &result_len
        );

        boost::iostreams::filtering_streambuf< boost::iostreams::input > steam;
        steam.push(boost::iostreams::bzip2_decompressor());
        mybuf dat((char*)decrypt_data, (char*)decrypt_data, (char*)decrypt_data + result_len);
        std::istream is(&dat);
        steam.push(is);
        mybuf out((char*)tmp, (char*)tmp + MAXLEN);
        std::streamsize slen = boost::iostreams::copy(steam, out);

        free(decrypt_data);
        return slen;
    }


    /**
     * @note 须在压入数据后再调用
     */
    int Encode(bool check = true) { return stat == bad ? -1 : _data_len + HEADLEN; }

    Protocol& operator<< (const unsigned int cmd) {
        if (stat == bad) return *this;
        _cmd = cmd;
        _head->cmd = hton(cmd);
        return *this;
    }

    Protocol& operator<< (Json &json){
        if (stat == bad) return *this;
        std::string str(json.Str());
        if (str.length() > MAXLEN - HEADLEN) stat = bad;
        return *this << str;
    }

    Protocol& operator<< (Json *json){
        if (stat == bad) return *this;
        std::string str(json->Str());
        if (str.length() > MAXLEN - HEADLEN) stat = bad;
        return *this << (std::string&)str;
    }

    Protocol& operator<< (const char* str) {
        if (stat == bad || str == 0 || strlen(str) > MAXLEN - HEADLEN) {
            stat = bad;
            return *this;
        }
        int ret = 0;
        if (0 < (ret = Compress(str, strlen(str)))){
            _data_len = ret;
            _head->len = hton(_data_len);
        } else {
            stat = bad;
        }
        return *this;
    }

    Protocol& operator<< (char* str) {
        if (stat == bad || str == 0 || strlen(str) > MAXLEN - HEADLEN) {
            stat = bad;
            return *this;
        }
        int ret = 0;
        if (0 < (ret = Compress(str, strlen(str)))){
            _data_len = ret;
            _head->len = hton(_data_len);
        } else {
            stat = bad;
        }
        return *this;
    }

    Protocol& operator<< (std::string *t) {
        if (stat == bad || t->length() > MAXLEN - HEADLEN) {
            stat = bad;
            return *this;
        }
        int ret = 0;
        if (0 < (ret = Compress(t->c_str(), t->length()))){
            _data_len = ret;
            _head->len = hton(_data_len);
        } else {
            stat = bad;
        }
        return *this;
    }

    Protocol& operator<< (std::string &t) {
        if (stat == bad || t.length() > MAXLEN - HEADLEN) {
            stat = bad;
            return *this;
        }
        int ret = 0;
        if (0 < (ret = Compress(t.c_str(), t.length()))){
            _data_len = ret;
            _head->len = hton(_data_len);
        } else {
            stat = bad;
        }
        return *this;
    }


    /**
     * 解包
     * @param check true:详解并验证 false:只取包头长度
     * @return -1:失败 其它：成功的包长
     */
    int Decode(bool check = false) {
        if (stat == bad) return -1;
        if (true == check) {
        }
        _data_len = ntoh(_head->len);
        _cmd = ntoh(_head->cmd);
        return _data_len + HEADLEN;
    }

    /**
     * @note 须先调用Decode
     */
    Protocol& operator>> (unsigned int &cmd) {
        if (stat == bad) return *this;
        cmd = _cmd;
        return *this;
    }

    Protocol& operator>> (char* t) {
        if (stat == bad) return *this;
        if (0<Decompress()){
            stat = good;
            strcpy(t , (const char*) tmp);
        } else {
            stat = bad;
        }
        return *this;
    }

    Protocol& operator>> (Json &t){
        if (stat == bad) return *this;
        if(0<Decompress()){
            t.ReadJson((const char*)tmp);
            stat = good;
        } else {
            stat = bad;
        }
        return *this;
    }

    Protocol& operator>> (Json *t){
        if (stat == bad) return *this;
        if(0<Decompress()){
            t->ReadJson((const char*)tmp);
            stat = good;
        } else {
            stat = bad;
        }
        return *this;
    }

    Protocol& operator>> (std::string *t) {
        if (stat == bad || _data_len > t->max_size()) {
            stat = bad;
            return *this;
        }
        if(0<Decompress()){
            (*t).assign((const char*) tmp);
            stat = good;
        } else {
            stat = bad;
        }
        return *this;
    }

    Protocol& operator>> (std::string &t) {
        if (stat == bad || _data_len > t.max_size()) {
            stat = bad;
            return *this;
        }
        if (0<Decompress()){
            t.assign((const char*) tmp);
            stat = good;
        } else {
            stat = bad;
        }
        return *this;
    }

    /**
     * @note 可在Decode前调用
     */
    unsigned int getLen() { return stat == bad ? -1 : ntoh(_head->len); }

    /**
     * @note 可在Decode前调用
     */
    unsigned int getCmd() { return stat == bad ? -1 : ntoh(_head->cmd); }

    Pack* getHead() { return _head; }

    void* getData() { return _data; }

private:
    Pack *_head; ///< 包头
    void *_data; ///< 数据
    unsigned int _cmd; ///< host格式的命令码
    unsigned int _data_len; ///< host格式的长度
    uint8_t tmp[MAXLEN];

    unsigned int result_len;
    uint8_t result[MAXLEN];
};

#endif /* PROTOCAL_H_ */
