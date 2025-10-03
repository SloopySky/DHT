#include "dht.h"
#include <assert.h>

/*
 * Communication sequence
 * ======================
 *
 * Free status at data line level high
 *
 * Start signal:
 * Data line level low for at least 18 ms
 * Data line level back high
 */
#define DHT_START_SIGNAL_DURATION_MS_MIN 18
#define DHT_START_SIGNAL_DURATION_MS_MAX 20

/*
 * Acknowledge:
 * DHT detects the start signal within 20-40 us and sets data line low for 80us and then high for 80us
 */
#define DHT_ACK_EDGES_NUMBER 2
#define DHT_ACK_TIMEOUT_US 60 // 40 us + reserve

/*
 * Data bit:
 * Every bit of data begins with data line low for 50us
 * Duration of the following data line high level signal determines whether the data bit is 0 or 1
 * Data bit 0 - data line high for 26-28 us
 * Data bit 1 - data line high for 70 us
 */
#define DHT_DATA_BIT_0 0
#define DHT_DATA_BIT_1 1
#define DHT_DATA_BIT_0_DURATION_US_MAX 35 // 28 us + reserve
#define DHT_DATA_BIT_1_DURATION_US_MAX 80 // 70 us + reserve
#define DHT_DATA_EDGES_PER_BYTE 16

/*
 * Data format:
 * 8 bits integral RH
 * 8 bits decimal RH
 * 8 bits integral T
 * 8 bits decimal T
 * 8 bits checksum
 */
#define DHT_INTEGRAL_RH_EDGES_INDEX 2
#define DHT_DECIMAL_RH_EDGES_INDEX 18
#define DHT_INTEGRAL_T_EDGES_INDEX 34
#define DHT_DECIMAL_T_EDGES_INDEX 50
#define DHT_CHECKSUM_EDGES_INDEX 66

/*
 * Checksum:
 * Last 8 bits of (integral RH + decimal RH + integral T + decimal T)
 *
 * End of sequence:
 * When the last bit data is transmitted, Dht sets data line low for 50us and then sets data back to the free status
 */


// current_edge field of struct Dht equals -1 indicates no data has been collected
#define DHT_NO_DATA_AVAILABLE -1

// Max interval between two edges
#define DHT_NEW_EDGE_TIMEOUT_US 100

// Most significant bit position in byte
#define DHT_MOST_SIGNIFICANT_BIT_OFFSET 7


// Private helper functions declaration
static inline enum DhtStatus dht_wait_for_ack(const struct Dht * const self);
static enum DhtStatus dht_decode_byte(const struct DhtEdge * const edges, uint8_t * const data_byte);
static unsigned dht_calculate_data_crc(const struct DhtData * const data);

enum DhtStatus dht_read(struct Dht * const self, struct DhtData * const data) {
    enum DhtStatus status = dht_start_read(self);
    if (status != DHT_OK) {
        return status;
    }
    dht_wait_for_completion(self);
    return dht_get_data(self, data);
}

enum DhtStatus dht_check_status(struct Dht * const self) {
    assert(self);

    if (self->current_edge == DHT_NO_DATA_AVAILABLE) {
        return DHT_NO_DATA;
    }
    
    if (self->current_edge < DHT_EDGES_NUMBER) {
        // Check timeout
        const unsigned last_edge_timestamp = self->edges[self->current_edge - 1].timestamp;
        const unsigned microseconds_elapsed = dht_get_microseconds() - last_edge_timestamp;
        if (microseconds_elapsed > DHT_NEW_EDGE_TIMEOUT_US) {
            // DHT_NEW_EDGE_TIMEOUT_US has elapsed since last edge
            return DHT_ERR_TIMEOUT;
        }
        // Transmission in progress
        return DHT_BUSY;
    }
    
    // else self->current_edge == DHT_EDGES_NUMBER
    return DHT_DATA_READY;
}

enum DhtStatus dht_start_read(struct Dht * const self) {
    assert(self);

    dht_abort_read(self);
    // Configure data line as output
    dht_config_data_line_output(self);
    // Enable data line interrupts
    dht_data_line_irq_enable(self);
    // Data line low for DHT_START_SIGNAL_DURATION_MS_MIN to DHT_START_SIGNAL_DURATION_MS_MAX
    dht_set_data_line_low(self);
    dht_sleep_ms(DHT_START_SIGNAL_DURATION_MS_MIN, DHT_START_SIGNAL_DURATION_MS_MAX);
    // Data line high
    dht_set_data_line_high(self);
    // Reset current_edge
    self->current_edge = 0;
    // Configure data line input
    dht_config_data_line_input(self);
   
    // Wait for ACK
    if (dht_wait_for_ack(self) == DHT_ERR_TIMEOUT) {
        dht_abort_read(self);
        return DHT_ERR_TIMEOUT;
    }

    return DHT_OK;
}

enum DhtStatus dht_get_data(struct Dht * const self, struct DhtData * const data) {
    assert(self);
    assert(data);

    enum DhtStatus status;

