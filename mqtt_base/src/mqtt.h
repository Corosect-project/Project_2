#ifndef D2AE867A_AFDC_452A_AB8B_77B11E1601C5
#define D2AE867A_AFDC_452A_AB8B_77B11E1601C5

#include <zephyr/kernel.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/net/socket.h>

extern bool connected;
extern struct zsock_pollfd fds[1];

struct mqtt_client *init_mqtt(const char *ip, uint32_t port);
int send_message(char *msg, char *topic);

#endif /* D2AE867A_AFDC_452A_AB8B_77B11E1601C5 */
