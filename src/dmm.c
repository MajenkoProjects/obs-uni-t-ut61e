#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include "dmm.h"

// uni-t-ut61e:conn=1a86.e008

void dmm_terminate(struct dmm *dmm) {
	sr_session_stop(dmm->session);
}

void datafeed_in(const struct sr_dev_inst *sdi __attribute__((unused)), const struct sr_datafeed_packet *packet, void *cb_data) {

	struct dmm *dmm = cb_data;

	if (SR_DF_ANALOG == packet->type) {

		dmm->last_update = time(NULL);
		const struct sr_datafeed_analog *data = packet->payload;

		float value;

		sr_analog_to_float(data, &value);

    	dmm->autorange = (data->meaning->mqflags & SR_MQFLAG_AUTORANGE);
    	dmm->manurange = !(data->meaning->mqflags & SR_MQFLAG_AUTORANGE);

		dmm->previous_value = dmm->current_value;
		dmm->current_value = fabs(value);
		dmm->is_negative = (value < 0);

		dmm->previous_range = dmm->current_range;

		dmm->micro = false;
		dmm->milli = false;
		dmm->kilo = false;
		dmm->mega = false;

		switch (data->meaning->mq) {
 			case SR_MQ_VOLTAGE:
    		case SR_MQ_CURRENT:
				if (data->meaning->mqflags & SR_MQFLAG_AC) {
					if (data->meaning->mq == SR_MQ_VOLTAGE) {
						dmm->current_range = VOLTS_AC;
					} else {
						dmm->current_range = AMPS_AC;
					}
				} else {
					if (data->meaning->mq == SR_MQ_VOLTAGE) {
						dmm->current_range = VOLTS_DC;
					} else {
						dmm->current_range = AMPS_DC;
					}
				}



				/* Ranges
				 *  0 - No range
                 *  4 - 1.0000 µV
                 *  5 - 10.000 µV
                 *  6 - 100.00 µV
                 *  7 - 1.0000 mV
                 *  8 - 10.000 mV
                 *  9 - 100.00 mV
				 * 10 - 1.0000 V
				 * 11 - 10.000 V
				 * 12 - 100.00 V
				 * 13 - 1000.0 V
			 	 */

				if (dmm->current_range != dmm->previous_range) {
					dmm->offset = 0;
				}

				if (dmm->current_value < 0.000002 && dmm->offset > 4) {
					dmm->offset = 4;
				}
				if (dmm->current_value > 0.0000022 && dmm->offset < 5) {
					dmm->offset = 5;
				}

				if (dmm->current_value < 0.00002 && dmm->offset > 5) {
					dmm->offset = 5;
				}
				if (dmm->current_value > 0.000022 && dmm->offset < 6) {
					dmm->offset = 6;
				}

				if (dmm->current_value < 0.0002 && dmm->offset > 6) {
					dmm->offset = 6;
				}
				if (dmm->current_value > 0.00022 && dmm->offset < 7) {
					dmm->offset = 7;
				}

				if (dmm->current_value < 0.002 && dmm->offset > 7) {
					dmm->offset = 7;
				}
				if (dmm->current_value > 0.0022 && dmm->offset < 8) {
					dmm->offset = 8;
				}

				if (dmm->current_value < 0.02 && dmm->offset > 8) {
					dmm->offset = 8;
				}
				if (dmm->current_value > 0.022 && dmm->offset < 9) {
					dmm->offset = 9;
				}

				if (dmm->current_value < 0.2 && dmm->offset > 9) {
					dmm->offset = 9;
				}
				if (dmm->current_value > 0.22 && dmm->offset < 10) {
					dmm->offset = 10;
				}

				if (dmm->current_value < 2.0 && dmm->offset > 10) {
					dmm->offset = 10;
				}
				if (dmm->current_value > 2.2 && dmm->offset < 11) {
					dmm->offset = 11;
				}


				if (dmm->current_value < 20.0 && dmm->offset > 11) {
					dmm->offset = 11;
				}
				if (dmm->current_value > 22.0 && dmm->offset < 12) {
					dmm->offset = 12;
				}

				if (dmm->current_value < 200.0 && dmm->offset > 12) {
					dmm->offset = 12;
				}
				if (dmm->current_value > 220.0 && dmm->offset < 13) {
					dmm->offset = 13;
				}


				switch (dmm->offset) {
					case 4: 
						dmm->micro = true;
						snprintf(dmm->text, 10, "%6.4f", dmm->current_value * 1000000.0);
						break;
					case 5: 
						dmm->micro = true;
						snprintf(dmm->text, 10, "%6.3f", dmm->current_value * 1000000.0);
						break;
					case 6: 
						dmm->micro = true;
						snprintf(dmm->text, 10, "%6.2f", dmm->current_value * 1000000.0);
						break;
					case 7: 
						dmm->milli = true;
						snprintf(dmm->text, 10, "%6.4f", dmm->current_value * 1000.0);
						break;
					case 8: 
						dmm->milli = true;
						snprintf(dmm->text, 10, "%6.3f", dmm->current_value * 1000.0);
						break;
					case 9: 
						dmm->milli = true;
						snprintf(dmm->text, 10, "%6.2f", dmm->current_value * 1000.0);
						break;
					case 10: 
						snprintf(dmm->text, 10, "%6.4f", dmm->current_value);
						break;
					case 11: 
						snprintf(dmm->text, 10, "%6.3f", dmm->current_value);
						break;
					case 12: 
						snprintf(dmm->text, 10, "%6.2f", dmm->current_value);
						break;
					case 13: 
						snprintf(dmm->text, 10, "%6.1f", dmm->current_value);
						break;
						
				}

				if (isinf(dmm->current_value)) {
					snprintf(dmm->text, 20, "!0L!!");
				}

				for (char *ptr = dmm->text; *ptr; ptr++) {
					if (*ptr == ' ') *ptr = '!';
				}


				break;


    		case SR_MQ_RESISTANCE:

				dmm->current_range = OHMS;

				if (dmm->current_range != dmm->previous_range) {
					dmm->offset = 0;
				}

				if (dmm->current_value < 2.0 && dmm->offset > 10) {
					dmm->offset = 10;
				}
				if (dmm->current_value > 2.2 && dmm->offset < 11) {
					dmm->offset = 11;
				}


				if (dmm->current_value < 20.0 && dmm->offset > 11) {
					dmm->offset = 11;
				}
				if (dmm->current_value > 22.0 && dmm->offset < 12) {
					dmm->offset = 12;
				}

				if (dmm->current_value < 200.0 && dmm->offset > 12) {
					dmm->offset = 12;
				}
				if (dmm->current_value > 220.0 && dmm->offset < 13) {
					dmm->offset = 13;
				}

				if (dmm->current_value < 2000.0 && dmm->offset > 13) {
					dmm->offset = 13;
				}
				if (dmm->current_value > 2200.0 && dmm->offset < 14) {
					dmm->offset = 14;
				}

				if (dmm->current_value < 20000.0 && dmm->offset > 14) {
					dmm->offset = 14;
				}
				if (dmm->current_value > 22000.0 && dmm->offset < 15) {
					dmm->offset = 15;
				}

				if (dmm->current_value < 200000.0 && dmm->offset > 15) {
					dmm->offset = 15;
				}
				if (dmm->current_value > 220000.0 && dmm->offset < 16) {
					dmm->offset = 15;
				}

				if (dmm->current_value < 2000000.0 && dmm->offset > 16) {
					dmm->offset = 16;
				}
				if (dmm->current_value > 2200000.0 && dmm->offset < 17) {
					dmm->offset = 17;
				}

				if (dmm->current_value < 20000000.0 && dmm->offset > 17) {
					dmm->offset = 17;
				}
				if (dmm->current_value > 22000000.0 && dmm->offset < 18) {
					dmm->offset = 18;
				}


				switch (dmm->offset) {
					case 10: 
						snprintf(dmm->text, 10, "%6.4f", dmm->current_value);
						break;
					case 11: 
						snprintf(dmm->text, 10, "%6.3f", dmm->current_value);
						break;
					case 12: 
						snprintf(dmm->text, 10, "%6.2f", dmm->current_value);
						break;
					case 13: 
						dmm->kilo = true;
						snprintf(dmm->text, 10, "%6.4f", dmm->current_value / 1000);
						break;
					case 14: 
						dmm->kilo = true;
						snprintf(dmm->text, 10, "%6.3f", dmm->current_value / 1000);
						break;
					case 15: 
						dmm->kilo = true;
						snprintf(dmm->text, 10, "%6.2f", dmm->current_value / 1000);
						break;
					case 16: 
						dmm->mega = true;
						snprintf(dmm->text, 10, "%6.4f", dmm->current_value / 1000000);
						break;
					case 17: 
						dmm->mega = true;
						snprintf(dmm->text, 10, "%6.3f", dmm->current_value / 1000000);
						break;
					case 18: 
						dmm->mega = true;
						snprintf(dmm->text, 10, "%6.2f", dmm->current_value / 1000000);
						break;
						
				}

				if (isinf(dmm->current_value)) {
					snprintf(dmm->text, 20, "!0L!!");
				}

				for (char *ptr = dmm->text; *ptr; ptr++) {
					if (*ptr == ' ') *ptr = '!';
				}


				break;

			case SR_MQ_CAPACITANCE:
				break;

			case SR_MQ_TEMPERATURE:
				break;

			case SR_MQ_FREQUENCY:
				break;

			case SR_MQ_DUTY_CYCLE:
				break;

			case SR_MQ_CONTINUITY:
				break;

			default:
				break;

		}

	}

}

