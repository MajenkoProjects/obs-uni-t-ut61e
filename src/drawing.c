#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <gd.h>

#include "dmm.h"

void *video_thread(void *data) {
	struct dmm *dmm = data;
	uint32_t pixels[WIDTH * HEIGHT];
	uint64_t cur_time = os_gettime_ns();

	struct obs_source_frame frame = {
		.data = {[0] = (uint8_t *)pixels},
		.linesize = {[0] = WIDTH * 4},
		.width = WIDTH,
		.height = HEIGHT,
		.format = VIDEO_FORMAT_BGRA,
	};


	while (os_event_try(dmm->stop_signal) == EAGAIN) {
//		gdImagePtr pic = // Create image
//		int op = 0;
//		for (int y = 0; y < HEIGHT; y++) {
//			for (int x = 0; x < WIDTH; x++) {
//				pixels[op++] = 0xFF000000 | palette[gdImageGetPixel(pic, x, y) & 0xF];
//			}
//		}
//		gdImageDestroy(pic);
		frame.timestamp = cur_time;
		obs_source_output_video(dmm->source, &frame);
		os_sleepto_ns(cur_time += 30000000);
	}
	return NULL;
}
