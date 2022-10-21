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
    std::cout << "Got data (" << count << " bytes) : ";
    for(int i = 0; i < count; i++) {
        cout << (*data)[i];
    }
    cout << endl << flush;
    stream->async_read_some(data->size(), (uint8_t*)data->c_str(),
                            std::bind(&read_callback, stream, data, _1, _2));
}

int main()
{
    std::string data(1024, '\0');

    auto stream = Stream::CreateSerial("/dev/ttyACM0", 115200);
    stream->enable_io_dump();

    stream->async_read_some(data.size(), (uint8_t*)data.c_str(),
                            std::bind(&read_callback, stream, &data, _1, _2));

    std::cout << "Started" << std::endl;
    
    //while(1) {
    for(int i = 0; i < 5; i++) {
        getchar();
        stream->async_write_some(msg.size(),
                                 (const uint8_t*)msg.c_str(),
                                 &write_callback);
    }

    return 0;
}
