#include "coAP_thread.h"
#include "thread_ipc.h"


 struct sockaddr_storage server;
/**@brief Resolves the configured hostname. */
 int server_resolve(void)
{
	int err;
	struct addrinfo *result;
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_DGRAM
	};
	char ipv4_addr[NET_IPV4_ADDR_LEN];

	err = getaddrinfo(CONFIG_COAP_SERVER_HOSTNAME, NULL, &hints, &result);
	if (err != 0) {
		printk("ERROR: getaddrinfo failed %d\n", err);
		return -EIO;
	}

	if (result == NULL) {
		printk("ERROR: Address not found\n");
		return -ENOENT;
	}

	/* IPv4 Address. */
	struct sockaddr_in *server4 = ((struct sockaddr_in *)&server);

	server4->sin_addr.s_addr =
		((struct sockaddr_in *)result->ai_addr)->sin_addr.s_addr;
	server4->sin_family = AF_INET;
	server4->sin_port = htons(CONFIG_COAP_SERVER_PORT);

	inet_ntop(AF_INET, &server4->sin_addr.s_addr, ipv4_addr,
		  sizeof(ipv4_addr));
	printk("IPv4 Address found %s\n", ipv4_addr);

	/* Free the address. */
	freeaddrinfo(result);

	return 0;
}



 u16_t next_token;
 int sock;
 struct pollfd fds;
/**@brief Initialize the CoAP client */
 int client_init_coAP(void)
{
	int err;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		printk("Failed to create CoAP socket: %d.\n", errno);
		return -errno;
	}

	err = connect(sock, (struct sockaddr *)&server,
		      sizeof(struct sockaddr_in));
	if (err < 0) {
		printk("Connect failed : %d\n", errno);
		return -errno;
	}

	/* Initialize FDS, for poll. */
	fds.fd = sock;
	fds.events = POLLIN;

	/* Randomize token. */
	next_token = sys_rand32_get();

	return 0;
}



struct data_item_t tx_data;
 //u8_t *data_buf;
/**@brief Handles responses from the remote CoAP server. */
 int client_handle_get_response(u8_t *buf, int received)
{
	int err;
	struct coap_packet reply;
	const u8_t *payload;
	u16_t payload_len;
	u8_t token[8];
	u16_t token_len;
	u8_t temp_buf[16];

	err = coap_packet_parse(&reply, buf, received, NULL, 0);
	if (err < 0) {
		printk("Malformed response received: %d\n", err);
		return err;
	}

	payload = coap_packet_get_payload(&reply, &payload_len);
	token_len = coap_header_get_token(&reply, token);

	if ((token_len != sizeof(next_token)) &&
	    (memcmp(&next_token, token, sizeof(next_token)) != 0)) {
		printk("Invalid token received: 0x%02x%02x\n",
		       token[1], token[0]);
		return 0;
	}

	snprintf(temp_buf, MAX(payload_len, sizeof(temp_buf)), "%s", payload);
         //data_buf=temp_buf; /*data to be sent to broker from mqtt thread*/
	printk("CoAP response: code: 0x%x, token 0x%02x%02x, payload: %s\n",
	       coap_header_get_code(&reply), token[1], token[0], temp_buf);
        
        /* create data item to send to mqtt thread */
        
        strncpy( tx_data.data,temp_buf, 16);
       // tx_data.data=temp_buf;
        /* send data to consumer threads */
        k_fifo_put(&data_fifo,&tx_data);        

	return 0;
}



 u8_t coap_buf[APP_COAP_MAX_MSG_LEN];
/**@biref Send CoAP GET request. */
 int client_get_send(void)
{
	int err;
	struct coap_packet request;

	next_token++;

	err = coap_packet_init(&request, coap_buf, sizeof(coap_buf),
			       APP_COAP_VERSION, COAP_TYPE_NON_CON,
			       sizeof(next_token), (u8_t *)&next_token,
			       COAP_METHOD_GET, coap_next_id());
	if (err < 0) {
		printk("Failed to create CoAP request, %d\n", err);
		return err;
	}

	err = coap_packet_append_option(&request, COAP_OPTION_URI_PATH,
					(u8_t *)CONFIG_COAP_RESOURCE,
					strlen(CONFIG_COAP_RESOURCE));
	if (err < 0) {
		printk("Failed to encode CoAP option, %d\n", err);
		return err;
	}

	err = send(sock, request.data, request.offset, 0);
	if (err < 0) {
		printk("Failed to send CoAP request, %d\n", errno);
		return -errno;
	}

	printk("CoAP request sent: token 0x%04x\n", next_token);

	return 0;
}



