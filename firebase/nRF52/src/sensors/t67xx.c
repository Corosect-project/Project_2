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
static const nrfx_timer_t timer = NRFX_TIMER_INSTANCE(0);

void gpiote_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  printk("pin: %d, pol: %d\n", pin, action);
  if (pin == falling_edge) {
    nrfx_timer_pause(&timer);
    /* Read out timer CC register and clear it */
    nrfx_timer_capture(&timer, NRF_TIMER_CC_CHANNEL0);
    uint32_t timer_cc_val = nrfx_timer_capture_get(&timer, NRF_TIMER_CC_CHANNEL0);
    printk("Raw timer val: %x\n", timer_cc_val);
    /* Calculate pulse length, 16M base freq */
    uint32_t pulse_len_us = (timer_cc_val >> 4) - 1;
    uint32_t pulse_len_ms = pulse_len_us / 1000;
    printk("ms: %d\n", pulse_len_ms);
    printk("us: %d\n", pulse_len_us);
  } else if (pin == rising_edge) {
    nrfx_timer_clear(&timer);
    nrfx_timer_resume(&timer);
  }
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
  // nrfx_ppi_channel_assign(rising_channel, event, task);

  event = nrfx_gpiote_in_event_addr_get(falling_channel);
  task = nrfx_timer_task_address_get(&timer, NRF_TIMER_TASK_STOP);
  uint32_t fork_task = nrfx_timer_capture_task_address_get(&timer, NRF_TIMER_CC_CHANNEL0);
  nrfx_ppi_channel_assign(falling_channel, event, fork_task);
  // nrfx_ppi_channel_fork_assign(falling_channel, fork_task);
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

static struct gpio_callback cb_data = {};
static volatile int count = 0;
void callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  LOG_DBG("CALLED: %d", pins);
  count++;
}

void t67xx_init() {
  timer_init();
  // ppi_init();
  gpiote_init();
  manual_isr_setup();

  const struct device *gpio = DEVICE_DT_GET(DT_NODELABEL(gpio0));
  if (!device_is_ready(gpio)) {
    LOG_ERR("Device is not ready");
    return;
  }
  // gpio_pin_configure(gpio, 3, GPIO_INPUT | GPIO_PULL_UP);
  // gpio_pin_interrupt_configure(gpio, 3, GPIO_INT_EDGE_RISING);
  // gpio_pin_configure(gpio, 4, GPIO_INPUT | GPIO_PULL_UP);
  // gpio_pin_interrupt_configure(gpio, 4, GPIO_INT_EDGE_FALLING);
  // LOG_DBG("Pin init complete");

  // gpio_init_callback(&cb_data, callback, 4);
  // int err = gpio_add_callback(gpio, &cb_data);
  // if (err < 0) LOG_ERR("CB_ADD_ERROR: %d", err);

  while (true) {
    // LOG_DBG("DATA: %d %d", gpio_pin_get(gpio, 3), gpio_pin_get(gpio, 4));
    k_msleep(10);
  };
}
