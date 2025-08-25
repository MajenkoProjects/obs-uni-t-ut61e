#ifndef _DMM_H
#define _DMM_H

#include <libsigrok/libsigrok.h>
#include <obs-module.h>
#include <util/threading.h>
#include <util/platform.h>
#include <obs.h>

#define WIDTH 240
#define HEIGHT 180

struct dmm {

    obs_source_t *source;
    os_event_t *stop_signal;
    pthread_t vthread;
    pthread_t dthread;
    bool initialized;
	bool run;

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

    bool DC;
    bool AC;
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

enum ranges {
    VOLTS_DC = 1,
    VOLTS_AC,
    AMPS_DC,
    AMPS_AC,
    OHMS,
    CONT,
    DIODE,
    FARADS,
    HZ,
    DUTY,
    DEGC,
    DEGF,
};


extern void *video_thread(void *data);
extern void *dmm_thread(void *data);
extern void dmm_terminate(struct dmm *dmm);

#endif
