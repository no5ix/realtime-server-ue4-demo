/*
 * Process.h
 *
 */

#ifndef PROCESS_H_
#define PROCESS_H_
#include "../src/Pre.h"
#include <mysql.h>
#include <boost/enable_shared_from_this.hpp>
#include "Business.hpp"
#include "../src/io_service_pool.h"
#include "../src/service_handler.h"
namespace bas{
class Worker;
}
/// 全局变量结构,用于保存用到的参数


namespace bus {
using namespace bas;
class Process: public boost::enable_shared_from_this<Process>
{
        typedef service_handler<Worker> server_handler_type;
    public:
        Process();
        virtual ~Process();
        int start()
        {
            return 0;
        }
        int stop(const int cnt=1);

        int process(server_handler_type &handler, Worker* worker)
        {
            deal(&handler, worker);
            return 0;
        }

    public:
        void deal(server_handler_type *handler, Worker* worker);
        void deal_done();


    private:
        Protocol _protocol;
        boost::shared_ptr<BusInf> _bus;
        server_handler_type *_handler;
        Worker *_worker;
};

} // namespace bus

char *verify_null(char *buf);

std::string UrlEncode(const std::string& szToEncode);
	

#endif /* PROCESS_H_ */
