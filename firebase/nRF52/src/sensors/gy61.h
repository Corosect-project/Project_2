#ifndef E5DAC023_569B_44F6_B347_797B26126EAB
#define E5DAC023_569B_44F6_B347_797B26126EAB

#include <stdint.h>
#include <stddef.h>

int gy61_init();
int gy61_read(uint16_t out_raw[3], size_t out_raw_len, double out[3], size_t out_len);

#endif /* E5DAC023_569B_44F6_B347_797B26126EAB */
