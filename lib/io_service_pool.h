/**
 * @file
 * @brief 以池方式封装io_service
 *
 */

#ifndef BAS_IO_SERVICE_POOL_HPP
#define BAS_IO_SERVICE_POOL_HPP

#include <boost/assert.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace bas
{

   ///io_service管理池,把io_service转为专门用一线程在后台运行
   class io_service_pool: private boost::noncopyable
   {
      public:
         /// 构造
         explicit io_service_pool(std::size_t pool_size) :
            io_services_(), work_(), threads_(), next_io_service_(0),
                     block_(false)
         {
            BOOST_ASSERT(pool_size != 0);

            // 创建io_service 池
            for (std::size_t i = 0; i < pool_size; ++i)
            {
               io_service_ptr io_service(new boost::asio::io_service);
               io_services_.push_back(io_service);
            }
         }

         /// 析构
         ~io_service_pool()
         {
            stop();

            // 析构所有 work.
            for (std::size_t i = 0; i < work_.size(); ++i)
               work_[i].reset();
            work_.clear();

            // 析构 io_service pool.
            for (std::size_t i = 0; i < io_services_.size(); ++i)
               io_services_[i].reset();
            io_services_.clear();
         }

         std::size_t size()
         {
            return io_services_.size();
         }

         /// 以非阻塞方式运行 io_service
         void start()
         {
            start(false);
         }

         /// 以阻塞方式运行 io_service
         void run()
         {
            start(true);
         }

         /// 停止io_service池
         void stop()
         {
            if (threads_.size() == 0)
               return;

            ////　使用析构 work方式,令所有　io_service可以正常结束
            for (std::size_t i = 0; i < work_.size(); ++i)
               work_[i].reset();
            work_.clear(); //　清空work容器

            if (!block_)
               wait();
         }

         /// 获取一个io_service
         boost::asio::io_service& get_io_service()
         {
            boost::asio::io_service& io_service =
                     *io_services_[next_io_service_];
            if (++next_io_service_ == io_services_.size())
               next_io_service_ = 0;
            return io_service;
         }

         /// 获取指定io_service
         boost::asio::io_service& get_io_service(std::size_t offset)
         {
            if (offset < io_services_.size())
               return *io_services_[offset];
            else
               return get_io_service();
         }

      private:
         /// 通知池中所有线程结束
         void wait()
         {
            if (threads_.size() == 0)
               return;

            // 等待通知池中所有线程结束
            for (std::size_t i = 0; i < threads_.size(); ++i)
               threads_[i]->join();

            // 清理线程数组
            threads_.clear();
         }

         /// 启动 io_service pool中的所有实例
         void start(bool block)
         {
            if (threads_.size() != 0)
               return;

            // 初始化所有　io_service 为后面的 run() 调用准备
            for (std::size_t i = 0; i < io_services_.size(); ++i)
            {
               io_services_[i]->reset();
            }

            for (std::size_t i = 0; i < io_services_.size(); ++i)
            {
               /// 为保证每个 io_service执行完 run()后不会结束,给每个io_service分配一个work,存放在池中
               work_ptr work(
                        new boost::asio::io_service::work(*io_services_[i]));
               work_.push_back(work);

               /// 创建线程专门执行 io_service的run()后继续监控响应,存入到池中
               thread_ptr thread(
                        new boost::thread(
                                 boost::bind(&boost::asio::io_service::run,
                                          io_services_[i])));
               threads_.push_back(thread);
            }

            block_ = block;

            if (block)
               wait();
         }

      private:
         typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;
         typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;
         typedef boost::shared_ptr<boost::thread> thread_ptr;

         /// 存储io_services
         std::vector<io_service_ptr> io_services_;

         /// 存储work
         std::vector<work_ptr> work_;

         /// 存储threads
         std::vector<thread_ptr> threads_;

         /// 下个连接将使用的io_service
         std::size_t next_io_service_;

         /// start()调用时的阻塞标识
         bool block_;
   };

} // namespace bas

#endif // BAS_IO_SERVICE_POOL_HPP
