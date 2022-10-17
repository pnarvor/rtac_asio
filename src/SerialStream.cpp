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



#include <rtac_asio/SerialStream.h>

namespace rtac { namespace asio {


SerialStream::SerialStream(AsyncService::Ptr service,
                           const std::string& device,
                           const Parameters& params) :
    StreamInterface(service),
    device_(device),
    parameters_(params),
    serial_(service->service(), device_)
{
    this->reset();
}

SerialStream::Ptr SerialStream::Create(AsyncService::Ptr service, 
                                       const std::string& device,
                                       const Parameters& params)
{
    return Ptr(new SerialStream(service, device, params));
}

void SerialStream::reset(const Parameters& params)
{
    parameters_ = params;
    this->reset();
}

void SerialStream::reset()
{
    serial_.set_option(parameters_.baudrate);
    serial_.set_option(parameters_.characterSize);
    serial_.set_option(parameters_.parity);
    serial_.set_option(parameters_.flowControl);
    serial_.set_option(parameters_.stopBits);

    if(auto err = this->flush(FlushBoth)) {
        std::ostringstream oss;
        oss << "Error on serial port flushing : " << err;
        throw std::runtime_error(oss.str());
    }
}

SerialStream::ErrorCode SerialStream::flush(FlushType flushType)
{
    if(::tcflush(serial_.lowest_layer().native_handle(), flushType) == 0) {
        return ErrorCode();
    }
    else {
        return ErrorCode(errno,
            boost::asio::error::get_system_category());
    }
}

void SerialStream::async_read_some(std::size_t bufferSize,
                                   uint8_t* buffer,
                                   Callback callback)
{
    serial_.async_read_some(boost::asio::buffer(buffer, bufferSize), callback);
}

void SerialStream::async_write_some(std::size_t count,
                                    const uint8_t* data,
                                    Callback callback)
{
    serial_.async_write_some(boost::asio::buffer(data, count), callback);
}

} //namespace asio
} //namespace rtac
