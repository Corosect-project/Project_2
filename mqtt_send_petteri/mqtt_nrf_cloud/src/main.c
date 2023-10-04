/*
 * Copyright (c) 2023 OAMK Corosect-project.
 * and 2023 OAMK Corosect-project 2. 
 * nRF cloud functionality added
 *  
 * SPDX-License-Identifier: MIT
 */

#include <dk_buttons_and_leds.h>
#include <nrfx_spim.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/l2cap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/mqtt.h>
#include <net/nrf_cloud.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>


LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

#include "config.h"
#include "mqtt.h"

#define ERROR(err) (err < 0)
#define ZEPHYR_ADDR "2001:db8::1"
#define SERVER_ADDR "2001:db8::2"
#define SERVER_PORT 1883

//Define in case you want use nrf_cloud
#define nrf_cloud 

volatile bool quit = false;
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(0x1820)),
};

void button_handler(uint32_t state, uint32_t has_changed);
void bt_ready();
static void bt_connected(struct bt_conn *conn, uint8_t err);
static void bt_disconnected(struct bt_conn *conn, uint8_t reason);

BT_CONN_CB_DEFINE(con_calbacks) = {.connected = bt_connected, .disconnected = bt_disconnected};

//Topics and data getting to program
struct topic_and_data {
  uint32_t topic_data_size;
  uint32_t message_data_size;
  uint8_t topic_data;
  uint8_t message_data;
};

struct topic_and_data nrf_cloud_topic_and_data(){ //nRF cloud topic and data

  struct topic_and_data data; //Data getting to program

  //Message and topic
  uint8_t * message = "Hello world from Zephyr";
  uint8_t * topic = "TEST";

  //Team id for nRF cloud
  uint8_t team_id_topic[] = "c88885a4-8051-438f-8bbf-359b3f62a586"; 
  
  //Client id for device
  //uint8_t client_id_topic[] = "683909493";
  
  //Data connect to same variable
  uint8_t topic_full = ""; 

  //Connecting data to same that printed in topic 
  sprintf(topic_full,"${mqttTopicPrefix}/m/d/${CC:FC:5C:5D:B1:7B}/d2c/%s/%s/${iot:ClientId}", team_id_topic, &topic);

  //Calculating topic and message size
  uint32_t topic_size = strlen((const char *)topic_full);
  uint32_t message_size = strlen(message);

  data.message_data = (uint8_t)message;
  data.message_data_size = message_size;
  data.topic_data = topic_full;
  data.topic_data_size = topic_size;
  return data;
}

void main(void) {

  
  int32_t err;
  LOG_INF("Hello World! %s", CONFIG_BOARD);
  
  if (ERROR(dk_buttons_init(button_handler))) LOG_ERR("Error initializing buttons");

  err = bt_enable(NULL);
  if (ERROR(err)) LOG_ERR("Error enabling BT %d", err);
  bt_ready();

  LOG_DBG("Waiting connection...");
  k_sleep(K_MSEC(30000));
  LOG_DBG("Continuing");
  init_network(ZEPHYR_ADDR);
  
  #ifdef nrf_cloud //If using nrf cloud
  LOG_INF("Running nrf cloud initializer");
  //nrf cloud initalizing and sending topic and message address to initializer
  //struct topic_and_data data_and_topic_initialize = nrf_cloud_topic_and_data();
  initializer_nrf_cloud(); 
  #endif


  struct mqtt_client *client_ctx = init_mqtt(SERVER_ADDR, SERVER_PORT);

  for (size_t i = 0; i < 11; i++) {
    err = mqtt_connect(client_ctx);
    if (err != 0) {
      LOG_ERR("MQTT ERROR: %d", err);
      while (!quit) {
        k_sleep(K_MSEC(1000));
      }
      return;
    }

    err = zsock_poll(fds, 1, 10000);
    LOG_DBG("POLL ret, %d", err);
    err = mqtt_input(client_ctx);
    if (ERROR(err)) LOG_ERR("Input error, %d", err);
    LOG_DBG("%d", connected);
    k_sleep(K_SECONDS(5));
    if (!connected) {
      err = mqtt_abort(client_ctx);
      LOG_ERR("Connection aborted, %d", err);
    } else break;
  }

  if (connected) {

    //In case you want to use nRF cloud
    #ifdef nrf_cloud 

    //Getting data_and topic 
    struct topic_and_data data_to_mqtt = nrf_cloud_topic_and_data();

    //err = nrf_cloud_send_data_mqtt(topic_size, data_team_id_topic);
    err = send_message(&(data_to_mqtt.message_data), &(data_to_mqtt.topic_data));
    #elif
    uint8_t *message = "Hello world from Zephyr";
    uint8_t *topic = "TEST";
    err = send_message(message, topic);
    #endif

    if (err) LOG_ERR("SEND ERROR: %d", err);
  }

  while (!quit && connected) {
    err = zsock_poll(fds, 1, 5000);
    if (err > 0) {
      if (fds[0].revents & ZSOCK_POLLIN) {
        err = mqtt_input(client_ctx);
        LOG_ERR("%d", err);
      }
    } else if (ERROR(err)) LOG_ERR("%d", errno);
    err = mqtt_live(client_ctx);
    LOG_ERR("%d", err);
    LOG_DBG("WAITING");
    k_sleep(K_MSEC(2000));
  }

  LOG_INF("QUIT\n");
  err = mqtt_disconnect(client_ctx);
  LOG_DBG("Disconnected, %d", err);
  k_sleep(K_MSEC(1000));
  bt_le_adv_stop();
  bt_disable();
}

void button_handler(uint32_t state, uint32_t has_changed) {
  if (DK_BTN2_MSK == has_changed) {
    quit = true;
  }
}

void bt_ready() {
  uint32_t err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), 0, 0);
  if (err == -ENOMEM) LOG_ERR("ENOMEM\n");
  else if (err == -ECONNREFUSED || err == -EIO) LOG_ERR("ECONNREFUSED\n");
  LOG_DBG("%d\n", err);
}

void bt_connected(struct bt_conn *conn, uint8_t err) {
  if (err) {
    LOG_ERR("Connection failed, %d", err);
  } else {
    LOG_INF("Connected");
    // k_sleep(K_SECONDS(1));
    // init_network(ZEPHYR_ADDR);
  }
}

void bt_disconnected(struct bt_conn *conn, uint8_t reason) { LOG_INF("Disconnected, reason %d", reason); }



