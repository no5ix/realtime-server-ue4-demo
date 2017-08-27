/**
 * @file
 * @brief 定义一些公共的头文件,及常用的工具
 *
 */

#ifndef HEAD_H_
#define HEAD_H_


#ifdef TCP
#define PROTOCAL boost::asio::ip::tcp::socket
#else
#define PROTOCAL boost::asio::ip::udp::socket
#endif

#define VLOG_DEBUG(n) (VLOG(L_DB)<<#n<<": "<<n)
#define VALUE_STR(n) (#n)
//#define TR printf("%s:%i\n",__FILE__,__LINE__)

//#define BOOST_ALL_NO_LIB
#include <boost/program_options.hpp>
namespace b = boost;
namespace op = b::program_options;

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/random.hpp>
#include <boost/math/common_factor_rt.hpp>
#include <boost/random/uniform_int.hpp>

#include <glog/logging.h>

#include <vector>
using std::vector;
#include <list>
using std::list;
#include <map>
using std::map;
using std::string;
using std::clog;
using std::cerr;
using std::cout;
using std::endl;
using std::make_pair;
using std::min;
using std::max;

#ifndef __HEX_DUMP__
#define __HEX_DUMP__
#include <cstdio>
#include <string.h>
#include <ctype.h>

template<typename RET>
RET atox(const char *pa)
{
    RET ret;
    try
    {
        if (pa == 0 || !isascii(*pa) || strlen(pa) == 0) return ret = 0;
        return ret = boost::lexical_cast<RET>(pa);
    }
    catch(boost::bad_lexical_cast &)
    {
        return ret = 0;
    }
    catch(...)
    {
        return ret = 0;
    }
    return ret = 0;
}
#define ATOL atox<long>
#define ATOF atox<float>
#define ATOI atox<int>
#define ATOLL atox<long long>

namespace bas{

/// 输出字符
struct FormatChar
{
    FormatChar(char c, char* s)
    {
        sprintf((char*) s, "%02X", c);
        s[2] = ' ';
    }
};

///排版输出一行字串
//template<typename PRINT = FormatChar, int LINE_SIZE = 16>
template<typename PRINT, int LINE_SIZE>
class FormatLine
{
    private:
        enum
        {
            REAL_SIZE = LINE_SIZE * 4 + 4
        };
        /// 定义一行buffer
        char line[REAL_SIZE];

    public:
        friend struct FormatChar;

        char* operator()(char* p, size_t& nBegin, size_t& nLeft)
        {
            if (nLeft <= 0)
                return NULL;

            /// 指向字符开始位置
            char* pt = p + nBegin;

            /// 每次取n个byte出来处理
            for (size_t i = 0; i < LINE_SIZE; ++i)
            {
                if (nLeft > 0)
                {
                    /// 把字符打印到左部
                    char c = pt[i];
                    PRINT(c, line + i * 3);

                    /// 转换后打印到右部,不可显字符显示为'.'
                    line[LINE_SIZE * 3 + 3 + i] = isprint(c) ? c : '.';

                    /// 剩余未打印字符数量减１
                    --nLeft;
                }
                else ///　全部字符处理完成,但未完成一行所需字符时补空格
                {
                    /// 左部结束后补空格
                    line[i * 3] = ' ';
                    line[i * 3 + 1] = ' ';

                    ///　右部结束后加补空格
                    line[LINE_SIZE * 3 + 3 + i] = ' ';
                }
            }
            /// 左部末尾显示空格
            line[LINE_SIZE * 3] = ' ';
            line[LINE_SIZE * 3 + 1] = ' ';
            line[LINE_SIZE * 3 + 2] = ' ';

            /// 右部末尾显示空格
            line[REAL_SIZE - 1] = '\0';
            /// 偏移记录开始的位置
            nBegin += LINE_SIZE;
            return line;
        }
};

///　提供外部调用的封装
//template<typename LINE = FormatLine<FormatChar,16> >
template<typename LINE>
void hexdump(char* p, size_t len)
{
    size_t i = 0;
    size_t j = len;
    char* s = 0;
    LINE ft;
    while (0 != (s = ft(p, i, j)))
        printf("%s\n", s);
}
#define HexDump hexdump<FormatLine<FormatChar, 32> >

}/* bas */

#endif /* __HEX_DUMP__ */

#include "Code.h"

#endif /* HEAD_H_ */
