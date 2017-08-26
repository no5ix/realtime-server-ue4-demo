/**
 * @file
 * @brief 服务端封装
 *
 */

#ifndef BAS_SERVER_HPP
#define BAS_SERVER_HPP

#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "io_service_pool.h"
#include "service_handler.h"
#include "service_handler_pool.h"

namespace bas
{

   /// 封装顶级应用 server.
   template<typename Work_Handler, typename Work_Allocator,
            typename Socket_Service = PROTOCAL>
   class server: private boost::noncopyable
   {
      public:
         /// 定义 service_handler的类型
         typedef service_handler<Work_Handler, Socket_Service>
                  service_handler_type;
         typedef boost::shared_ptr<service_handler_type> service_handler_ptr;

         /// 定义 service_handler_pool的类型
         typedef service_handler_pool<Work_Handler, Work_Allocator,
                  Socket_Service> service_handler_pool_type;
         typedef boost::shared_ptr<service_handler_pool_type>
                  service_handler_pool_ptr;

         /// 构造 server 并 listen 指定的 TCP 地址及端口
         explicit server(const std::string& address, unsigned short port,
                  std::size_t io_service_pool_size,
                  std::size_t work_service_pool_size,
                  service_handler_pool_type* service_handler_pool) :
                     accept_service_pool_(1),
                     io_service_pool_(io_service_pool_size),
                     work_service_pool_(work_service_pool_size),
                     service_handler_pool_(service_handler_pool),
                     acceptor_(accept_service_pool_.get_io_service()),
                     endpoint_(boost::asio::ip::address::from_string(address),
                              port), started_(false)
         {
            BOOST_ASSERT(service_handler_pool != 0);
         }

         /// 析构 server
         ~server()
         {
            /// 停止 io_service loop.
            stop();
            /// 析构 service_handler pool.
            service_handler_pool_.reset();
         }

         /// 服务端运行的主线程
         void run()
         {
            if (started_)
               return;

            /// 建立acceptor 并设置连接参数 SO_REUSEADDR
            acceptor_.open(endpoint_.protocol());
            acceptor_.set_option(
                     boost::asio::ip::tcp::acceptor::reuse_address(true));
            acceptor_.bind(endpoint_);
            acceptor_.listen();
            /// 接受新连接:调 accept_one()预分配新的资源到下次的受理连接中
            accept_one();

            /// 以异步非阻塞同步方式启动 work_service_pool 处理
            work_service_pool_.start();
            /// 以异步非阻塞方式启动 io_service_pool 来处理socket的i/o操作
            io_service_pool_.start();

            started_ = true;
            /// accept_service_pool阻塞异步accept
            accept_service_pool_.run();

            /// 停止 io_service_pool.
            io_service_pool_.stop();
            /// 停止 work_service_pool.
            work_service_pool_.stop();

            started_ = false;
         }

         /// 停止 server.
         void stop()
         {
            if (!started_)
               return;
#ifdef TCP
            // 停止本thread控制的acceptor
            acceptor_.get_io_service().dispatch(
                     boost::bind(&boost::asio::ip::tcp::acceptor::close,
                              &acceptor_));
#endif
            // 主动关闭所有连接
            service_handler_pool_->close();
            // 停止其它阻塞中的 accept_service_pool
            accept_service_pool_.stop();
         }

      private:
         /// 为一个新受理的连接分配资源
         void accept_one()
         {
            /// 获取处理handler同时分配给handler相关的i/o等资源
            service_handler_ptr handler =
                     service_handler_pool_->get_service_handler(
                              io_service_pool_.get_io_service(),
                              work_service_pool_.get_io_service());
#ifdef TCP
            /// 注册一个异步受理事件
            acceptor_.async_accept(
                     handler->socket().lowest_layer(),
                     boost::bind(&server::handle_accept, this,
                              boost::asio::placeholders::error, handler));
#endif
         }

         /// 连接accept后调用处理操作
         void handle_accept(const boost::system::error_code& e,
                  service_handler_ptr handler)
         {
            if (!e)
            {
               /// 启动 handler 的处理过程
               handler->start();
               /// 再次接受下一个连接
               accept_one();
            }
            else
               handler->close(e);
         }

      private:
         /// 异步 accept用的io_service池,只有一个
         io_service_pool accept_service_pool_;

         /// 给socket用的异步i/o(收发)的io_service池
         io_service_pool io_service_pool_;

         /// 用于通知service_handler调用处理函数的io_service池
         io_service_pool work_service_pool_;

         /// 处理响应service_handler池的指针
         service_handler_pool_ptr service_handler_pool_;

         /// 侦听用的受理器acceptor
         boost::asio::ip::tcp::acceptor acceptor_;

         /// 标识服务端的endpoint
         boost::asio::ip::tcp::endpoint endpoint_;

         /// 标识服务是否started
         bool started_;
   };

} // namespace bas

#endif // BAS_SERVER_HPP
