#include <zephyr.h>
#include <stdio.h>
#include <drivers/uart.h>
#include <string.h>

#include <net/mqtt.h>
#include <net/socket.h>
#include <modem/lte_lc.h>
#if defined(CONFIG_LWM2M_CARRIER)
#include <lwm2m_carrier.h>
#endif


/*Parameters for mqtt thread*/
#define MQTT_STACK_SIZE 4096
#define MQTT_THREAD_PRIORITY 5
#define MQTT_POLL_THREAD_PRIORITY 4
#define MQTT_POLL_STACK_SIZE 4096




/* Buffers for MQTT client. */
extern  u8_t rx_buffer[CONFIG_MQTT_MESSAGE_BUFFER_SIZE];
extern u8_t tx_buffer[CONFIG_MQTT_MESSAGE_BUFFER_SIZE];
extern u8_t payload_buf[CONFIG_MQTT_PAYLOAD_BUFFER_SIZE];

/* The mqtt client struct */
extern struct mqtt_client client;

/* MQTT Broker details. */
extern struct sockaddr_storage broker;

/* Connected flag */
 extern bool connected;

/* File descriptor */
 extern struct pollfd fds_mqtt;

/* Error return for mqtt functions*/
 extern int err;

/*message to be sent to broker*/
//extern u8_t *msg;




/**@brief Recoverable BSD library error. */
extern void bsd_recoverable_error_handler(uint32_t err);

/**@brief Function to print strings without null-termination
 */
extern  void data_print(u8_t *prefix, u8_t *data, size_t len);

/**@brief Function to publish data on the configured topic
 */
 extern int data_publish(struct mqtt_client *c, enum mqtt_qos qos,
	u8_t *data, size_t len);

/**@brief Function to subscribe to the configured topic
*/
 extern int subscribe(void);

/**@brief Function to read the published payload.
 */
 extern int publish_get_payload(struct mqtt_client *c, size_t length);

/**@brief MQTT client event handler
 */
extern void mqtt_evt_handler(struct mqtt_client *const c,
		      const struct mqtt_evt *evt);

/**@brief Resolves the configured hostname and
 * initializes the MQTT broker structure
 */
 extern void broker_init(void);

/**@brief Initialize the MQTT client structure
 */
 extern void client_init(struct mqtt_client *client);

/**@brief Initialize the file descriptor structure used by poll.
 */
extern int fds_init(struct mqtt_client *c);

/**@brief Configures modem to provide LTE link. Blocks until link is
 * successfully established.
 */
 extern void modem_configure(void);

/**@brief Initializes the mqtt related functions
 */
extern void init(void);