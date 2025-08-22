#include <obs-module.h>
#include <util/threading.h>
#include <util/platform.h>
#include <obs.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "dmm.h"

static int singleton = 0;

static const char *dmm_get_name(void *unused) {
	UNUSED_PARAMETER(unused);
	return "UT61E DMM";
}

static void dmm_destroy(void *data) {
	struct dmm *dmm = data;

	if (dmm) {
		dmm->run = false;
		if (dmm->initialized) {
			os_event_signal(dmm->stop_signal);
		}

		if (dmm->vthread) {
			pthread_join(dmm->vthread, NULL);
		}

		if (dmm->dthread) {
			pthread_join(dmm->dthread, NULL);
		}

		if (dmm->stop_signal) {
			os_event_destroy(dmm->stop_signal);
		}

		if (dmm) {
			bfree(dmm);
		}
	}

	singleton = 0;
}

static void *dmm_create(obs_data_t *settings, obs_source_t *source) {

	if (singleton != 0) {
		return NULL;
	}

	singleton = 1;

	struct dmm *dmm = bzalloc(sizeof(struct dmm));

	dmm->source = source;
	dmm->run = true;

	if (os_event_init(&dmm->stop_signal, OS_EVENT_TYPE_MANUAL) != 0) {
		dmm_destroy(dmm);
		return NULL;
	}

	if (pthread_create(&dmm->vthread, NULL, video_thread, dmm) != 0) {
		dmm_destroy(dmm);
		return NULL;
	}

	if (pthread_create(&dmm->dthread, NULL, dmm_thread, dmm) != 0) {
		dmm_destroy(dmm);
		return NULL;
	}

	dmm->initialized = true;


	UNUSED_PARAMETER(settings);

	return dmm;
}

static uint32_t dmm_get_width(void *data) {
	UNUSED_PARAMETER(data);
	return WIDTH;
}

static uint32_t dmm_get_height(void *data) {
	UNUSED_PARAMETER(data);
	return HEIGHT;
}

struct obs_source_info ut61e_source = {
	.id				= "ut61e_source",
	.type			= OBS_SOURCE_TYPE_INPUT,
	.output_flags	= OBS_SOURCE_ASYNC_VIDEO,
	.get_name		= dmm_get_name,
	.create			= dmm_create,
	.destroy		= dmm_destroy,
	.get_width		= dmm_get_width,
	.get_height		= dmm_get_height,
};
