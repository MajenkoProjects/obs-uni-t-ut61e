#ifndef _DMM_H
#define _DMM_H

#include <libsigrok/libsigrok.h>
#include <obs-module.h>
#include <util/threading.h>
#include <util/platform.h>
#include <obs.h>

#define WIDTH 280
#define HEIGHT 180

struct dmm {

    obs_source_t *source;
    os_event_t *stop_signal;
    pthread_t vthread;
    pthread_t dthread;
    bool initialized;
	bool run;

	int fade;

    struct sr_context *sr_ctx;
    struct sr_session *session;

    time_t last_update;
	float previous_value;
    float current_value;
    int decimal;

	bool is_negative;

	char text[10];

	int previous_range;
	int current_range;

	int offset;

	bool milli;
	bool micro;

    bool hold;
    bool diode;
    bool continuity;
    bool delta;
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

#define DC				0x0000
#define AC				0x1000
#define VOLTS			0x0001
#define VOLTS_DC 		0x0001
#define VOLTS_AC 		0x1001
#define AMPS   			0x0002
#define AMPS_DC  		0x0002
#define AMPS_AC  		0x1002
#define OHMS			0x0004
#define CONT			0x0008
#define DIODE			0x0010
#define FARADS			0x0020
#define HZ				0x0040
#define DUTY			0x0080
#define DEGC			0x0100
#define DEGF			0x0200


extern void *video_thread(void *data);
extern void *dmm_thread(void *data);
extern void dmm_terminate(struct dmm *dmm);

#endif
