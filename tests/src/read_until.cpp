#include <iostream>
#include <functional>
using namespace std;
using namespace std::placeholders;

#include <rtac_asio/Stream.h>
using namespace rtac::asio;

std::string msg = "Hello there !\n";

int main()
{
    std::string data(1024, '\0');

    auto stream = Stream::CreateSerial("/dev/ttyACM0", 115200);
    stream->start();
    std::cout << "Started" << std::endl;
    
    //while(1) {
    for(int i = 0; i < 5; i++) {
        getchar();
        stream->write(msg.size(), (const uint8_t*)msg.c_str());
        //std::cout << "Read " << stream->read(msg.size(), (uint8_t*)data.c_str())
        //          << " bytes." << std::endl;
        std::cout << "Read " << stream->read_until(data.size(), (uint8_t*)data.c_str(), '\n', 1000)
                  << " bytes." << std::endl << std::flush;
    }

    return 0;
}
