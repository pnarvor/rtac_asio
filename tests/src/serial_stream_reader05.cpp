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

int main()
{
    std::string data(1024, '\0');

    auto service = AsyncService::Create();
    auto serial = SerialStream::Create(service, "/dev/ttyACM0");
    auto reader = StreamReader::Create(serial);
    auto writer = StreamWriter::Create(serial);

    service->start();
    std::cout << "Started" << std::endl;
    std::cout << "Service running ? : " << !service->stopped() << std::endl;
    
    while(1) {
        getchar();
        writer->write(msg.size(), (const uint8_t*)msg.c_str());
        std::cout << "Service running ? : " << !service->stopped() << std::endl;
        //std::cout << "Read " << reader->read(msg.size(), (uint8_t*)data.c_str())
        //          << " bytes." << std::endl;
        std::cout << "Read " << reader->read_until(data.size(), (uint8_t*)data.c_str(), '\n', 1000)
                  << " bytes." << std::endl;
    }

    service->stop();

    return 0;
}
