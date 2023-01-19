#include <iostream>
#include <functional>
using namespace std;
using namespace std::placeholders;

#include <rtac_asio/Stream.h>
using namespace rtac::asio;

const uint8_t cmd[] = {0x90, 0x00};

void write_callback(Stream::Ptr stream, const boost::system::error_code& err, std::size_t count)
{
    std::cout << "Wrote " << count << "characters" << std::endl << std::flush;
    stream->async_write(2, cmd, std::bind(write_callback, stream, _1, _2));
}

int main()
{
    auto serial = Stream::CreateSerial("/dev/POLOLU", 115200);
    serial->start();
    serial->enable_io_dump("rx.dump", "tx.dump");

    //write_callback(serial, boost::system::error_code(), 0);

    uint8_t resp[2];

    unsigned int count = 0;
    while(1) {
        //serial->write(2, cmd);
        //serial->read(2, resp);
        if(serial->write(2, cmd) != 2) throw std::runtime_error("Write error.");
        if(serial->read(2, resp) != 2) throw std::runtime_error("Read error.");

        cout << count++ << " got read : " 
             << (unsigned int)resp[0] << " "
             << (unsigned int)resp[1] << endl << flush;
    }

    return 0;
}

