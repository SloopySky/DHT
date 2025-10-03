#include "dht_esp.h"
#include <esp_timer.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static void IRAM_ATTR dht_data_line_irq(void *arg) {
    struct Dht * self = (struct Dht *)arg;
    dht_handle_data_line_edge(self);
}

void dht_data_line_init(struct DhtEsp * const self) {
    assert(self);

    const gpio_config_t config = {
        .pin_bit_mask = (1 << self->pin),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&config));
    ESP_ERROR_CHECK(gpio_set_level(self->pin, 1));

    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_EDGE));
    ESP_ERROR_CHECK(gpio_isr_handler_add(self->pin, dht_data_line_irq, (void *)&self->dht));
}

void dht_config_data_line_output(struct Dht * const self) {
    // data line pin configured as input-output
}

void dht_config_data_line_input(struct Dht * const self) {
    // data line pin configured as input-output
}

void dht_data_line_irq_enable(struct Dht * const self) {
    struct DhtEsp * dht_esp = (struct DhtEsp *)self;
    ESP_ERROR_CHECK(gpio_intr_enable(dht_esp->pin));
}

void dht_data_line_irq_disable(struct Dht * const self) {
    struct DhtEsp * dht_esp = (struct DhtEsp *)self;
    ESP_ERROR_CHECK(gpio_intr_disable(dht_esp->pin));
}

void dht_set_data_line_high(struct Dht * const self) {
    struct DhtEsp * dht_esp = (struct DhtEsp *)self;
    ESP_ERROR_CHECK(gpio_set_level(dht_esp->pin, 1));
}

void dht_set_data_line_low(struct Dht * const self) {
    struct DhtEsp * dht_esp = (struct DhtEsp *)self;
    ESP_ERROR_CHECK(gpio_set_level(dht_esp->pin, 0));
}

enum DhtDataLevel dht_get_data_line_level(struct Dht * const self) {
    struct DhtEsp * dht_esp = (struct DhtEsp *)self;
    return gpio_get_level(dht_esp->pin);
}

void dht_sleep_ms(unsigned min_time_ms, unsigned max_time_ms) {
    unsigned delay = (max_time_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS;
    vTaskDelay(delay);
}

void dht_notify_sequence_completed(struct Dht const * self) {
    struct DhtEsp * dht_esp = (struct DhtEsp *)self;
    BaseType_t higher_prio_task_woken;
    vTaskNotifyGiveFromISR(dht_esp->task, &higher_prio_task_woken);
    portYIELD_FROM_ISR(higher_prio_task_woken);
}

void dht_wait_for_completion(struct Dht const * self) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

unsigned dht_get_microseconds(void) {
    return (unsigned)esp_timer_get_time();
}

