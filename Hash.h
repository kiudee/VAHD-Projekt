#ifndef HASH_H_
#define HASH_H_

#include <cstdint>

static const uint32_t s = 2654435769;

double h(int kk)
{
    uint32_t i = 15;  // m = 2^i
    uint64_t k = static_cast<uint32_t>(kk);
    uint64_t t = s * k & 0x00000000FFFFFFFF;
    t = t >> (32 - i);

    return t/static_cast<double>(2 << 14);
}

//TODO extra hash function for data objects?
double g(int kk)
{
    return h(kk);
}

#endif // HASH_H_
