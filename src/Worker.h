/**
 * @file Worker.h
 * @brief
 *
 */

#ifndef WORKER_H_
#define WORKER_H_
#include "service_handler.h"
#include "Protocol.h"
#include "../bus/Process.h"
#include <boost/enable_shared_from_this.hpp>
namespace bas {
/**
 * 实际工作类
 */
class Worker;
typedef boost::shared_ptr<Worker> worker_ptr;
class Worker
{
        typedef service_handler<Worker> server_handler_type;
    public:
        Worker();
        virtual ~Worker();

        /// 关闭时调用
        void on_close(server_handler_type& handler,
                    const boost::system::error_code& e);

        void on_clear(server_handler_type& handler)
        {
        }

        /// 新连接打开时调用
        void on_open(server_handler_type& handler);

        void on_read(server_handler_type& handler,
                    std::size_t bytes_transferred);
        void on_write(server_handler_type & handler,
                    std::size_t bytes_transferred);

        void ack(server_handler_type& handler, int len);
    private:
        Protocol _proto;
        bus::Process _proc;
        //char    _heartbeat[11];
        //int _hb_len;
};

/**
 * 工作实例生成器
 */
class Worker_Allocator
{
    public:
        typedef PROTOCAL socket_type;

        Worker_Allocator()
        {
        }
        /// 为适应service_handler 调用的模板模式
        socket_type* make_socket(boost::asio::io_service& io_service)
        {
            return new socket_type(io_service);
        }

        Worker* make_handler()
        {
            return new Worker();
        }
};
}
#endif /* WORKER_H_ */
