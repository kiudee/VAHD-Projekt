#include <iostream>
#include <math.h>
#include "Subjects1-6.h"
#include "Objects.h"
#include "Node.h"
#include "Supervisor.h"

typedef unsigned long uint32;
typedef unsigned long long uint64;
static const uint32 s = 2654435769;


double mult_hash(int kk)
{
    uint32 i = 15;// m = 2^i
    uint64 k = (uint32) kk;
    uint64 t = s * k & 0x00000000FFFFFFFF;
    t = t >> (32 - i);

    return t/(double)(2 << 14);
}

int main()
{

}

