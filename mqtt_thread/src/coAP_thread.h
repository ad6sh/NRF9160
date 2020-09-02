#include <stdio.h>
#include <string.h>

#include <zephyr.h>
#include <net/coap.h>
#include <net/socket.h>
#include <modem/lte_lc.h>
#if defined(CONFIG_LWM2M_CARRIER)
#include <lwm2m_carrier.h>
#endif

#define APP_COAP_SEND_INTERVAL_MS 5000
#define APP_COAP_MAX_MSG_LEN 1280
#define APP_COAP_VERSION 1

extern int sock;
extern struct pollfd fds;
extern  struct sockaddr_storage server;
extern u16_t next_token;

extern u8_t coap_buf[APP_COAP_MAX_MSG_LEN];


/*Parameters for coAP thread*/
#define COAP_STACK_SIZE 4096
#define COAP_THREAD_PRIORITY 3


/**@brief Resolves the configured hostname. */
extern int server_resolve(void);


/**@brief Initialize the CoAP client */
extern int client_init_coAP(void);

/**@brief Handles responses from the remote CoAP server. */
extern int client_handle_get_response(u8_t *buf, int received);

/**@biref Send CoAP GET request. */
extern int client_get_send(void);

/* Returns 0 if data is available.
 * Returns -EAGAIN if timeout occured and there is no data.
 * Returns other, negative error code in case of poll error.
 */
extern int wait(int timeout);


