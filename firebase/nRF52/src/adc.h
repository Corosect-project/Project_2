#ifndef E18FF3AC_3019_465D_95D6_48359CCB2FB3
#define E18FF3AC_3019_465D_95D6_48359CCB2FB3

#include <zephyr/drivers/adc.h>

int init_channel(const struct adc_channel_cfg *chan);
int read(const struct adc_sequence *seq);

#endif /* E18FF3AC_3019_465D_95D6_48359CCB2FB3 */
