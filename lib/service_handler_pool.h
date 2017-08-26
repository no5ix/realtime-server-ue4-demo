/**
 * @file
 * @brief 处理线程池
 *
 */

#ifndef BAS_SERVICE_HANDLER_POOL_HPP
#define BAS_SERVICE_HANDLER_POOL_HPP

#include <boost/assert.hpp>
#include <boost/asio/detail/mutex.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

#include "service_handler.h"

namespace bas
{

#define BAS_GRACEFUL_CLOSED_WAIT_DELAY 5

   /// service_handler 池.
   template<typename Work_Handler, typename Work_Allocator,
            typename Socket_Service = PROTOCAL>
   class service_handler_pool: private boost::noncopyable
   {
      public:
         /// 定义 service_handler类型.
         typedef service_handler<Work_Handler, Socket_Service>
                  service_handler_type;
         typedef boost::shared_ptr<service_handler_type> service_handler_ptr;

         /// 定义 work_allocator 类型,负责新建Work_Handler类实例;
         ///　Work_Handler类负责执行业务逻辑
         typedef Work_Allocator work_allocator_type;
         typedef boost::shared_ptr<work_allocator_type> work_allocator_ptr;

         /// 构造 service_handler pool.
         explicit service_handler_pool(
                  work_allocator_type* work_allocator,
                  std::size_t initial_pool_size,
                  std::size_t read_buffer_size,
                  std::size_t write_buffer_size = 0,
                  std::size_t timeout_seconds = 0,
                  std::size_t closed_wait_delay =
                           BAS_GRACEFUL_CLOSED_WAIT_DELAY) :
            service_handlers_(), work_allocator_(work_allocator),
                     initial_pool_size_(initial_pool_size),
                     read_buffer_size_(read_buffer_size),
                     write_buffer_size_(write_buffer_size),
                     timeout_seconds_(timeout_seconds),
                     closed_wait_delay_(closed_wait_delay),
                     next_service_handler_(0)
         {
            BOOST_ASSERT(work_allocator != 0);
            BOOST_ASSERT(initial_pool_size != 0);

            // 预创建一定数量的 service_handler 放入pool.
            for (std::size_t i = 0; i < initial_pool_size_; ++i)
            {
               service_handler_ptr service_handler(make_handler());
               service_handlers_.push_back(service_handler);
            }
         }

         /// 析构
         ~service_handler_pool()
         {
            for (std::size_t i = 0; i < service_handlers_.size(); ++i)
               service_handlers_[i].reset();
            service_handlers_.clear();

            work_allocator_.reset();
         }

         /// 获取一个 service_handler
         service_handler_ptr get_service_handler(
                  boost::asio::io_service& io_service,
                  boost::asio::io_service& work_service)
         {
            service_handler_ptr service_handler;

            /// 取一个不繁忙的 service_handler
            if (!service_handlers_[next_service_handler_]->is_busy())
            {
               ///- 如再下一个已到预定义最大数量的 handler,则从头开始取。
               service_handler = service_handlers_[next_service_handler_];
               if (++next_service_handler_ == service_handlers_.size())
                  next_service_handler_ = 0;
            }
            else
               ///- 如果下一个 handler繁忙,则从头开始取
               next_service_handler_ = 0;

            /// 如果取 handler失败,则创建一个新的,并放入池中
            if (service_handler.get() == 0)
            {
               service_handler.reset(make_handler());
               service_handlers_.push_back(service_handler);
            }

            /// 绑定 service handler / io_service / work_service 三者
            service_handler->bind(io_service, work_service, work_allocator());
            return service_handler;
         }

         /// 获取一个指定 mutex的service_handler
         service_handler_ptr get_service_handler(
                  boost::asio::io_service& io_service,
                  boost::asio::io_service& work_service,
                  boost::asio::detail::mutex& mutex)
         {
            // For client handler, need lock in multiple thread model.
            boost::asio::detail::mutex::scoped_lock lock(mutex);
            return get_service_handler(io_service, work_service);
         }

         /// 主动关闭所有连接
         void close()
         {
            for (size_t i = 0; i < service_handlers_.size(); ++i)
            {
               service_handlers_[i]->close();
            }
         }

      private:

         /// 取得work handler分配器指针
         work_allocator_type& work_allocator()
         {
            return *work_allocator_;
         }

         /// 创建新的 service_handler.
         service_handler_type* make_handler()
         {
            return new service_handler_type(work_allocator().make_handler(),
                     read_buffer_size_, write_buffer_size_, timeout_seconds_,
                     closed_wait_delay_);
         }

      private:

         /// 预分配的service_handler池
         std::vector<service_handler_ptr> service_handlers_;

         /// work_handler分配器
         work_allocator_ptr work_allocator_;

         /// 预分配的service_handler数量
         std::size_t initial_pool_size_;

         /// 异步读操作的最大缓存区
         std::size_t read_buffer_size_;

         /// 异步写操作的最大缓存区
         std::size_t write_buffer_size_;

         /// 连接的socket超时时间
         std::size_t timeout_seconds_;

         /// socket重用之前的等待时间
         std::size_t closed_wait_delay_;

         /// 指向向量列表中下个应该为连接工作所使用的 service_handler
         std::size_t next_service_handler_;
   };

} // namespace bas

#endif // BAS_SERVICE_HANDLER_POOL_HPP