void *dmm_thread(void *data) {

	struct dmm *dmm = data;

	dmm->sr_ctx = NULL;

	struct sr_dev_driver *driver;
	struct sr_dev_driver **drivers;
	struct sr_config *conf;
	struct sr_dev_inst *current_device = NULL;

	GSList *devices;
	GSList *drvopts = NULL;

	int i;

	if (sr_init(&dmm->sr_ctx) != SR_OK) {
		printf("Error creating context\n");
		return NULL;
	}

	drivers = sr_driver_list(dmm->sr_ctx);
	driver = NULL;

	for (i = 0; drivers[i]; i++) {
		if (strcmp(drivers[i]->name, "uni-t-ut61e") == 0) {
			driver = drivers[i];
			break;
		}
	}

	if (NULL == driver) {
		printf("Error finding the driver. Is your sigrok up to date?\n");
		sr_exit(dmm->sr_ctx);
		return NULL;
	}

	if (SR_OK != sr_driver_init(dmm->sr_ctx, driver)) {
		printf("Error initializing the driver\n");
		sr_exit(dmm->sr_ctx);
		return NULL;
	}

	conf = malloc(sizeof(struct sr_config));


	conf->key = SR_CONF_CONN;
	conf->data = g_variant_new_string("1a86.e008");

	drvopts = g_slist_append(drvopts, conf);

    while (os_event_try(dmm->stop_signal) == EAGAIN) {

		if (current_device == NULL) {
			os_sleepto_ns(os_gettime_ns() + 30000000);

			devices = sr_driver_scan(driver, drvopts);

			if (devices == NULL) {
				continue;
			}


			current_device = devices->data;
			printf("Found %s\n", sr_dev_inst_model_get(current_device));
			g_slist_free(devices);

			sr_session_new(dmm->sr_ctx, &dmm->session);
			sr_session_datafeed_callback_add(dmm->session, datafeed_in, dmm);

			if (SR_OK != sr_dev_open(current_device)) {
				printf("Unable to open device!\n");
				sr_session_destroy(dmm->session);
				current_device = NULL;
				continue;
			}

			if (SR_OK != sr_session_dev_add(dmm->session, current_device)) {
				printf("Error adding device to the session\n");
				sr_session_destroy(dmm->session);
				current_device = NULL;
				continue;
			}

			sr_session_start(dmm->session);
		}

		sr_session_run(dmm->session);
		printf("Ended main loop\n");

		current_device = NULL;

		os_sleepto_ns(os_gettime_ns() + 30000000);

	}

	sr_session_destroy(dmm->session);
	

	free(conf);


	return NULL;
}
