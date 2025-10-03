#include "dht_esp.h"
#include <esp_log.h>

static const char * TAG = "dht";

void dht_data_line_init(struct DhtEsp * const self);

void dht_esp_init(struct DhtEsp * const self, uint32_t pin, TaskHandle_t task) {
    assert(self);
    self->task = task;
    self->pin = pin;
    dht_init(&self->dht);
    dht_data_line_init(self);
}

void dht_print_edges(const struct Dht * const self) {
    int i;
    for (i = 0; i < self->current_edge - 1; ++i) {
		ESP_LOGI(TAG, "%d: %d us %d", i,
			self->edges[i + 1].timestamp - self->edges[i].timestamp,
			self->edges[i].level);
	}
    ESP_LOGI(TAG, "%d: %d", i, self->edges[i].level);
}

