/**
 * @file
 * @brief 处理框架
 *
 */

#ifndef BAS_SERVICE_HANDLER_HPP
#define BAS_SERVICE_HANDLER_HPP

#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "Pre.h"

#include "io_buffer.h"
#include "Protocol.h"

namespace bas {
/// 分发的消息类型
struct event
{
        enum state_t
        {
            none = 0,
            parent_open,
            parent_read,
            parent_write,
            parent_close,
            child_open,
            child_read,
            child_write,
            child_close,
            user = 1000
        };

        std::size_t state_;
        std::size_t value_;

        event(std::size_t state = 0, std::size_t value = 0) :
                    state_(state), value_(value)
        {
        }
};

template<typename, typename, typename > class service_handler_pool;
template<typename, typename, typename > class server;
template<typename, typename, typename > class client;

/// socket的异步操作
template<typename Work_Handler, typename Socket_Service = PROTOCAL>
class service_handler: public boost::enable_shared_from_this<
service_handler<Work_Handler, Socket_Service> >,
private boost::noncopyable
{
    public:
    using boost::enable_shared_from_this<service_handler<Work_Handler,
    Socket_Service> >::shared_from_this;

    /// 定义 service_handler类型
            typedef service_handler<Work_Handler, Socket_Service>
            service_handler_type;

            /// 定义 work_handler类型
            typedef Work_Handler work_handler_type;

            /// 定义将作异步操作的 socket类型
            typedef Socket_Service socket_type;

            /// 构造 service_handler
            explicit service_handler(work_handler_type* work_handler,
                        std::size_t read_buffer_size, std::size_t write_buffer_size,
                        std::size_t timeout_seconds, std::size_t closed_wait_delay) :
            work_handler_(work_handler),
            socket_(),
            timer_(),
            io_service_(0),
            work_service_(0),
            timer_count_(0),
            stopped_(true),
            closed_(true),
            timeout_seconds_(timeout_seconds),
            closed_wait_time_(
                        boost::posix_time::seconds(closed_wait_delay)),
            restriction_time_(
                        boost::posix_time::microsec_clock::universal_time()),
            read_buffer_(read_buffer_size),
            write_buffer_(write_buffer_size)
            {
                BOOST_ASSERT(work_handler != 0);
                BOOST_ASSERT(closed_wait_delay != 0);
            }

            /// 折构
            ~service_handler()
            {
            }

            /// 取数据
            Protocol& protocol()
            {
                return protocol_;
            }

            /// 从io_buffer中获取输入数据
            io_buffer& read_buffer()
            {
                return read_buffer_;
            }

            std::string& peer()
            {
                return peer_;
            }
            /// 从io_buffer中获取输出数据
            io_buffer& write_buffer()
            {
                return write_buffer_;
            }

            /// 获取已关联的socket
            socket_type& socket()
            {
                BOOST_ASSERT(socket_.get() != 0);
                return *socket_;
            }

            /// 关闭连接
            void close(const boost::system::error_code& e)
            {
                // 如果线程已完毕,返回
                if (stopped_)
                return;

                // 分派消息到socket 的io_service thread
                io_service().dispatch(
                            boost::bind(&service_handler_type::close_i,
                                        shared_from_this(), e));
            }

            /// 使用error_code=0 来关闭
            void close()
            {
                close(
                            boost::system::error_code(0,
                                        boost::system::get_system_category()));
            }

            /// 供其它线程注册一个异步从socket读取一定的数据的操作,
            ///　收到数据后回调　service_handler_type::async_read_some_i<>()
            template<typename Buffers>
            void async_read_some(Buffers& buffers)
            {
                io_service().dispatch(
                            boost::bind(
                                        &service_handler_type::template async_read_some_i<Buffers>,
                                        shared_from_this(), buffers));
            }

            /// 供其它线程异步从socket读取一定的数据的操作;
            /// 调用者必须保证　read_buffer().space() > 0.
            void async_read_some()
            {
                BOOST_ASSERT(read_buffer().space() != 0);
                boost::asio::mutable_buffers_1 buf = boost::asio::buffer(
                            read_buffer().data() + read_buffer().size(),
                            (std::size_t) read_buffer().space());
                async_read_some(buf);
            }

            /// 异步读取一定量数据.
            /// @note 调用者必须保证 length != 0 and length <= read_buffer().space()
            void async_read(std::size_t length)
            {
                BOOST_ASSERT(length != 0 && length <= read_buffer().space());
                async_read(
                            boost::asio::buffer(
                                        read_buffer().data() + read_buffer().size(),
                                        length));
            }

            /// 异步读取一个数据包.(TCP/UDP可用)
            void async_read_pack()
            {
                BOOST_ASSERT(read_buffer().space() != 0);
                boost::asio::mutable_buffers_1 buf = boost::asio::buffer(
                            read_buffer().data() + read_buffer().size(), HEADLEN);

                boost::asio::async_read(
                            socket(),
                            buf,
                            boost::asio::transfer_all(),
                            boost::bind(&service_handler_type::pack_body,
                                        shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
            }

            /// 向socket io调用一个异步读操作
            template<typename Buffers>
            void async_read(Buffers& buffers)
            {
                io_service().dispatch(
                            boost::bind(&service_handler_type::template async_read_i<Buffers>,
                                        shared_from_this(), buffers));
            }

            /// 注册异步写操作
            /// @note  调用者必须保证 write_buffer().size() > 0
            void async_write()
            {
                BOOST_ASSERT(write_buffer().size() != 0);
                async_write(
                            boost::asio::buffer(write_buffer().data(),
                                        write_buffer().size()));
            }

            /// 注册异步写一定量数据操作
            /// @note 调用者必须保证 length != 0 and length <= write_buffer().size()
            void async_write(std::size_t length)
            {
                BOOST_ASSERT(length != 0 && length <= write_buffer().size());
                async_write(boost::asio::buffer(write_buffer().data(), length));
            }

            /// 注册异步写操作
            template<typename Buffers>
            void async_write(Buffers buffers)
            {
                io_service().dispatch(
                            boost::bind(&service_handler_type::template async_write_i<Buffers>,
                                        shared_from_this(), buffers));
            }

            /// 通知父handler
            void post_parent(const bas::event event)
            {
                work_service().post(
                            boost::bind(&service_handler_type::do_parent,
                                        shared_from_this(), event));
            }

            /// 通知子handler
            void post_child(const bas::event event)
            {
                work_service().post(
                            boost::bind(&service_handler_type::do_child,
                                        shared_from_this(), event));
            }

            /// 取得socket使用的io_service
            boost::asio::io_service& io_service()
            {
                BOOST_ASSERT(io_service_ != 0);
                return *io_service_;
            }

            /// 取得通知worke handler用的io_service
            boost::asio::io_service& work_service()
            {
                BOOST_ASSERT(work_service_ != 0);
                return *work_service_;
            }

            private:
            template<typename , typename , typename > friend class service_handler_pool;
            template<typename , typename , typename > friend class server;
            template<typename , typename , typename > friend class client;
            template<typename , typename > friend class service_handler;

            /// 检查handler是否繁忙
            bool is_busy()
            {
                /// 状态是closed或线程中即时产生时间<关闭时算出的可重用时间认为忙
                return !closed_
                || (boost::posix_time::microsec_clock::universal_time()
                            < restriction_time_);
            }

            /// 把 service_handler与给定的 io_service和 work_service绑定
            template<typename Work_Allocator>
            void bind(boost::asio::io_service& io_service,
                        boost::asio::io_service& work_service,
                        Work_Allocator& work_allocator)
            {
                BOOST_ASSERT(timer_count_ == 0);

                closed_ = false;
                stopped_ = false;

                /// 通过模板实现后延式的模板模式调用
                socket_.reset(work_allocator.make_socket(io_service));
                timer_.reset(new boost::asio::deadline_timer(io_service));

                io_service_ = &io_service;
                work_service_ = &work_service;

                /// 清理读写缓冲区
                read_buffer().clear();
                write_buffer().clear();

                /// 调用户实现中的 on_clear()操作
                work_handler_->on_clear(*this);
            }

            /// 开启一个异步连接
            void connect(boost::asio::ip::tcp::endpoint& endpoint)
            {
                BOOST_ASSERT(socket_.get() != 0);
                BOOST_ASSERT(timer_.get() != 0);
                BOOST_ASSERT(work_service_ != 0);

                if (timer_count_ == 0)
                set_expiry(timeout_seconds_);

                socket().lowest_layer().async_connect(
                            endpoint,
                            boost::bind(&service_handler_type::handle_connect,
                                        shared_from_this(),
                                        boost::asio::placeholders::error));
            }

            /// 接受连接成功后,第一个将被调用的地方
            void start()
            {
                BOOST_ASSERT(socket_.get() != 0);
                BOOST_ASSERT(timer_.get() != 0);
                BOOST_ASSERT(work_service_ != 0);

                if (timer_count_ == 0)
					set_expiry(timeout_seconds_);

                try{
                   peer_ = socket_->remote_endpoint().address().to_string();
                   // 将Post消息通知work_service执行do_open()
                   work_service().post(
                            boost::bind(&service_handler_type::do_open,
                                        shared_from_this()));
                }catch(...){
                   socket_->close();
                }
            }

            private:
            /// 异步读取包头完成后,读包体操作
            void pack_body(const boost::system::error_code& e,
                        std::size_t bytes_transferred)
            {
                // 如果停止标志为真,直接返回
                if (stopped_)
                return;

                ///收到消息后.重新开始计算超时时间
                set_expiry(timeout_seconds_);
                if (!e)
                {

                    //HexDump((char*) (read_buffer().data()), 256);

                    if (Protocol::good != protocol_.init(
                                            (char*) (read_buffer().data()
                                                        + read_buffer().size()), HEADLEN))
                    close();

                    unsigned int len = protocol_.getLen();
                    //std::clog << "need len: " << len << std::endl;
                    //if (len > 0) /// 正常的包
                    {
                        boost::asio::mutable_buffers_1 buf = boost::asio::buffer(
                                    read_buffer().data() + read_buffer().size() + HEADLEN, len);

                        boost::asio::async_read(
                                    socket(),
                                    buf,
                                    boost::asio::transfer_all(),
                                    boost::bind(&service_handler_type::pack_done,
                                                shared_from_this(),
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
                    }
                    //else  /// 错误的,只有包头的包
                    {
                        //pack_done (e, bytes_transferred);
                    }

                }
                else
                {
                    /// 错误信息则关闭线程中的 socket连接
                    close_i(e);
                }
            }

            /// 异步读取包体完成后,调用户的操作
            void pack_done(const boost::system::error_code& e,
                        std::size_t bytes_transferred)
            {
                // 如果停止标志为真,直接返回
                if (stopped_)
                return;

                ///收到消息后.重新开始计算超时时间
                set_expiry(timeout_seconds_);
                if (!e)
                {
                    if(Protocol::good != protocol_.init(
                                (char*) (read_buffer().data()
                                            + read_buffer().size()), bytes_transferred + HEADLEN))
                    {  ///错误的信息包，关闭连接
                       ///@todo close_i();
                    }
                    /// 如果不是错误信息,则Post到 work_service中执行 do_read.
                    work_service().post(
                                boost::bind(&service_handler_type::do_read,
                                            shared_from_this(), bytes_transferred + HEADLEN));
                }
                else
                {
                    /// 错误信息则关闭线程中的 socket连接
                    close_i(e);
                }
            }

            /// 注册一个异步读操作(TCP可用)
            template<typename Buffers>
            void async_read_some_i(Buffers& buffers)
            {
                // 标识结束则返回
                if (stopped_)
                return;
                socket().async_read_some(
                            buffers,
                            boost::bind(&service_handler_type::handle_read,
                                        shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));

            }

            /// 注册一个异步读操作
            template<typename Buffers>
            void async_read_i(Buffers& buffers)
            {
                // 标识结束则返回
                if (stopped_)
                return;

                boost::asio::async_read(
                            socket(),
                            buffers,
                            boost::asio::transfer_all(),
                            boost::bind(&service_handler_type::handle_read,
                                        shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
            }

            /// 注册一个异步写操作
            template<typename Buffers>
            void async_write_i(Buffers& buffers)
            {
                if (stopped_)
                return;

                boost::asio::async_write(
                            socket(),
                            buffers,
                            boost::bind(&service_handler_type::handle_write,
                                        shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
            }

            /// 设置连接的超时时间
            void set_expiry(std::size_t timeout_seconds)
            {
                //printf("设置超时时间\n");
                if (timeout_seconds == 0)
                return;

                BOOST_ASSERT(timer_.get() != 0);

                ++timer_count_;
                timer_->expires_from_now(
                            boost::posix_time::seconds(timeout_seconds));
                timer_->async_wait(
                            boost::bind(&service_handler_type::handle_timeout,
                                        shared_from_this(),
                                        boost::asio::placeholders::error));
            }

            /// 连接操作成功后调用
            void handle_connect(const boost::system::error_code& e)
            {
                // 如果handler 已停止,直接返回
                if (stopped_)
                return;

                if (!e)
                {
                    start();
                }
                else
                {
                    // 带 error_code 关闭
                    close_i(e);
                }
            }

            /// 注册异步读操作
            void handle_read(const boost::system::error_code& e,
                        std::size_t bytes_transferred)
            {
                // 如果停止标志为真,直接返回
                if (stopped_)
                return;

                ///收到消息后.重新开始计算超时时间
                set_expiry(timeout_seconds_);
                if (!e)
                {
                    /// 如果不是错误信息,则Post到 work_service中执行 do_read.
                    work_service().post(
                                boost::bind(&service_handler_type::do_read,
                                            shared_from_this(), bytes_transferred));
                }
                else
                {
                    /// 错误信息则关闭线程中的 socket连接
                    close_i(e);
                }
            }

            /// 注册异步写操作
            void handle_write(const boost::system::error_code& e,
                        std::size_t bytes_transferred)
            {
                // 如果标识为已停止,直接返回
                if (stopped_)
                return;

                if (!e)
                {
                    // 通知to work_service执行do_write()
                    work_service().post(
                                boost::bind(&service_handler_type::do_write,
                                            shared_from_this(), bytes_transferred));
                }
                else
                {
                    // 带error_code 关闭
                    close_i(e);
                }
            }

            /// 超时后的调用
            void handle_timeout(const boost::system::error_code& e)
            {
                --timer_count_;

                // 如果handler 已经停止或timer 已取消,直接返回
                if (stopped_ || e == boost::asio::error::operation_aborted)
                return;

                if (!e)
                {
                    // 设置 error_code 为boost::system::timed_out,并关闭
                    close_i(
                                boost::system::error_code(
                                            boost::asio::error::timed_out,
                                            boost::system::get_system_category()));
                }
                else
                {
                    // 带error_code 关闭
                    close_i(e);
                }
            }

            /// 关闭handler thread 中的socket io_service
            void close_i(const boost::system::error_code& e)
            {
                if (!stopped_)
                {

                    stopped_ = true;

                    // 关闭socket
                    boost::system::error_code ignored_ec;
                    socket().lowest_layer().shutdown(PROTOCAL::shutdown_both, ignored_ec);
                    socket().lowest_layer().close();

                    // 取消计时器
                    if (timer_count_ != 0)
                    {
                        timer_->cancel();
                    }

                    // 传消息到用户 work_service调用 do_close()
                    work_service().post(
                                boost::bind(&service_handler_type::do_close,
                                            shared_from_this(), e));
                }
            }

            /// 在work_service 线程中调用on_open()
            void do_open()
            {
                // 如果thread已经停止,直接返回
                if (stopped_)
                return;

                // 调用　work handler的on_open()
                work_handler_->on_open(*this);
            }

            /// 通知work_service 调 on_read()
            void do_read(std::size_t bytes_transferred)
            {
                // 如果已经标识为停止,直接返回
                if (stopped_)
                return;

                /// 调work handler的 on_read()
                work_handler_->on_read(*this, bytes_transferred);
            }

            /// 通知 work_service thread.
            void do_write(std::size_t bytes_transferred)
            {
                // 标识已结束则退出
                if (stopped_)
                return;

                /// 调work handler的 on_write()
                work_handler_->on_write(*this, bytes_transferred);
            }

            /// 设置parent handler 中的 work_service
            template<typename Parent_Handler>
            void set_parent(Parent_Handler* handler)
            {
                // 标识已结束则退出
                if (stopped_)
                return;

                /// 调work handler 的 on_set_parent()
                work_handler_->on_set_parent(*this, handler);
            }

            /// 设置parent handler 中的work_service
            template<typename Child_Handler>
            void set_child(Child_Handler* handler)
            {
                // 标识已结束则退出
                if (stopped_)
                return;

                /// 调work handler 的 on_set_child()
                work_handler_->on_set_child(*this, handler);
            }

            /// 执行work_service 中的on_parent()
            void do_parent(const bas::event event)
            {
                // 标识已结束则返回
                if (stopped_)
                return;

                /// 调work handler的 on_parent()
                work_handler_->on_parent(*this, event);
            }

            /// 执行 work_service中的on_child()
            void do_child(const bas::event event)
            {
                // 标识已结束则返回
                if (stopped_)
                return;

                /// 调 work handler的 on_child()
                work_handler_->on_child(*this, event);
            }

            /// 关闭并重置handler
            void do_close(const boost::system::error_code& e)
            {
                /// 调 work handler 的on_close()
                work_handler_->on_close(*this, e);

                timer_.reset();

                // 设置可重用时间,检查是否繁忙时也用到这个值
                restriction_time_
                = boost::posix_time::microsec_clock::universal_time()
                + closed_wait_time_;

                closed_ = true;

                // 这里可以考虑同时关闭socket,
            }

            private:
            typedef boost::shared_ptr<work_handler_type> work_handler_ptr;
            typedef boost::shared_ptr<socket_type> socket_ptr;
            typedef boost::shared_ptr<boost::asio::deadline_timer> timer_ptr;

            /// Work 指针
            work_handler_ptr work_handler_;

            /// Socket 指针
            socket_ptr socket_;

            /// 超时计时器指针
            timer_ptr timer_;

            /// 用于socket的异步通知io_service
            boost::asio::io_service* io_service_;

            /// 用于异步通知work handler的 io_service
            boost::asio::io_service* work_service_;

            /// 等待计时器
            std::size_t timer_count_;

            /// 标识 handler已停止
            bool stopped_;

            /// 标识 handler已关闭以便,一段时间后重用
            bool closed_;

            /// 连接超时时间
            std::size_t timeout_seconds_;

            /// 关闭等待时间
            boost::posix_time::time_duration closed_wait_time_;

            /// 可重用的时间器
            boost::posix_time::ptime restriction_time_;

            /// 读入用的缓冲区
            io_buffer read_buffer_;

            /// 写出用的缓冲区
            io_buffer write_buffer_;

            /// 接收到的协议数据结构
            Protocol protocol_;

            /// 对端IP，方便客户端断开时记录日志
            std::string peer_;
        };

        } // namespace bas

#endif // BAS_SERVICE_HANDLER_HPP
