#include "mqtt.h"
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <net/nrf_cloud.h>

LOG_MODULE_DECLARE(app, LOG_LEVEL_DBG);

// MQTT global variables
struct zsock_pollfd fds[1] = {0};
bool connected = false;

// MQTT
static uint8_t rx_buffer[256];
static uint8_t tx_buffer[256];
static struct mqtt_client client_ctx;
static struct sockaddr_storage broker;





void mqtt_evt_handler(struct mqtt_client *client, const struct mqtt_evt *evt);

/**
 * @brief Initialize mqtt parameters and set broker ip and port
 *
 * @param server_ip broker's ip address
 * @param port broker's port
 * @return Pointer to the client struct or NULL on failure
 */
struct mqtt_client *init_mqtt(const char *server_ip, uint32_t port) {

  uint32_t err;

  uint8_t *client_id = "ZEPHYR";
  mqtt_client_init(&client_ctx);
  client_ctx.broker = &broker;
  client_ctx.evt_cb = mqtt_evt_handler;
  client_ctx.client_id.utf8 = client_id;
  client_ctx.client_id.size = sizeof(client_id) - 1;
  client_ctx.password = NULL;   // No password
  client_ctx.user_name = NULL;  // No username
  client_ctx.protocol_version = MQTT_VERSION_3_1_1;
  client_ctx.transport.type = MQTT_TRANSPORT_SECURE;  // No TLS certificates

  client_ctx.rx_buf = rx_buffer;
  client_ctx.rx_buf_size = sizeof(rx_buffer);
  client_ctx.tx_buf = tx_buffer;
  client_ctx.tx_buf_size = sizeof(tx_buffer);


  struct mqtt_sec_config *tls_config = &client_ctx.transport.tls.config;

  static int m_sec_tags[] =  {2,3};

  tls_config->peer_verify = TLS_PEER_VERIFY_REQUIRED;
  tls_config->cipher_list = NULL;
  tls_config->sec_tag_list = m_sec_tags;
  tls_config->sec_tag_count = ARRAY_SIZE(m_sec_tags);
  tls_config->hostname = "Test";
  tls_config->set_native_tls = true;


  struct sockaddr_in6 *broker6 = (struct sockaddr_in6 *)&broker;
  memset(broker6, 0, sizeof(*broker6));  // Zero the struct

  broker6->sin6_family = AF_INET6;
  broker6->sin6_port = htons(port);
  err = net_addr_pton(AF_INET6, server_ip, &broker6->sin6_addr);  // Assign broker's ip address to sockaddr struct
  LOG_DBG("TEST: %d", err);
  // zsock_inet_pton(AF_INET6, SERVER_ADDR, &broker6->sin6_addr);

  // Set pollable events for later use
  fds[0].fd = client_ctx.transport.tcp.sock;
  fds[0].events = ZSOCK_POLLIN;

  if (err == 0) return &client_ctx;
  else return NULL;
}

/**
 * @brief Publish message to specific topic
 *
 * @param msg message string
 * @param topic MQTT topic
 * @return 0 or negative error code (Zephyr errno.h)
 */
int send_message(char *msg, char *topic) {
  struct mqtt_publish_param param = {0};
 
  param.message.payload.data = msg;
  param.message.payload.len = strlen(msg);
  param.message.topic.qos = MQTT_QOS_0_AT_MOST_ONCE;
  param.message.topic.topic.utf8 = topic;
  param.message.topic.topic.size = strlen(topic);
  return mqtt_publish(&client_ctx, &param);
}

void mqtt_evt_handler(struct mqtt_client *client, const struct mqtt_evt *evt) {
  LOG_ERR("MQTT_CALLBACK");
  switch (evt->type) {
    case MQTT_EVT_CONNACK:
      if (evt->result != 0) {
        LOG_ERR("Error connecting: %d", evt->result);
        break;
      }
      LOG_INF("MQTT connected");
      connected = true;
      break;
    case MQTT_EVT_DISCONNECT:
      LOG_INF("MQTT disconnected");
      connected = false;
      break;
    case MQTT_EVT_PINGRESP:
      LOG_INF("Ping response");
      break;
    default:
      LOG_INF("MQTT default");
      break;
  }
}



//Nrf cloud funcionality starts after this


//nRF cloud handler
void cloud_handler(struct nrf_cloud_evt *evt) {
  evt->type = NRF_CLOUD_EVT_TRANSPORT_CONNECTED;
  evt->data.len = 15; //Lenght of data
  //evt->data.ptr = ; //Pointer to data
  evt->topic.len = 15; //Lenght of topic
  //evt->topic.ptr = ; //Pointer to topic
}


void initializer_nrf_cloud(){ //Initialize data for nrf_cloud
  LOG_INF("Running initializer to nrf cloud");
  //Nrf cloud struct usage
  struct nrf_cloud_init_param Nrf_ble_cloud;
  Nrf_ble_cloud.client_id = NULL; 
  //Set to null unless NRF_CLOUD_CLIENT_ID_SRC_RUNTIME is set
  //Nrf_ble_cloud->event_handler
  Nrf_ble_cloud.event_handler = cloud_handler;

  //const struct Nrf_ble_cloud2 *nrf_ble_cloud_final;

  //Initalizing nrf cloud connection
  int error = nrf_cloud_init(&Nrf_ble_cloud);

  
  //Testing if initializing returns error
  if(error != 0 ) LOG_ERR("Error in initializing of nrf cloud: %d", error);

  //Connecting to nrf cloud
  nrf_cloud_connect();

}


void nrf_send_data_mqtt(uint32_t mqtt_lenght, const void * pointer_mqtt()){

  //Mqtt topic for nRF cloud
  struct nrf_cloud_topic Nrf_send_mqtt;
  Nrf_send_mqtt.len = mqtt_lenght;
  Nrf_send_mqtt.ptr = pointer_mqtt();

}