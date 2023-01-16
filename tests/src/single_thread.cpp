#include <iostream>
#include <functional>
using namespace std;
using namespace std::placeholders;

#include <rtac_asio/AsyncService.h>
#include <rtac_asio/Stream.h>
using namespace rtac::asio;

std::string msg = "Hello there !\n";

void write_callback(const SerialStream::ErrorCode& /*err*/,
                    std::size_t writeCount)
{
    std::cout << "Wrote data (" << writeCount << " bytes)." << std::endl;
}

void read_callback(Stream::Ptr stream,
                   std::string* data,
                   const SerialStream::ErrorCode& /*err*/,
                   std::size_t count)
{
    if(count > 0) {
        std::cout << "Got data (" << count << " bytes) : '";
        for(std::size_t i = 0; i < count; i++) {
            std::cout << (*data)[i];
        }
        std::cout << "'" << std::endl;
    }
    else {
        std::cout << "Got no data (timeout reached ?)" << std::endl;
    }
    stream->async_read(msg.size(), (uint8_t*)data->c_str(),
                       std::bind(&read_callback, stream, data, _1, _2));

    stream->async_write(msg.size(), (const uint8_t*)msg.c_str(),
                        &write_callback);
}

int main()
{
    std::string data(1024, '\0');

    auto stream = Stream::CreateSerial("/dev/ttyACM0", 115200);

    stream->async_read(msg.size(), (uint8_t*)data.c_str(),
                       std::bind(&read_callback, stream, &data, _1, _2));
    stream->async_write(msg.size(), (const uint8_t*)msg.c_str(),
                        &write_callback);

    std::cout << "Started" << std::endl;
    stream->run();
    std::cout << "Stopped" << std::endl;

    return 0;
}
