#include <iostream>
#include <functional>
using namespace std;
using namespace std::placeholders;

#include <rtac_asio/Stream.h>
using namespace rtac::asio;

std::string msg = "Hello there !\n";

void write_callback(const SerialStream::ErrorCode& err,
                    std::size_t writeCount)
{
    std::cout << "Wrote data (" << writeCount << " bytes)." << std::endl;
}

void read_callback(Stream::Ptr stream,
                   std::string* data,
                   const SerialStream::ErrorCode& err,
                   std::size_t count)
{
    if(count > 0) {
        std::cout << "Got data (" << count << " bytes) : '";
        for(int i = 0; i < count; i++) {
            std::cout << (*data)[i];
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

    auto stream = Stream::CreateSerial("/dev/ttyACM0", 115200);
    stream->start();
    stream->enable_io_dump();

    if(!stream->async_read_until(data.size(), (uint8_t*)data.c_str(), '\n',
                                 std::bind(&read_callback, stream, &data, _1, _2)))
    {
        throw std::runtime_error("Read could not start.");
    }
    std::cout << "Started" << std::endl;
    
    //getchar();
    while(1) {
    //for(int i = 0; i < 10; i++) {
        stream->async_write(msg.size(), (const uint8_t*)msg.c_str(),
                            &write_callback);
        std::cout << "====== Write loop" << std::endl << std::flush;
        getchar();
    }

    return 0;
}
