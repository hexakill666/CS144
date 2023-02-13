#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) 
    : _first_unassemble_index(0)
    , _total_unassemble_size(0)
    , _eof(false)
    , _unassemble_buffer(capacity, '\0')
    , _is_taken(capacity, false)
    , _output(capacity)
    , _capacity(capacity) { }

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if(eof){
        _eof = true;
    }
    if(data.empty() && _eof && empty()){
        _output.end_input();
        return;
    }
    if(index >= _first_unassemble_index + _capacity){
        return;
    }
    if(index >= _first_unassemble_index){
        size_t offset = index - _first_unassemble_index;
        size_t real_size = min(data.size(), _capacity - _output.buffer_size() - offset);
        if(real_size < data.size()){
            _eof = false;
        }
        for(size_t a = 0; a < real_size; a++){
            if(_is_taken[offset + a]){
                continue;
            }
            _unassemble_buffer[offset + a] = data[a];
            _is_taken[offset + a] = true;
            _total_unassemble_size++;
        }
    }
    else if(index + data.size() > _first_unassemble_index){
        size_t offset = _first_unassemble_index - index;
        size_t real_size = min(data.size() - offset, _capacity - _output.buffer_size());
        if(real_size < data.size() - offset){
            _eof = false;
        }
        for(size_t a = 0; a < real_size; a++){
            if(_is_taken[a]){
                continue;
            }
            _unassemble_buffer[a] = data[offset + a];
            _is_taken[a] = true;
            _total_unassemble_size++;
        }
    }
    check_contiguous();
    if(_eof && empty()){
        _output.end_input();
    }
}

void StreamReassembler::check_contiguous(){
    string temp = "";
    while(_is_taken.front()){
        temp += _unassemble_buffer.front();
        _unassemble_buffer.pop_front();
        _is_taken.pop_front();
        _unassemble_buffer.push_back('\0');
        _is_taken.push_back(false);
    }
    if(!temp.empty()){
        _output.write(temp);
        _first_unassemble_index += temp.size();
        _total_unassemble_size -= temp.size();
    }
}

size_t StreamReassembler::unassembled_bytes() const { 
    return _total_unassemble_size;
}

bool StreamReassembler::empty() const { 
    return unassembled_bytes() == 0;
}
