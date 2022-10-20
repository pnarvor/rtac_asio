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



#ifndef _DEF_RTAC_ASIO_TCP_CLIENT_STREAM_H_
#define _DEF_RTAC_ASIO_TCP_CLIENT_STREAM_H_

#include <memory>

#include <boost/asio/ip/udp.hpp>

#include <rtac_asio/AsyncService.h>
#include <rtac_asio/StreamInterface.h>

namespace rtac { namespace asio {

/**
 * The main purpose of this class is to buffer TCP packets.
 *
 * A read on a boost::asio UDP socket will lose some data if the whole packet
 * is not read fully at once. This class buffers UDP data until read or flushed
 * by the user.
 *
 * The default buffer size of 1024 should be safe enough for any UDP packet
 * (but please check this in your own application).
 */
class TCPClientStream : public StreamInterface
{
    public:

    using Ptr      = std::shared_ptr<TCPClientStream>;
    using ConstPtr = std::shared_ptr<const TCPClientStream>;

    using ErrorCode = StreamInterface::ErrorCode;
    using Callback  = StreamInterface::Callback;

    using Socket       = boost::asio::ip::tcp::socket;
    using EndPoint     = boost::asio::ip::tcp::endpoint;

    protected:

    std::unique_ptr<Socket> socket_;
    EndPoint                remote_;

    TCPClientStream(AsyncService::Ptr service,
                    const std::string& remoteIP,
                    uint16_t remotePort);

    void receive_continue(std::size_t bufferSize,
                          uint8_t* buffer,
                          Callback callback,
                          const ErrorCode& err,
                          std::size_t received);

    public:

    static Ptr Create(AsyncService::Ptr service,
                      const std::string& remoteIP,
                      uint16_t remotePort);

    const EndPoint& remote() const { return remote_; }
    
    void close();
    void reset(const EndPoint& remote);
    void reset();
    void flush();

    void async_read_some(std::size_t bufferSize,
                         uint8_t*    buffer,
                         Callback    callback);
    void async_write_some(std::size_t    count,
                          const uint8_t* data,
                          Callback       callback);
};

} //namespace asio
} //namespace rtac

#endif //_DEF_RTAC_ASIO_TCP_CLIENT_STREAM_H_
