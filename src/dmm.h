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

extern void *video_thread(void *data);
extern void *dmm_thread(void *data);


#endif
