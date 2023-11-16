#include <hal/nrf_gpiote.h>
#include <nrfx_gpiote.h>
#include <nrfx_ppi.h>
#include <nrfx_timer.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, LOG_LEVEL_DBG);

nrfx_gpiote_pin_t rising_edge = 3;
nrfx_gpiote_pin_t falling_edge = 4;
static const nrfx_timer_t timer = NRFX_TIMER_INSTANCE(1);  // Timer 0 is in radio/bluetooth use so use timer 1 instead

void gpiote_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  LOG_DBG("pin: %d, pol: %d", pin, action);
  if (pin != falling_edge) return;
  /* Read out timer CC register and clear it */
  // nrfx_timer_capture(&timer, NRF_TIMER_CC_CHANNEL0);
  uint32_t timer_cc_val = nrfx_timer_capture_get(&timer, NRF_TIMER_CC_CHANNEL0);
  LOG_INF("Raw timer val: %x", timer_cc_val);
  /* Calculate pulse length, 16M base freq */
  uint32_t pulse_len_us = (timer_cc_val >> 4) - 1;
  uint32_t pulse_len_ms = pulse_len_us / 1000;
  LOG_INF("ms: %d, %x", pulse_len_ms, pulse_len_ms);
  LOG_INF("us: %d, %x", pulse_len_us, pulse_len_us);
  LOG_INF("PPM: %d", (pulse_len_ms - 2) * 2);
  LOG_INF("");

  // nrfx_timer_clear(&timer);
}

void timer_init() {
  nrfx_timer_config_t config = NRFX_TIMER_DEFAULT_CONFIG;
  config.bit_width = TIMER_BITMODE_BITMODE_32Bit;
  config.frequency = NRF_TIMER_FREQ_16MHz;

  int err = nrfx_timer_init(&timer, &config, NULL);
  if (err != NRFX_SUCCESS) LOG_ERR("Timer init failed: %d", err);
  nrfx_timer_enable(&timer);
}

void ppi_init() {
  nrf_ppi_channel_t rising_channel;
  nrf_ppi_channel_t falling_channel;

  int err = nrfx_ppi_channel_alloc(&rising_channel);
  if (err != NRFX_SUCCESS) {
    LOG_ERR("Failed to allocate ppi channel: %d", err);
    return;
  }

  err = nrfx_ppi_channel_alloc(&falling_channel);
  if (err != NRFX_SUCCESS) {
    LOG_ERR("Failed to allocate ppi channel: %d", err);
    return;
  }

  err = nrfx_ppi_channel_enable(rising_channel);
  if (err != NRFX_SUCCESS) {
    LOG_ERR("Failed to enable ppi channel: %d", err);
    return;
  }

  err = nrfx_ppi_channel_enable(falling_channel);
  if (err != NRFX_SUCCESS) {
    LOG_ERR("Failed to enable ppi channel: %d", err);
    return;
  }

  uint32_t event = nrfx_gpiote_in_event_addr_get(rising_edge);
  uint32_t task = nrfx_timer_task_address_get(&timer, NRF_TIMER_TASK_START);
  uint32_t fork_task = nrfx_timer_task_address_get(&timer, NRF_TIMER_TASK_CLEAR);
  err = nrfx_ppi_channel_assign(rising_channel, event, task);
  if (err != NRFX_SUCCESS) LOG_ERR("RISING TASK: %d");
  err = nrfx_ppi_channel_fork_assign(rising_channel, fork_task);
  if (err != NRFX_SUCCESS) LOG_ERR("RISING FORK_TASK: %d");

  event = nrfx_gpiote_in_event_addr_get(falling_edge);
  task = nrfx_timer_task_address_get(&timer, NRF_TIMER_TASK_STOP);
  fork_task = nrfx_timer_capture_task_address_get(&timer, NRF_TIMER_CC_CHANNEL0);
  err = nrfx_ppi_channel_assign(falling_channel, event, task);
  if (err != NRFX_SUCCESS) LOG_ERR("FALLING TASK: %d");
  nrfx_ppi_channel_fork_assign(falling_channel, fork_task);
  if (err != NRFX_SUCCESS) LOG_ERR("FALLING FORK_TASK: %d");
}

void gpiote_init() {
  nrfx_gpiote_in_config_t falling_config = {0};
  nrfx_gpiote_in_config_t rising_config = {0};

  falling_config.hi_accuracy = 1;
  falling_config.pull = NRF_GPIO_PIN_PULLUP;
  falling_config.sense = NRF_GPIOTE_POLARITY_HITOLO;

  rising_config.hi_accuracy = 1;
  rising_config.pull = NRF_GPIO_PIN_PULLUP;
  rising_config.sense = NRF_GPIOTE_POLARITY_LOTOHI;

  int err = nrfx_gpiote_init(0);
  // if (err != NRFX_SUCCESS) {
  //   LOG_ERR("Gpiote init failed: %d", err);
  //   return;
  // }

  err = nrfx_gpiote_in_init(falling_edge, &falling_config, gpiote_event_handler);
  if (err != NRFX_SUCCESS) {
    LOG_ERR("Gpiote in init failed: %d", err);
    return;
  }

  err = nrfx_gpiote_in_init(rising_edge, &rising_config, gpiote_event_handler);
  if (err != NRFX_SUCCESS) {
    LOG_ERR("Gpiote in init failed: %d", err);
    return;
  }

  nrfx_gpiote_in_event_enable(rising_edge, true);
  nrfx_gpiote_in_event_enable(falling_edge, true);
}

void manual_isr_setup() {
  IRQ_DIRECT_CONNECT(GPIOTE_IRQn, 0, nrfx_gpiote_irq_handler, 0);
  irq_enable(GPIOTE_IRQn);
}

void t67xx_init() {
  timer_init();
  gpiote_init();
  ppi_init();
  manual_isr_setup();

  while (true) {
    // LOG_DBG("DATA: %d %d", gpio_pin_get(gpio, 3), gpio_pin_get(gpio, 4));
    k_msleep(10);
  };
}
