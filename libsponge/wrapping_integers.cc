#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    return WrappingInt32{static_cast<uint32_t>(n) + isn.raw_value()};
}
 
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    auto low32bit = static_cast<uint32_t>(n - isn);
    auto high32bit1 = (checkpoint + (1 << 31)) & 0xFFFFFFFF00000000;
    auto high32bit2 = (checkpoint - (1 << 31)) & 0xFFFFFFFF00000000;
    auto res1 = low32bit | high32bit1, res2 = low32bit | high32bit2;
    if (max(res1, checkpoint) - min(res1, checkpoint) < max(res2, checkpoint) - min(res2, checkpoint)) 
        return res1;
    return res2;
}
