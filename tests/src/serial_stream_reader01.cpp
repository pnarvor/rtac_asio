#include <iostream>
#include <functional>
using namespace std;
using namespace std::placeholders;

#include <rtac_asio/AsyncService.h>
#include <rtac_asio/SerialStream.h>
#include <rtac_asio/StreamReader.h>
#include <rtac_asio/StreamWriter.h>
using namespace rtac::asio;

std::string msg = "Hello there !\n";

void write_callback(const SerialStream::ErrorCode& err,
                    std::size_t writeCount)
{
    std::cout << "Wrote data (" << writeCount << " bytes)." << std::endl;
}

void read_callback(StreamReader::Ptr reader,
                   std::string* data,
                   const SerialStream::ErrorCode& err,
                   std::size_t count)
{
    std::cout << "Got data (" << count << " bytes) : ";
    for(int i = 0; i < count; i++) {
        cout << (*data)[i];
    }
    cout << endl << flush;
    reader->async_read_some(data->size(), (uint8_t*)data->c_str(),
                            std::bind(&read_callback, reader, data, _1, _2));
}

int main()
{
    std::string data(1024, '\0');

    auto service = AsyncService::Create();
    auto serial = SerialStream::Create(service, "/dev/ttyACM0");
    auto reader = StreamReader::Create(serial);
    auto writer = StreamWriter::Create(serial);

    reader->async_read_some(data.size(), (uint8_t*)data.c_str(),
                            std::bind(&read_callback, reader, &data, _1, _2));

    service->start();
    std::cout << "Started" << std::endl;
    
    while(1) {
        getchar();
        writer->async_write_some(msg.size(),
                                 (const uint8_t*)msg.c_str(),
                                 &write_callback);
        std::cout << "Service running ? : " << !service->stopped() << std::endl;
    }

    service->stop();

    return 0;
}
