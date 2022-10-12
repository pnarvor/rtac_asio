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

    //auto serial = Stream::Create(SerialStream::Create(service, "/dev/ttyACM0"));
    auto serial = Stream::CreateSerial("/dev/ttyACM0");

    std::cout << "Started" << std::endl;

    std::cout << "Read " << serial->read(msg.size(), (uint8_t*)data.c_str(), 1000)
              << std::endl;
    
    //while(1) {
    for(int i = 0; i < 10; i++) {
        getchar();
        std::cout << "Write "
                  << serial->write(msg.size(), (const uint8_t*)msg.c_str(), 1000)
                  << std::endl << std::flush;
        std::cout << "Read "
                  << serial->read(msg.size(), (uint8_t*)data.c_str()) 
                  << std::endl << std::flush;
        std::cout << "Data read : '" << data << "'" << std::endl;
    }

    return 0;
}
