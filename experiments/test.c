#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libsigrok/libsigrok.h>

// uni-t-ut61e:conn=1a86.e008


struct display {
	time_t last_update;
	float value;
	int decimal;
	bool DC;
	bool AC;
	bool hold;
	bool diode;
	bool continuity;
	bool delta;
	bool micro;
	bool milli;
	bool volts;
	bool amps;
	bool c;
	bool f;
	bool percent;
	bool mega;
	bool kilo;
	bool ohms;
	bool hz;
	bool millif;
	bool microf;
	bool nanof;
	bool farad;
	bool autorange;
	bool manurange;
};

struct display display_data;


void datafeed_in(const struct sr_dev_inst *sdi, const struct sr_datafeed_packet *packet, void *cb_data) {

	int scale = 0;

	if (SR_DF_ANALOG == packet->type) {

		display_data.last_update = time(NULL);
		const struct sr_datafeed_analog *data = packet->payload;

		float value;

		sr_analog_to_float(data, &value);

  		display_data.AC = (data->meaning->mqflags & SR_MQFLAG_AC);
    	display_data.DC = (data->meaning->mqflags & SR_MQFLAG_DC);
    	display_data.diode = (data->meaning->mqflags & SR_MQFLAG_DIODE);
    	display_data.hold = (data->meaning->mqflags & SR_MQFLAG_HOLD);
    	display_data.autorange = (data->meaning->mqflags & SR_MQFLAG_AUTORANGE);
    	display_data.manurange = !(data->meaning->mqflags & SR_MQFLAG_AUTORANGE);
    	display_data.delta = (data->meaning->mqflags & SR_MQFLAG_RELATIVE);

		switch (data->meaning->mq) {
 			case SR_MQ_VOLTAGE:
				display_data.volts = 1;

				if (value < 1.0) {
					value *= 1000.0;
					scale ++;
				}

				if (value < 1.0) {
					value *= 1000.0;
					scale ++;
				}

				display_data.milli = (scale == 1);
				display_data.micro = (scale == 2);
				display_data.value = value;
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

		}


		printf("[1;1H");
		if (display_data.DC) printf("DC"); else printf("  ");
		if (display_data.AC) printf("AC"); else printf("  ");

		if (display_data.hold) printf(" H "); else printf("   ");
		if (display_data.diode) printf("-|>-"); else printf("    ");
		if (display_data.continuity) printf("·))"); else printf("   ");

		if (display_data.delta) printf("Δ"); else printf(" ");
		if (display_data.micro) printf("µ"); else printf(" ");
		if (display_data.milli) printf("m"); else printf(" ");
		if (display_data.volts) printf("V"); else printf(" ");
		if (display_data.amps) printf("A"); else printf(" ");
		printf("\n");

		printf("%.4f\n", display_data.value);
		



	}

//struct sr_datafeed_analog {
//    void *data;
//    uint32_t num_samples;
//    struct sr_analog_encoding *encoding;
//    struct sr_analog_meaning *meaning;
//    struct sr_analog_spec *spec;

}


int main(int argc, char **argv) {
	struct sr_context *sr_ctx = NULL;
	struct sr_dev_driver *driver;
	struct sr_dev_driver **drivers;
	struct sr_config *conf;
	struct sr_session *session;
	struct sr_dev_inst *current_device = NULL;

	GSList *devices;
	GSList *drvopts = NULL;

	GVariantBuilder *vbl;

	GMainLoop *main_loop = NULL;

	int i;

	printf("[2J");


	if (sr_init(&sr_ctx) != SR_OK) {
		printf("Error creating context\n");
		return -1;
	}

	drivers = sr_driver_list(sr_ctx);
	driver = NULL;

	for (i = 0; drivers[i]; i++) {
		if (strcmp(drivers[i]->name, "uni-t-ut61e") == 0) {
			driver = drivers[i];
			break;
		}
	}

	if (NULL == driver) {
		printf("Error finding the driver. Is your sigrok up to date?\n");
		sr_exit(sr_ctx);
		return -1;
	}

	if (SR_OK != sr_driver_init(sr_ctx, driver)) {
		printf("Error initializing the driver\n");
		sr_exit(sr_ctx);
		return -1;
	}

	conf = malloc(sizeof(struct sr_config));

//	vbl = g_variant_builder_new(G_VARIANT_TYPE_DICTIONARY);
//	g_variant_builder_add(vbl, "{ss}", "conn", "1a86.e008");

	conf->key = SR_CONF_CONN;
//	conf->data = g_variant_builder_end(vbl);
	conf->data = g_variant_new_string("1a86.e008");

	drvopts = g_slist_append(drvopts, conf);


	while (1) {

		if (current_device == NULL) {
			sleep(1);

			devices = sr_driver_scan(driver, drvopts);

			if (devices == NULL) {
				continue;
			}


			current_device = devices->data;
			printf("Found %s\n", sr_dev_inst_model_get(current_device));
			g_slist_free(devices);

			sr_session_new(sr_ctx, &session);
			sr_session_datafeed_callback_add(session, datafeed_in, NULL);

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

//			main_loop = g_main_loop_new(NULL, FALSE);
//			sr_session_stopped_callback_set(session, (sr_session_stopped_callback)g_main_loop_quit, main_loop);

			sr_session_start(session);
		}

		printf("Started main loop\n");

//		g_main_loop_run(main_loop);
		sr_session_run(session);
		printf("Ended main loop\n");

		current_device = NULL;

		sleep(1);

	}
	



	return 0;
}
