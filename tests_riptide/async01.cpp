#include <iostream>
#include <functional>
using namespace std;
using namespace std::placeholders;

#include <rtac_asio/Stream.h>
using namespace rtac::asio;

const uint8_t cmd[] = {0x90, 0x00};
uint8_t resp[2];
Stream::Ptr serial;

void write_callback(const boost::system::error_code& err, std::size_t count)
{
    cout << "Written " << count << endl << flush;
    std::this_thread::sleep_for(250ms);
    serial->async_write(2, cmd, write_callback);
}

void read_callback(const boost::system::error_code& err, std::size_t count)
{
    //serial->async_write(2, cmd, write_callback);

    cout << "Read " << count << " : "
         << (unsigned int)resp[0]
         << (unsigned int)resp[1] << endl << flush;

    serial->async_read(2, resp, read_callback);
}

int main()
{
    serial = Stream::CreateSerial("/dev/POLOLU", 115200);
    serial->start();
    serial->enable_io_dump("rx.dump", "tx.dump");

    serial->async_read(2, resp, read_callback);
    serial->async_write(2, cmd, write_callback);

    getchar();

    return 0;
}

