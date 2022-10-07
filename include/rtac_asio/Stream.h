/**
 * Boost Software License - Version 1.0 - August 17th, 2003
 * 
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 * 
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */



#ifndef _DEF_RTAC_ASIO_STREAM_H_
#define _DEF_RTAC_ASIO_STREAM_H_

#include <chrono>
#include <mutex>
#include <condition_variable>

#include <rtac_asio/StreamInterface.h>

namespace rtac { namespace asio {

class Stream
{
    public:

    using ErrorCode = StreamInterface::ErrorCode;
    using Callback  = StreamInterface::Callback;

    using Clock     = std::chrono::system_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    static constexpr TimePoint NoTimeout = TimePoint

    struct Request {
        int64_t   requestedSize;
        int64_t   processed;
        Callback  handler;

        TimePoint timeoutDate;

        //std::mutex  mutex_;
        //bool        waiterNotified_;
        //std::condition_variable waiter_;
        
        int64_t remaining() const { return requestedSize - processed; }
    };

    struct ReadRequest : public Request {
        uint8_t* data;
    };

    struct WriteRequest : public Request {
        const uint8_t* data;
    };

    protected:

    StreamInterface::ConstPtr stream_;

    mutable ReadRequest  readRequest_;
    mutable WriteRequest writeRequest_;

    void read_request_continue (const ErrorCode& err, std::size_t readCount) const;
    void write_request_continue(const ErrorCode& err, std::size_t writtenCount) const;
    
    public:

    Stream(StreamInterface::ConstPtr stream);

    void async_read_some(std::size_t count, uint8_t* data,
                         Callback callback) const;
    void async_write_some(std::size_t count, const uint8_t* data,
                          Callback callback) const;

    void async_read (std::size_t count, uint8_t* data,       Callback callback) const;
    void async_write(std::size_t count, const uint8_t* data, Callback callback) const;

    //std::size_t read(std::size_t count, uint8_t* data) const;
    //std::size_t read(std::size_t count, uint8_t* data,
    //                 int64_t timeoutMillis) const;
    //

};

} //namespace asio
} //namespace rtac

#endif //_DEF_RTAC_ASIO_STREAM_H_
