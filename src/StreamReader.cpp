/**
 * Boost Software License - Version 1.0 - August 17th, 2003
 * 
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 * 
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */



#include <rtac_asio/StreamReader.h>

using namespace std::placeholders;

namespace rtac { namespace asio {

StreamReader::StreamReader(StreamInterface::Ptr stream) :
    stream_(stream),
    readCounter_(0),
    readId_(0),
    timer_(stream_->service()->service())
{}

StreamReader::~StreamReader()
{
    this->disable_dump();
}

StreamReader::Ptr StreamReader::Create(StreamInterface::Ptr stream)
{
    return Ptr(new StreamReader(stream));
}

void StreamReader::flush()
{
    readBuffer_.consume(readBuffer_.size());
    stream_->flush();
}

void StreamReader::reset()
{
    stream_->reset();
}

void StreamReader::enable_dump(const std::string& filename, bool appendMode)
{
    if(rxDump_.is_open()) {
        std::cerr << "rx dump already enabled. Close before reopen." << std::endl;
        return;
    }
    
    auto mode = std::ofstream::out;
    if(appendMode) {
        mode = std::ofstream::app;
    }
    rxDump_.open(filename, mode);
    if(!rxDump_.is_open()) {
        std::cerr << "rtac_asio : Could not open file "
                  << filename << " for writing." << std::endl;
    }
}

void StreamReader::disable_dump()
{
    if(rxDump_.is_open()) {
        rxDump_.close();
    }
}

void StreamReader::dump_callback(Callback callback, uint8_t* data,
                                 const ErrorCode& err, std::size_t readCount)
{
    if(!err && this->dump_enabled()) {
        for(std::size_t i = 0; i < readCount; i++) {
            std::cout << data[i];
            rxDump_ << data[i];
        }
        rxDump_.flush();
    }
    callback(err, readCount);
}

bool StreamReader::new_read(std::size_t requestedSize, uint8_t* data, Callback callback,
                            unsigned int timeoutMillis)
{
    std::lock_guard<std::mutex> lock(readMutex_);
    if(readId_ != 0) {
        // device busy
        return false;
    }
    
    readCounter_++;
    readId_        = readCounter_;
    requestedSize_ = requestedSize;
    processed_     = 0;
    dst_           = data;
    callback_      = callback;

    if(timeoutMillis > 0) {
        timer_.expires_from_now(Millis(timeoutMillis));
        timer_.async_wait(std::bind(&StreamReader::timeout_reached, this, readId_, _1));
    }

    return true;
}

void StreamReader::finish_read(const ErrorCode& err)
{
    std::lock_guard<std::mutex> lock(readMutex_);
    if(readId_ == 0) {
        // No read to end (should not happend)
        return;
    }
    readId_ = 0;
    // This calls the user callback in an executor loop (avoid potential
    // deadlock with the readMutex_if the user callback asks for another read).
    stream_->service()->post(std::bind(callback_, err, processed_));
    // from this moment, requestedSize_, processed_, dst_ and callback_ are
    // devalidated and available for a new read.
}

/**
 * Checks if the readId associated with the caller checks out with the current
 * one saved in the readId_ attribute.
 *
 * This allows to potentially detect callbacks which are no longer relevant.
 * (For example a read callback might be called after a timeout already
 * happened. This callback needs either to generate an error or to be ignored).
 */
bool StreamReader::readid_ok(unsigned int readId) const
{
    std::lock_guard<std::mutex> lock(readMutex_);
    return readId == readId_;
}

void StreamReader::timeout_reached(unsigned int readId, const ErrorCode& err)
{
    if(!readid_ok(readId)) {
        return;
    }
    // If timeout was reached, a waiter might have been set
    waiterNotified_ = true;
    waiter_.notify_all();
    this->finish_read(ErrorCode()); // error should be timeout but could not
                                    // find how to make one.
}

/**
 * This function reads data from the stream.
 *
 * If the readBuffer_ is not empty, it will read data from the readBuffer_
 * until the requested size is full or the read buffer is empty. It then
 * immediatly calls the user callback without trying to read from the
 * underlying stream read.
 *
 * New data is read from the underlying stream only if the readBuffer_ was
 * empty when this method was called.
 */
void StreamReader::do_read_some(std::size_t count, uint8_t* data,
                                Callback callback)
{
    if(readBuffer_.size() > 0) {
        // readBuffer_ not empty
        std::istream is(&readBuffer_);
        char c = is.get();
        int readCount = 0;
        while(!is.eof() && readCount < count) {
            data[readCount] = c;
            readCount++;
            c = is.get();
        }
        readBuffer_.consume(readCount);
        stream_->service()->post(std::bind(callback, ErrorCode(), readCount));
    }
    else {
        if(!this->dump_enabled()) {
            stream_->async_read_some(count, data, callback);
        }
        else {
            stream_->async_read_some(count, data,
                std::bind(&StreamReader::dump_callback, this, callback, data, _1, _2));
        }
    }
}

bool StreamReader::async_read_some(std::size_t count, uint8_t* data,
                                   Callback callback, unsigned int timeoutMillis)
{
    if(!this->new_read(count, data, callback, timeoutMillis)) {
        return false;
    }

    this->do_read_some(requestedSize_, dst_,
        std::bind(&StreamReader::async_read_some_continue, this, readId_, _1, _2));

    return true;
}

void StreamReader::async_read_some_continue(unsigned int readId,
                                            const ErrorCode& err, std::size_t readCount)
{
    if(!readid_ok(readId)) {
        return;
    }
    processed_ = readCount;
    this->finish_read(err);
}

bool StreamReader::async_read(std::size_t count, uint8_t* data,
                              Callback callback, unsigned int timeoutMillis)
{
    if(!this->new_read(count, data, callback, timeoutMillis)) {
        return false;
    }

    this->do_read_some(requestedSize_, dst_,
        std::bind(&StreamReader::async_read_continue, this, readId_, _1, _2));

    return true;
}

void StreamReader::async_read_continue(unsigned int readId, const ErrorCode& err,
                                 std::size_t readCount)
{
    if(!readid_ok(readId)) {
        // probably a timeout was reached
        return;
    }

    processed_ += readCount;
    if(!err && processed_ < requestedSize_) {
        this->do_read_some(requestedSize_ - processed_, dst_ + processed_,
            std::bind(&StreamReader::async_read_continue, this, readId, _1, _2));
    }
    else {
        this->finish_read(err);
    }
}

std::size_t StreamReader::read(std::size_t count, uint8_t* data,
                               unsigned int timeoutMillis)
{
    std::unique_lock<std::mutex> lock(mutex_); // will release mutex when out of scope

    if(!this->async_read(count, data,
                         std::bind(&StreamReader::read_callback, this, _1, _2),
                         timeoutMillis))
    {
        // device probably busy
        return 0;
    }

    waiterNotified_ = false; // this protects against spurious wakeups.
    waiter_.wait(lock, [&]{ return waiterNotified_; });

    return processed_;
}

void StreamReader::read_callback(const ErrorCode& err, std::size_t readCount)
{
    // finish read was already called through the async_read primitive
    waiterNotified_ = true;
    waiter_.notify_all();
}

bool StreamReader::async_read_until(std::size_t maxSize, uint8_t* data, char delimiter,
                                    Callback callback, unsigned int timeoutMillis)
{
    if(!this->new_read(maxSize, data, callback, timeoutMillis)) {
        return false;
    }
    
    // First checking if delimiter in buffer
    if(readBuffer_.size() > 0) {
        // readBuffer_ not empty
        std::istream is(&readBuffer_);
        char c = is.get();
        while(!is.eof()) {
            dst_[processed_] = c;
            processed_++;
            if(c == delimiter || processed_ >= requestedSize_) {
                // delimiter found or maximum user buffer size reached
                readBuffer_.consume(processed_);
                this->finish_read(ErrorCode());
                return true;
            }
            c = is.get();
        }
        readBuffer_.consume(processed_);
    }

    // if reaching here, readBuffer_ is empty
    this->do_read_some(requestedSize_, dst_,
        std::bind(&StreamReader::async_read_until_continue, this,
                  readId_, delimiter, _1, _2));

    return true;
}

void StreamReader::async_read_until_continue(unsigned int readId,
                                             char delimiter,
                                             const ErrorCode& err,
                                             std::size_t readCount)
{
    if(!readid_ok(readId)) {
        // probably a timeout was reached
        return;
    }

    const uint8_t* data = dst_ + processed_;
    int i = 0;
    for(; i < readCount; i++) {
        if(data[i] == delimiter) {
            // delimiter was found. Saving remaining data in readBuffer_
            i++;
            std::ostream os(&readBuffer_);
            unsigned int toBeCommited = 0;
            for(int j = i; j < readCount; j++) {
                os << data[j];
                toBeCommited++;
            }
            readBuffer_.commit(toBeCommited);
            processed_ += i;
            this->finish_read(err);
            return;
        }
    }

    // delimiter was not found.
    processed_ += i;
    if(err || processed_ >= requestedSize_) {
        this->finish_read(err);
    }
    else {
        // delimiter not found and no error. Continuing read.
        this->do_read_some(requestedSize_ - processed_, dst_ + processed_,
            std::bind(&StreamReader::async_read_until_continue, this,
                      readId, delimiter, _1, _2));
    }
}

std::size_t StreamReader::read_until(std::size_t maxSize, uint8_t* data,
                                     char delimiter, unsigned int timeoutMillis)
{
    std::unique_lock<std::mutex> lock(mutex_); // will release mutex when out of scope

    if(!this->async_read_until(maxSize, data, delimiter,
                               std::bind(&StreamReader::read_callback, this, _1, _2),
                               timeoutMillis))
    {
        // device probably busy
        return 0;
    }

    waiterNotified_ = false; // this protects against spurious wakeups.
    waiter_.wait(lock, [&]{ return waiterNotified_; });

    return processed_;
}

} //namespace asio
} //namespace rtac


