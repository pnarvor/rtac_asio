#include <iostream>
#include <functional>
#include <thread>
using namespace std;
using namespace std::placeholders;

#include <rtac_asio/StringStream.h>
#include <rtac_asio/Stream.h>
using namespace rtac::asio;

//std::string msg = "Hello there !\nHello there !\n";
unsigned int N = 0;

void write_callback(Stream::Ptr stream,
                    const Stream::ErrorCode& /*err*/,
                    std::size_t writeCount)
{
    std::ostringstream oss;
    for(unsigned int n = 0; n < N; n++) {
        oss << "Hello there ! " << n << "\r\n";
    }
    std::string msg = oss.str();
    //std::cout << "Wrote data (" << writeCount << " bytes)." << std::endl;
    stream->async_write(msg.size(), (const uint8_t*)msg.c_str(),
                        std::bind(write_callback, stream, _1, _2));
    if(++N == 100) {
        N = 0;
    }
    std::this_thread::sleep_for(100ms);
}

void read_callback(Stream::Ptr stream,
                   std::string* data,
                   const SerialStream::ErrorCode& /*err*/,
                   std::size_t count)
{
    if(count > 0) {
        std::cout << "Got data (" << count << " bytes) : '";
        for(std::size_t i = 0; i < count; i++) {
            if((*data)[i] == '\n') {
                std::cout << "[LF]";
            }
            else if((*data)[i] == '\r') {
                std::cout << "[CR]";
            }
            else {
                std::cout << (*data)[i];
            }
        }
        std::cout << "'" << std::endl;
    }
    else {
        std::cout << "Got no data (timeout reached ?)" << std::endl;
    }
    if(!stream->async_read_until(data->size(), (uint8_t*)data->c_str(), '\n',
                             std::bind(&read_callback, stream, data, _1, _2)))
                             //std::bind(&read_callback, stream, data, _1, _2), 1000))
    {
        std::cout << "Read could not continue." << std::endl;
        throw std::runtime_error("Read could not continue.");
    }
}


int main()
{
    std::string data(1024, '\0');

    auto service = AsyncService::Create();
    auto stream = Stream::Create(StringStream::Create(service, 8192));

    //stream->async_write(msg.size(), (const uint8_t*)msg.c_str(),
    //                    std::bind(write_callback, stream, _1, _2));
    write_callback(stream, Stream::ErrorCode(), 0);

    if(!stream->async_read_until(data.size(), (uint8_t*)data.c_str(), '\n',
                                 std::bind(&read_callback, stream, &data, _1, _2)))
    {
        throw std::runtime_error("Read could not start.");
    }
    std::cout << "Started" << std::endl;

    //stream->start();
    //getchar();
    stream->run();

    return 0;
}
