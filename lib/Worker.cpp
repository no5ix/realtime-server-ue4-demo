/**
 * @file Worker.cpp
 * @brief
 *
 */
#include <boost/function.hpp>
#include <glog/logging.h>
#include "Worker.h"

namespace bas {
//using namespace std;
Worker::Worker() {}

Worker::~Worker() {}

void Worker::on_close(server_handler_type & handler,
            const boost::system::error_code & e)
{//@TODO 断开时更新IP连接状态
    if (e.value() == 2)
        VLOG(8) << " client [" << handler.peer() << "] disconnected! " << e.message() << std::endl;
}

void Worker::on_write(server_handler_type & handler,
            std::size_t bytes_transferred)
{
    handler.async_read_pack();
}

void Worker::on_open(server_handler_type & handler)
{
    try{//@TODO 增加IP连接记录
        VLOG(8) << "client [" << handler.socket().remote_endpoint().address().to_string() << "] connect";
        handler.async_read_pack();
    } catch (boost::system::system_error &err){
        VLOG(8) << "connect err: " << err.what();
    }
}

void Worker::ack(server_handler_type& handler, int len)
{
    boost::asio::mutable_buffers_1 buf = boost::asio::buffer(handler.write_buffer().data(), len);
    handler.async_write(buf);
}

void Worker::on_read(server_handler_type& handler, std::size_t bytes_transferred)
{
    VLOG(200) << "Worker: " << boost::this_thread::get_id();
    _proto.init((char*) handler.read_buffer().data(), bytes_transferred);
    _proc.process(handler,this);
}
}
