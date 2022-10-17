#include <iostream>
#include <functional>
using namespace std;
using namespace std::placeholders;

#include <rtac_asio/AsyncService.h>
#include <rtac_asio/SerialStream.h>
#include <rtac_asio/StreamReader.h>
using namespace rtac::asio;

std::string msg = "Hello there !\n";

void write_callback(const SerialStream::ErrorCode& err,
                    std::size_t writeCount)
{
    std::cout << "Wrote data (" << writeCount << " bytes)." << std::endl;
}

int main()
{
    std::string data(1024, '\0');

    auto service = AsyncService::Create();
    auto serial = SerialStream::Create(service, "/dev/ttyACM0");
    auto stream = StreamReader::Create(serial);

    service->start();
    std::cout << "Started" << std::endl;
    std::cout << "Service running ? : " << !service->stopped() << std::endl;
    
    while(1) {
        getchar();
        serial->async_write_some(msg.size(),
                                 (const uint8_t*)msg.c_str(),
                                 &write_callback);
        std::cout << "Service running ? : " << !service->stopped() << std::endl;
        //std::cout << "Read " << stream->read(msg.size(), (uint8_t*)data.c_str())
        //          << " bytes." << std::endl;
        std::cout << "Read " << stream->read_until(data.size(), (uint8_t*)data.c_str(), '\n', 1000)
                  << " bytes." << std::endl;
    }

    service->stop();

    return 0;
}
