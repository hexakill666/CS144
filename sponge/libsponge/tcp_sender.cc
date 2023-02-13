#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(
    const size_t capacity, const uint16_t retx_timeout, 
    const std::optional<WrappingInt32> fixed_isn
    )
    : _is_syn(false)
    , _is_fin(false)
    , _rto(retx_timeout)
    , _time_elapse(0)
    , _outgoing_byte_size(0)
    , _pre_window_size(1)
    , _consecutive_retransmission(0)
    , _outgoing_seg()
    , _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) 
    {}

uint64_t TCPSender::bytes_in_flight() const { 
    return _outgoing_byte_size;
}

void TCPSender::fill_window() {
    size_t cur_window_size = _pre_window_size ? _pre_window_size : 1;
    while(_outgoing_byte_size < cur_window_size){
        TCPSegment seg;
        if(!_is_syn){
            seg.header().syn = true;
            _is_syn = true;
        }
        seg.header().seqno = next_seqno();
        size_t payload_size = min(
            cur_window_size - _outgoing_byte_size - seg.header().syn, 
            TCPConfig::MAX_PAYLOAD_SIZE
        );
        string payload = _stream.read(payload_size);
        if(!_is_fin && _stream.eof() && payload.size() + _outgoing_byte_size < cur_window_size){
            seg.header().fin = true;
            _is_fin = true;
        }
        seg.payload() = Buffer(move(payload));
        if(seg.length_in_sequence_space() == 0){
            break;
        }
        if(_outgoing_seg.empty()){
            _rto = _initial_retransmission_timeout;
            _time_elapse = 0;
        }
        _segments_out.push(seg);
        _outgoing_seg.push(make_pair(_next_seqno, seg));
        _outgoing_byte_size += seg.length_in_sequence_space();
        _next_seqno += seg.length_in_sequence_space();
        if(seg.header().fin){
            break;
        }
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { 
    uint64_t abs_seqno = unwrap(ackno, _isn, _next_seqno);
    if(abs_seqno > _next_seqno){
        return;
    }
    while(!_outgoing_seg.empty()){
        TCPSegment& seg = _outgoing_seg.front().second;
        if(_outgoing_seg.front().first + seg.length_in_sequence_space() <= abs_seqno){
            _outgoing_byte_size -= seg.length_in_sequence_space();
            _outgoing_seg.pop();
            _rto = _initial_retransmission_timeout;
            _time_elapse = 0;
        }
        else{
            break;
        }
    }
    _consecutive_retransmission = 0;
    _pre_window_size = window_size;
    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
    _time_elapse += ms_since_last_tick;
    if(!_outgoing_seg.empty() && _time_elapse >= _rto){
        if(_pre_window_size > 0){
            _rto <<= 1;
        }
        _time_elapse = 0;
        _segments_out.push(_outgoing_seg.front().second);
        ++_consecutive_retransmission;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { 
    return _consecutive_retransmission; 
}

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    _segments_out.push(seg);
}