    status = dht_check_status(self);
    if (status != DHT_DATA_READY) {
        return status;   
    }

    // integral RH
    status = dht_decode_byte(self->edges + DHT_INTEGRAL_RH_EDGES_INDEX, &data->humidity.integral);
    if (status != DHT_OK) {
        return status;
    }

    // decimal RH
    // status |= dht_decode_byte(self->edges + DHT_DECIMAL_RH_EDGES_INDEX, &data->humidity.decimal);
    // always 0
    data->humidity.decimal = 0;

    // integral T
    status = dht_decode_byte(self->edges + DHT_INTEGRAL_T_EDGES_INDEX, &data->temperature.integral);
    if (status != DHT_OK) {
        return status;
    }

    // decimal T
    // status |= dht_decode_byte(self->edges + DHT_DECIMAL_T_EDGES_INDEX, &data->temperature.decimal);
    // always 0
    data->temperature.decimal = 0;

    // crc
    status = dht_decode_byte(self->edges + DHT_CHECKSUM_EDGES_INDEX, &data->crc);
    if (dht_calculate_data_crc(data) != data->crc) {
        return DHT_ERR_CRC;
    }

    return DHT_OK;
}

void dht_abort_read(struct Dht * const self) {
    assert(self);

    self->current_edge = DHT_NO_DATA;
    dht_data_line_irq_disable(self);
}

void dht_handle_data_line_edge(struct Dht * const self) {
    assert(self);

    enum DhtDataLevel level = dht_get_data_line_level(self);
    if ((self->current_edge == 0 && level == DHT_DATA_LOW) || // first edge must be falling
        (self->current_edge > 0 && self->current_edge < DHT_EDGES_NUMBER)) {
        self->edges[self->current_edge].timestamp = dht_get_microseconds();
        self->edges[self->current_edge].level = level;
        self->current_edge++;

        if (self->current_edge == DHT_EDGES_NUMBER) {
            dht_notify_sequence_completed(self);
        }
    }
}

unsigned dht_get_edges_count(const struct Dht * const self) {
    return self->current_edge > 0 ? self->current_edge : 0;
}

const char * dht_status_to_str(enum DhtStatus status) {
    const char * str;
    switch (status) {
        case DHT_OK:
            str = "DHT_OK";
            break;
        case DHT_NO_DATA:
            str = "DHT_NO_DATA";
            break;
        case DHT_BUSY:
            str = "DHT_NO_DATA";
            break;
        case DHT_DATA_READY:
            str = "DHT_DATA_READY";
            break;
        case DHT_ERR_TIMEOUT:
            str = "DHT_ERR_TIMEOUT";
            break;
        case DHT_ERR_SEQUENCE_INVALID:
            str = "DHT_ERR_SEQUENCE_INVALID";
            break;
        case DHT_ERR_CRC:
            str = "DHT_ERR_CRC";
            break;
        default:
            str = "DHT_STATUS_UNKNOWN";
    }
    
    return str;
}

void dht_init(struct Dht * const self) {
    assert(self);
    self->current_edge = DHT_NO_DATA_AVAILABLE;
}


static inline enum DhtStatus dht_wait_for_ack(const struct Dht * const self) {
    const unsigned start_time = dht_get_microseconds();

    while(self->current_edge == 0) {
        unsigned microseconds_elapsed = dht_get_microseconds() - start_time;
        if (microseconds_elapsed > DHT_ACK_TIMEOUT_US) {
            return DHT_ERR_TIMEOUT;
        }
    }

    return DHT_OK;
}

static enum DhtStatus dht_decode_byte(const struct DhtEdge * const edges, uint8_t * const data_byte) {
    *data_byte = 0;
    int bit_offset = DHT_MOST_SIGNIFICANT_BIT_OFFSET;

    for (unsigned i = 0; i < DHT_DATA_EDGES_PER_BYTE; i += 2) {
        // edges[i] - data begins with data line low for 50us
        // edges[i + 1] - data bit
        // edges[i + 2] - start of next data bit or end of sequence
        if (edges[i].level != DHT_DATA_LOW || edges[i + 1].level != DHT_DATA_HIGH) {
            return DHT_ERR_SEQUENCE_INVALID;
        }

        unsigned data_bit_duration_us = edges[i + 2].timestamp - edges[i + 1].timestamp;
        if (data_bit_duration_us > DHT_DATA_BIT_1_DURATION_US_MAX) {
            return DHT_ERR_TIMEOUT;
        }

        // bit 1 if too long for bit 0 else bit 0
        uint8_t bit = data_bit_duration_us > DHT_DATA_BIT_0_DURATION_US_MAX ? DHT_DATA_BIT_1 : DHT_DATA_BIT_0;
        *data_byte |= bit << bit_offset;
        --bit_offset;
    }

    return DHT_OK;
}

static inline unsigned dht_calculate_data_crc(const struct DhtData * const data) {
    return (data->humidity.integral + data->humidity.decimal + data->temperature.integral + data->temperature.decimal) & 0xFF;
}

