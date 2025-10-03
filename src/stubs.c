#include "dht.h"

struct DhtEspStubs {
    struct Dht dht;
    enum DhtDataLevel level;
};

static unsigned microseconds = 0;

void dht_config_data_line_output(struct Dht * const self) { }

void dht_config_data_line_input(struct Dht * const self) { }

void dht_data_line_irq_enable(struct Dht * const self) { }

void dht_data_line_irq_disable(struct Dht * const self) { }

void dht_set_data_line_high(struct Dht * const self) {
    struct DhtEspStubs * dht = (struct DhtEspStubs *)self;
    dht->level = DHT_DATA_HIGH; 
}

void dht_set_data_line_low(struct Dht * const self) {
    struct DhtEspStubs * dht = (struct DhtEspStubs *)self;
    dht->level = DHT_DATA_LOW;
}

enum DhtDataLevel dht_get_data_line_level(struct Dht * const self) {
    struct DhtEspStubs * dht = (struct DhtEspStubs *)self;
    return dht->level;
}

void dht_sleep_ms(unsigned min_time_ms, unsigned max_time_ms) { }

void dht_notify_sequence_completed(struct Dht const * self) { }

void dht_wait_for_completion(struct Dht const * self) { }

unsigned dht_get_microseconds(void) {
    return microseconds;
}

