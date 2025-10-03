/**
 * @file dht.h
 * @brief This file declares interfaces required and implemented by DHT sensor component
 */

#ifndef DHT_H_
#define DHT_H_

#include <stdint.h>

/*
 * Implemented interfaces
 * ======================
 * 
 * This section defines interfaces implemented by the DHT component
 */

/**
 * @brief DHT status
 */
enum DhtStatus {
    DHT_OK                   = 0b00000000,
    DHT_NO_DATA              = 0b00000001,
    DHT_BUSY                 = 0b00000010,
    DHT_DATA_READY           = 0b00000100,
    DHT_ERR_TIMEOUT          = 0b00001000,
    DHT_ERR_SEQUENCE_INVALID = 0b00010000,
    DHT_ERR_CRC              = 0b00100000,
};

/**
 * @brief DHT data line levels definition
 */
enum DhtDataLevel { DHT_DATA_LOW = 0, DHT_DATA_HIGH = 1 };

/**
 * @brief DHT data line edge structure
 */
struct DhtEdge {
    unsigned timestamp;
    enum DhtDataLevel level;
};

// Total number of data line edges expected in one transmission sequence
#define DHT_EDGES_NUMBER 84 // 2 ack edges + 40 data bits * 2 + 2 end of sequence edges

/**
 * @brief DHT class definition
 */
struct Dht {
	struct DhtEdge edges[DHT_EDGES_NUMBER];
    int current_edge;
};

/**
 * @brief DHT data structure
 */
struct DhtData {
    struct {
        uint8_t integral;
        uint8_t decimal;
    } humidity, temperature;
    uint8_t crc;
};

/**
 * @brief Execute DHT read data sequence
 *
 * @param self pointer to Dht instance
 * @param[out] data pointer to the data structure to store the humidity and temperature values in
 * @return
 *     - DHT_OK success
 *     - DHT_ERR_TIMEOUT no edge detected within expected timeframe
 *     - DHT_ERR_SEQUENCE_INVALID edges sequence not valid
 *     - DHT_ERR_CRC checksum not consistent
*/
enum DhtStatus dht_read(struct Dht * const self, struct DhtData * const data);

/**
 * @brief Check DHT status
 *
 * @param self pointer to Dht instance
 * @return
 *     - DHT_NO_DATA no data available, sequence not started;
 *     - DHT_BUSY sequence in progress
 *     - DHT_DATA_READY sequence finished
 *     - DHT_ERR_TIMEOUT no response received on time
 */
enum DhtStatus dht_check_status(struct Dht * const self);

/**
 * @brief Initiate DHT transmission sequence
 *
 * @param self pointer to Dht instance
 * @return
 *     - DHT_OK success
 *     - DHT_ERR_TIMEOUT no response received on time
 */
enum DhtStatus dht_start_read(struct Dht * const self);

/**
 * @brief Read Humidity [%] and Temperature [°C] collected from DHT sensor
 *
 * @param self pointer to Dht instance
 * @param[out] data pointer to the data structure to store the humidity and temperature values in
 * @return
 *     - DHT_OK success
 *     - DHT_NO_DATA no data available, sequence not started;
 *     - DHT_BUSY sequence in progress
 *     - DHT_ERR_TIMEOUT no edge detected within expected timeframe
 *     - DHT_ERR_SEQUENCE_INVALID edges sequence not valid
 *     - DHT_ERR_CRC checksum not consistent
 */
enum DhtStatus dht_get_data(struct Dht * const self, struct DhtData * const data);

/**
 * @brief Abort DHT transmission sequence
 *
 * @param self pointer to Dht instance
 */
void dht_abort_read(struct Dht * const self);

/**
 * @brief Handler to be executed on data line edge
 *
 * @param self pointer to Dht instance
 */
void dht_handle_data_line_edge(struct Dht * const self);

/**
 * @brief Get number of collected data line edges
 *
 * @param self pointer to Dht instance
 * @return
 *     - number of data line edges
 */
unsigned dht_get_edges_count(const struct Dht * const self);

/**
 * @brief Convert DhtStatus to string format
 *
 * @param DhtStatus
 * @return
 *     - DhtStatus in string format
 */
const char * dht_status_to_str(enum DhtStatus status);

/**
 * @brief Initialize Dht instance
 *
 * @param self pointer to Dht instance
 */
void dht_init(struct Dht * const self);


/*
 * DHT required interfaces
 * ========================
 * 
 * This section defines interfaces required by the DHT component
 * To be implemented in the platform specific code
 */

/**
 * @brief Configure data line as output
 * 
 * @param self pointer to Dht instance
 */
extern void dht_config_data_line_output(struct Dht * const self);

/**
 * @brief Configure data line as input
 * 
 * @param self pointer to Dht instance
 */
extern void dht_config_data_line_input(struct Dht * const self);

/**
 * @brief Enable data line interrupts
 *
 * @param self pointer to Dht instance
 */
extern void dht_data_line_irq_enable(struct Dht * const self);

/**
 * @brief Disable data line interrupts
 *
 * @param self pointer to Dht instance
 */
extern void dht_data_line_irq_disable(struct Dht * const self);

/**
 * @brief Set data line level high
 * 
 * @param self pointer to Dht instance
 */
extern void dht_set_data_line_high(struct Dht * const self);

/**
 * @brief Set data line level low
 * 
 * @param self pointer to Dht instance
 */
extern void dht_set_data_line_low(struct Dht * const self);

/**
 * @brief Get data line level
 * 
 * @param self pointer to Dht instance
 * @return
 *     - DHT_DATA_LOW
 *     - DHT_DATA_HIGH
 */
extern enum DhtDataLevel dht_get_data_line_level(struct Dht * const self);

/**
 * @brief Sleep
 * 
 * @note This interface can be implemented as blocking or not (e.g. if RTOS is used)
 * 
 * @param min_time_ms min sleep time in miliseconds
 * @param max_time_ms max sleep time in miliseconds
 */
extern void dht_sleep_ms(unsigned min_time_ms, unsigned max_time_ms);

/**
 * @brief Notify sequence completed
 * 
 * @note This interface can be implemented as blocking or not (e.g. if RTOS is used)
 * 
 * @param self pointer to Dht instance
 */
extern void dht_notify_sequence_completed(struct Dht const * self);

/**
 * @brief Wait for sequence completion notification
 * 
 * @note This interface can be implemented as blocking or not (e.g. if RTOS is used)
 * 
 * @param self pointer to Dht instance
 */
extern void dht_wait_for_completion(struct Dht const * self);

/**
 * @brief Get current time in microseconds
 * 
 * @return current global time in microseconds
 */
extern unsigned dht_get_microseconds(void);

#endif /* DHT_H_ */

