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



#ifndef _DEF_RTAC_ASIO_SERIAL_STREAM_H_
#define _DEF_RTAC_ASIO_SERIAL_STREAM_H_

#include <memory>

#include <boost/asio/streambuf.hpp>
#include <boost/asio/serial_port.hpp>

#include <rtac_asio/AsyncService.h>
#include <rtac_asio/StreamInterface.h>

namespace rtac { namespace asio {

class SerialStream : public StreamInterface
{
    public:

    using Ptr      = std::shared_ptr<SerialStream>;
    using ConstPtr = std::shared_ptr<const SerialStream>;

    using ErrorCode = StreamInterface::ErrorCode;
    using Callback  = StreamInterface::Callback;

    using SerialPort = boost::asio::serial_port;
    using ReadBuffer = boost::asio::streambuf;

    enum FlushType {
        FlushReceive = TCIFLUSH,
        FlushSend    = TCOFLUSH,
        FlushBoth    = TCIOFLUSH
    };

    struct Parameters
    {
        SerialPort::baud_rate      baudrate;
        SerialPort::character_size characterSize;
        SerialPort::parity         parity;
        SerialPort::flow_control   flowControl;
        SerialPort::stop_bits      stopBits;

        Parameters(unsigned int bauds = 115200) :
            baudrate(bauds),
            characterSize(8),
            parity(SerialPort::parity::none),
            flowControl(SerialPort::flow_control::none),
            stopBits(SerialPort::stop_bits::one)
        {}
    };

    protected:

    std::string device_;
    Parameters  parameters_;

    std::unique_ptr<SerialPort> serial_;

    SerialStream(AsyncService::Ptr service,
                 const std::string& device,
                 const Parameters& params);

    public:

    ~SerialStream();

    static Ptr Create(AsyncService::Ptr service,
                      const std::string& device,
                      const Parameters& params = Parameters());

    const std::string& device()     const { return device_;     }
    const Parameters&  parameters() const { return parameters_; }

    void close();
    void reset(const Parameters& params);
    void reset();
    ErrorCode flush(FlushType flushType);
    void flush() { this->flush(FlushBoth); }
    bool is_open() const;

    void async_read_some(std::size_t bufferSize,
                         uint8_t*    buffer,
                         Callback    callback);
    void async_write_some(std::size_t    count,
                          const uint8_t* data,
                          Callback       callback);
};

} //namespace asio
} //namespace rtac

#endif //_DEF_RTAC_ASIO_SERIAL_STREAM_H_
