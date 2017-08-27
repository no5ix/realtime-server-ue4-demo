/**
 * @file
 * @brief 数据缓冲区
 *
 */

#ifndef BAS_IO_BUFFER_HPP
#define BAS_IO_BUFFER_HPP

#include <boost/assert.hpp>
#include <memory>
#include <vector>

namespace bas
{

   /// 收发用的Buffer
   class io_buffer
   {
      public:
         /// 定义存储的数据类型
         typedef unsigned char byte_type;

         /// 定义标识位置的数据类型
         typedef std::size_t size_type;

         /// 构造
         explicit io_buffer(size_type capacity) :
            begin_offset_(0), end_offset_(0), buffer_(capacity)
         {
         }

         /// 清空buffer.
         void clear()
         {
            begin_offset_ = 0;
            end_offset_ = 0;
         }

         /// 取得未读数据的头位置
         byte_type* data()
         {
            return &buffer_[0] + begin_offset_;
         }

         /// 取得未读数据的头位置
         const byte_type* data() const
         {
            return &buffer_[0] + begin_offset_;
         }

         /// 检查是否有未读数据
         bool empty() const
         {
            return begin_offset_ == end_offset_;
         }

         /// 取得未读数据长度
         size_type size() const
         {
            return end_offset_ - begin_offset_;
         }

         /// 重置buffer的大小
         void resize(size_type length)
         {
            BOOST_ASSERT(length <= capacity());
            if (begin_offset_ + length <= capacity())
            {
               end_offset_ = begin_offset_ + length;
            }
            else
            {
               //把数据从后前往前面的头移过去
               memmove(&buffer_[0], &buffer_[0] + begin_offset_, size());
               end_offset_ = length;
               begin_offset_ = 0;
            }
         }

         /// 取得buffer的最大容量值
         size_type capacity() const
         {
            return buffer_.size();
         }

         /// 取得buffer 的空闲值
         size_type space() const
         {
            return capacity() - end_offset_;
         }

         /// 跳过头部部分数据
         void consume(size_type count)
         {
            BOOST_ASSERT(count <= size());
            begin_offset_ += count;
            if (empty())
            {
               clear();
            }
         }

         /// 增加尾部长度
         void produce(size_type count)
         {
            BOOST_ASSERT(count <= space());
            end_offset_ += count;
         }

         /// 移除头部无用数据
         void crunch()
         {
            if (begin_offset_ != 0)
            {
               if (begin_offset_ != end_offset_)
               {
                  memmove(&buffer_[0], &buffer_[0] + begin_offset_, size());
                  end_offset_ = size();
                  begin_offset_ = 0;
               }
               else
               {
                  clear();
               }
            }
         }

      private:
         // 未读数据的开始位置
         size_type begin_offset_;

         // 未读数据的结束位置
         size_type end_offset_;

         // 缓冲区
         std::vector<byte_type> buffer_;
   };

} // namespace bas

#endif // BAS_IO_BUFFER_HPP
