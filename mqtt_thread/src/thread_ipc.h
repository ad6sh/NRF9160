#include <kernel.h>

/*data to be sent to broker from mqtt thread,assigning value from coAP thread*/
//extern u8_t *data_buf;

extern struct k_fifo data_fifo;

struct data_item_t {
  void *fifo_reserved;   /* 1st word reserved for use by fifo */
  u8_t data[16];   /* to store time data*/
};

extern struct data_item_t tx_data;
