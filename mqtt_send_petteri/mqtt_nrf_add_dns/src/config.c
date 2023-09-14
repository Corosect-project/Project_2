#include "config.h"

#include <zephyr/logging/log.h>
#include <zephyr/net/net_config.h>
#include <zephyr/net/net_if.h>

LOG_MODULE_DECLARE(app, LOG_LEVEL_DBG);

static struct in6_addr my_addr;

/**
 * @brief Initialize network interface for later use
 *
 * @param ip this device's ip address
 */
void init_network(char *ip) {
  int err;

  struct net_if *iface = net_if_get_default();
  if (!iface) {
    LOG_ERR("Didn't get interface");
    return;
  }

  if (!net_if_is_admin_up(iface)) {
    LOG_ERR("IFACE is not admin up");
  }

  // Check if interface is up and set it up otherwise
  if (!net_if_is_up(iface)) {
    LOG_DBG("Setting iface up");
    err = net_if_up(iface);
    if (err < 0) LOG_ERR("Error setting iface to up, %d", err);
  }

  err = net_addr_pton(AF_INET6, ip, &my_addr);  // Convert the string to byte format and copy the address to in6_addr struct
  if (err < 0) LOG_ERR("Error on coppying ip, %d", err);
  struct net_if_addr *iface_addr = net_if_ipv6_addr_add(iface, &my_addr, NET_ADDR_MANUAL, 0);  // Add the address to the interface
  if (!iface_addr) LOG_ERR("Couldn't set ip");

  if (net_if_is_admin_up(iface) && net_if_is_carrier_ok(iface) && !net_if_is_dormant(iface)) {
    LOG_DBG("IF STATUS: OPER UP");
  }

  LOG_DBG("Start net_config");
  err = net_config_init_by_iface(iface, "Zephyr MQTT", NET_CONFIG_NEED_IPV6, 30000);  // Initialize rest of the network stack
  if (err < 0) LOG_ERR("Initialization error, %d", err);
  LOG_DBG("Network setup done");
}
