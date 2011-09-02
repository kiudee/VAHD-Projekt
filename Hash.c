#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>

static const uint32_t s = 2654435769;
char binary[256];
void dec2bin(uint32_t);
float debruijn0shift(float x);
float debruijn1shift(float x);

uint32_t mult_hash(uint32_t kk, uint32_t i) {
	uint64_t k = kk;
	uint64_t t = s * k & 0x00000000FFFFFFFF;
	t = t >> (32 - i);

	return t;
}

int main(int argc, char **argv) {

	uint32_t l = mult_hash(atoi(argv[1]), 10);
	float r =  1/(float)l;

	float x = 0.123;
	printf("hallo %f" "\n", x);
	int * ptr = (int *) (& x) ;
	*ptr >>= 1 ; // left shift by n bits
	printf("hallo %f" "\n", x);

	r = debruijn0shift(r);


	return 0;
}

void dec2bin(uint32_t decimal) {
	int k = 0, n = 0;
	int remain;
	char temp[256];

	do {

		remain = decimal % 2;
		// whittle down the decimal number
		decimal = decimal / 2;
		// converts digit 0 or 1 to character '0' or '1'
		temp[k++] = remain + '0';
	} while (decimal > 0);

	// reverse the spelling
	while (k >= 0)
		binary[n++] = temp[--k];

	binary[n - 1] = 0; // end with NULL
}

float debruijn0shift(float x){
   int * ptr = (int *) (& x) ;
   *ptr >> 1 ;
   return x;
}

float debruijn1shift(float x){
   x+=1.0;
   int * ptr = (int *) (& x) ;
   *ptr >> 1 ;
   return x;
}


