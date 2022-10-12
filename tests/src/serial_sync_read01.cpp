#include <iostream>
#include <functional>
using namespace std;
using namespace std::placeholders;

#include <rtac_asio/AsyncService.h>
#include <rtac_asio/SerialStream.h>
#include <rtac_asio/Stream.h>
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

    Stream serial(SerialStream::Create(service, "/dev/ttyACM0"));

    service->start();
    std::cout << "Started" << std::endl;

    std::cout << "Read " << serial.read(msg.size() + 1, (uint8_t*)data.c_str(), 1000) << std::endl;
    
    while(1) {
        getchar();
        serial.async_write(msg.size() + 1,
                           (const uint8_t*)msg.c_str(),
                           &write_callback);
        std::cout << "Read " << serial.read(msg.size() + 1, (uint8_t*)data.c_str()) << std::endl;
        std::cout << "Data read : '" << data << "'" << std::endl;
    }

    service->stop();

    return 0;
}
