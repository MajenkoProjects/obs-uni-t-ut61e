#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include "dmm.h"

// uni-t-ut61e:conn=1a86.e008


void datafeed_in(const struct sr_dev_inst *sdi __attribute__((unused)), const struct sr_datafeed_packet *packet, void *cb_data) {

	struct dmm *dmm = cb_data;

	int scale = 0;

	if (SR_DF_ANALOG == packet->type) {

		dmm->last_update = time(NULL);
		const struct sr_datafeed_analog *data = packet->payload;

		float value;

		sr_analog_to_float(data, &value);

  		dmm->AC = (data->meaning->mqflags & SR_MQFLAG_AC);
    	dmm->DC = (data->meaning->mqflags & SR_MQFLAG_DC);
    	dmm->diode = (data->meaning->mqflags & SR_MQFLAG_DIODE);
    	dmm->hold = (data->meaning->mqflags & SR_MQFLAG_HOLD);
    	dmm->autorange = (data->meaning->mqflags & SR_MQFLAG_AUTORANGE);
    	dmm->manurange = !(data->meaning->mqflags & SR_MQFLAG_AUTORANGE);
    	dmm->delta = (data->meaning->mqflags & SR_MQFLAG_RELATIVE);

		switch (data->meaning->mq) {
 			case SR_MQ_VOLTAGE:
				dmm->volts = 1;

				if (value < 1.0) {
					value *= 1000.0;
					scale ++;
				}

				if (value < 1.0) {
					value *= 1000.0;
					scale ++;
				}

				dmm->milli = (scale == 1);
				dmm->micro = (scale == 2);
				dmm->value = value;
				break;

    		case SR_MQ_CURRENT:
				break;

    		case SR_MQ_RESISTANCE:
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
	struct sr_session *session;
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

			sr_session_new(dmm->sr_ctx, &session);
			sr_session_datafeed_callback_add(session, datafeed_in, dmm);

			if (SR_OK != sr_dev_open(current_device)) {
				printf("Unable to open device!\n");
				sr_session_destroy(session);
				current_device = NULL;
				continue;
			}

			if (SR_OK != sr_session_dev_add(session, current_device)) {
				printf("Error adding device to the session\n");
				sr_session_destroy(session);
				current_device = NULL;
				continue;
			}

			sr_session_start(session);
		}

		sr_session_run(session);
		printf("Ended main loop\n");

		current_device = NULL;

		os_sleepto_ns(os_gettime_ns() + 30000000);

	}
	



	return NULL;
}
