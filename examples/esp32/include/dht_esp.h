#ifndef DHT_ESP_H_
#define DHT_ESP_H_

#include <stdint.h>
#include <dht.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

struct DhtEsp {
    struct Dht dht;
    uint32_t pin;
    TaskHandle_t task;
};

void dht_esp_init(struct DhtEsp * const self, uint32_t pin, TaskHandle_t task);

void dht_print_edges(const struct Dht * const self);

#endif /* DHT_ESP_H_ */