#if defined(CONFIG_LWM2M_CARRIER)
K_SEM_DEFINE(carrier_registered, 0, 1);

void lwm2m_carrier_event_handler(const lwm2m_carrier_event_t *event)
{
	switch (event->type) {
	case LWM2M_CARRIER_EVENT_BSDLIB_INIT:
		printk("LWM2M_CARRIER_EVENT_BSDLIB_INIT\n");
		break;
	case LWM2M_CARRIER_EVENT_CONNECT:
		printk("LWM2M_CARRIER_EVENT_CONNECT\n");
		break;
	case LWM2M_CARRIER_EVENT_DISCONNECT:
		printk("LWM2M_CARRIER_EVENT_DISCONNECT\n");
		break;
	case LWM2M_CARRIER_EVENT_READY:
		printk("LWM2M_CARRIER_EVENT_READY\n");
		k_sem_give(&carrier_registered);
		break;
	case LWM2M_CARRIER_EVENT_FOTA_START:
		printk("LWM2M_CARRIER_EVENT_FOTA_START\n");
		break;
	case LWM2M_CARRIER_EVENT_REBOOT:
		printk("LWM2M_CARRIER_EVENT_REBOOT\n");
		break;
	}
}
#endif /* defined(CONFIG_LWM2M_CARRIER) */



/* Returns 0 if data is available.
 * Returns -EAGAIN if timeout occured and there is no data.
 * Returns other, negative error code in case of poll error.
 */
 int wait(int timeout)
{
	int ret = poll(&fds, 1, timeout);

	if (ret < 0) {
		printk("poll error: %d\n", errno);
		return -errno;
	}

	if (ret == 0) {
		/* Timeout. */
		return -EAGAIN;
	}

	if ((fds.revents & POLLERR) == POLLERR) {
		printk("wait: POLLERR\n");
		return -EIO;
	}

	if ((fds.revents & POLLNVAL) == POLLNVAL) {
		printk("wait: POLLNVAL\n");
		return -EBADF;
	}

	if ((fds.revents & POLLIN) != POLLIN) {
		return -EAGAIN;
	}

	return 0;
}



/**@brief coAP thread entry point 
 */
 void coAP_entry_point(void *a, void *b, void *c)
 {

s64_t next_msg_time = APP_COAP_SEND_INTERVAL_MS;
	int err, received;

	printk("The nRF CoAP client sample started\n");

	//modem_configure();

	if (server_resolve() != 0) {
		printk("Failed to resolve server name\n");
		return;
	}

	if (client_init_coAP() != 0) {
		printk("Failed to initialize CoAP client\n");
		return;
	}

	next_msg_time = k_uptime_get();

	while (1) {
		if (k_uptime_get() >= next_msg_time) {
			if (client_get_send() != 0) {
				printk("Failed to send GET request, exit...\n");
				break;
			}

			next_msg_time += APP_COAP_SEND_INTERVAL_MS;
		}

		s64_t remaining = next_msg_time - k_uptime_get();

		if (remaining < 0) {
			remaining = 0;
		}

		err = wait(remaining);
		if (err < 0) {
			if (err == -EAGAIN) {
				continue;
			}

			printk("Poll error, exit...\n");
			break;
		}

		received = recv(sock, coap_buf, sizeof(coap_buf), MSG_DONTWAIT);
		if (received < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				printk("socket EAGAIN\n");
				continue;
			} else {
				printk("Socket error, exit...\n");
				break;
			}
		}

		if (received == 0) {
			printk("Empty datagram\n");
			continue;
		}

		err = client_handle_get_response(coap_buf, received);
               // printk("payload from entry point: %s\n",temp_buf);
		if (err < 0) {
			printk("Invalid response, exit...\n");
			break;
		}
	}

	(void)close(sock);

 }














