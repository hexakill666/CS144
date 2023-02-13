#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) 
    : _buffer()
    , _capacity_size(capacity)
    , _total_read_size(0)
    , _total_write_size(0)
    , _is_end_input(false) { }

size_t ByteStream::write(const string &data) { 
    size_t write_byte = 0;
    if(input_ended()){
        return write_byte;
    }
    while(write_byte < data.length() && _buffer.size() < _capacity_size){
        _buffer.push_back(data[write_byte++]);
    }
    _total_write_size += write_byte;
    return write_byte;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const { 
    string peek_str;
    for(size_t a = 0; a < len && a < _buffer.size(); a++){
        peek_str.push_back(_buffer[a]);
    }
    return peek_str;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    size_t read_byte = 0;
    while(read_byte < len && !_buffer.empty()){
        _buffer.pop_front();
        read_byte++;
    }
    _total_read_size += read_byte;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) { 
    string read_str = peek_output(len);
    pop_output(len);
    return read_str;
}

void ByteStream::end_input() { 
    _is_end_input = true;
}

bool ByteStream::input_ended() const { 
    return _is_end_input;
}

size_t ByteStream::buffer_size() const { 
    return _buffer.size();
}

bool ByteStream::buffer_empty() const { 
    return _buffer.empty();
}

bool ByteStream::eof() const { 
    return input_ended() && buffer_empty();
}

size_t ByteStream::bytes_written() const { 
    return _total_write_size;
}

size_t ByteStream::bytes_read() const { 
    return _total_read_size;
}

size_t ByteStream::remaining_capacity() const { 
    return _capacity_size - buffer_size();
}
