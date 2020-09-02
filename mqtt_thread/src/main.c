#include "mqtt_thread.h"
#include "coAP_thread.h"

/*params for coAP thread*/
extern void coAP_entry_point(void *, void *, void *);
K_THREAD_STACK_DEFINE(coAP_stack_area, COAP_STACK_SIZE);
struct k_thread coAP_thread_data;

/*params for MQTT thread sending data*/
extern void mqtt_entry_point(void *, void *, void *);
K_THREAD_STACK_DEFINE(mqtt_stack_area, MQTT_STACK_SIZE);
struct k_thread mqtt_thread_data;

/*params for MQTT poll thread*/
extern void mqtt_poll_entry_point(void *, void *, void *);
K_THREAD_STACK_DEFINE(mqtt_poll_stack_area, MQTT_POLL_STACK_SIZE);
struct k_thread mqtt_poll_thread_data;





void main(void)
{
	init();
        
         k_tid_t coAP_tid = k_thread_create(&coAP_thread_data, coAP_stack_area,
                                 K_THREAD_STACK_SIZEOF(coAP_stack_area),
                                 coAP_entry_point,
                                 NULL, NULL, NULL,
                                 COAP_THREAD_PRIORITY, 0, K_NO_WAIT);
         
         
         k_tid_t mqtt_poll_tid = k_thread_create(&mqtt_poll_thread_data, mqtt_poll_stack_area,
                                 K_THREAD_STACK_SIZEOF(mqtt_poll_stack_area),
                                 mqtt_poll_entry_point,
                                 NULL, NULL, NULL,
                                 MQTT_POLL_THREAD_PRIORITY, 0, K_NO_WAIT);
        


         k_tid_t mqtt_tid = k_thread_create(&mqtt_thread_data, mqtt_stack_area,
                                 K_THREAD_STACK_SIZEOF(mqtt_stack_area),
                                 mqtt_entry_point,
                                 NULL, NULL, NULL,
                                 MQTT_THREAD_PRIORITY, 0, K_NO_WAIT);

}
