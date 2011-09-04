#include <math.h>
#include <iostream>
#include "Subjects1-6.h"
#include "Objects.h"
#include "Node.h"
#include "Supervisor.h"

static const uint32_t s = 2654435769;


double mult_hash(int kk)
{
    uint32_t i = 15;  // m = 2^i
    uint64_t k = static_cast<uint32_t>(kk);
    uint64_t t = s * k & 0x00000000FFFFFFFF;
    t = t >> (32 - i);

    return t/static_cast<double>(2 << 14);
}

int main()
{

}

