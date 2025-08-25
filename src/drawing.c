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

const char *text_font = "LiberationSans-Bold.ttf";
const char *seg_font = "DSEG7Classic-Italic.ttf";

const char shadow_offset = 2;

void *video_thread(void *data) {
	struct dmm *dmm = data;
	uint32_t pixels[WIDTH * HEIGHT];
	uint64_t cur_time = os_gettime_ns();

	gdImagePtr background;

	background = gdImageCreateTrueColor(WIDTH, HEIGHT);
	gdImageSaveAlpha(background, true);
	gdImageAlphaBlending(background, true);

	int bgcol = gdImageColorAllocateAlpha(background, 128, 228, 188, 0);
	int speckle = gdImageColorAllocateAlpha(background, 115, 115, 115, 110);

	gdImageFilledRectangle(background, 0, 0, WIDTH, HEIGHT, bgcol);

	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			if ((rand() % 100) > 80) {
				gdImageSetPixel(background, x, y, speckle);
			}
		}
	}

	struct obs_source_frame frame = {
		.data = {[0] = (uint8_t *)pixels},
		.linesize = {[0] = WIDTH * 4},
		.width = WIDTH,
		.height = HEIGHT,
		.format = VIDEO_FORMAT_BGRA,
	};


	while (os_event_try(dmm->stop_signal) == EAGAIN) {

		gdImagePtr img = gdImageCreateTrueColor(WIDTH, HEIGHT);
		gdImageSaveAlpha(img, true);
		gdImageAlphaBlending(img, true);

		gdImageCopy(img, background, 0, 0, 0, 0, WIDTH, HEIGHT);

		gdImagePtr lcd = gdImageCreateTrueColor(WIDTH, HEIGHT);
		gdImageSaveAlpha(lcd, true);
		gdImageAlphaBlending(lcd, false);

		int lcd_bg = gdImageColorAllocateAlpha(lcd, 200, 0, 0, 127);
		int lcd_shadow = gdImageColorAllocateAlpha(lcd, 0, 0, 0, 90);
		int lcd_hint = gdImageColorAllocateAlpha(lcd, 0, 0, 0, 120);
		int lcd_text = gdImageColorAllocateAlpha(lcd, 0, 0, 0, (rand() % 10) + 40);

		gdImageFilledRectangle(lcd, 0, 0, WIDTH, HEIGHT, lcd_bg);
		gdImageAlphaBlending(lcd, true);

		if (dmm->DC) gdImageStringTTF(lcd, NULL, lcd_shadow, text_font, 10, 0, 20 + shadow_offset, 20 + shadow_offset, "DC");
		if (dmm->AC) gdImageStringTTF(lcd, NULL, lcd_shadow, text_font, 10, 0, 40 + shadow_offset, 20 + shadow_offset, "AC");
		gdImageStringTTF(lcd, NULL, lcd_shadow, seg_font, 40, 0, 20 + shadow_offset, 80 + shadow_offset, dmm->text);

		if (dmm->is_negative) gdImageFilledRectangle(lcd, 1 + shadow_offset, 50 + shadow_offset, 14 + shadow_offset, 53 + shadow_offset, lcd_shadow);

		if (dmm->micro) gdImageStringTTF(lcd, NULL, lcd_shadow, text_font, 10, 0, 204, 24, "µ");
		if (dmm->milli) gdImageStringTTF(lcd, NULL, lcd_shadow, text_font, 10, 0, 214, 24, "m");
		if (dmm->current_range == VOLTS_AC) gdImageStringTTF(lcd, NULL, lcd_shadow, text_font, 10, 0, 220 + shadow_offset, 20 + shadow_offset, "V");
		if (dmm->current_range == VOLTS_DC) gdImageStringTTF(lcd, NULL, lcd_shadow, text_font, 10, 0, 220 + shadow_offset, 20 + shadow_offset, "V");
		if (dmm->current_range == AMPS_AC) gdImageStringTTF(lcd, NULL, lcd_shadow, text_font, 10, 0, 230 + shadow_offset, 20 + shadow_offset, "A");
		if (dmm->current_range == AMPS_DC) gdImageStringTTF(lcd, NULL, lcd_shadow, text_font, 10, 0, 230 + shadow_offset, 20 + shadow_offset, "A");




		gdImageStringTTF(lcd, NULL, lcd_hint, text_font, 10, 0, 20, 20, "DC");
		gdImageStringTTF(lcd, NULL, lcd_hint, text_font, 10, 0, 40, 20, "AC");
		gdImageStringTTF(lcd, NULL, lcd_hint, seg_font, 40, 0, 20, 80, "8.8888");
		gdImageFilledRectangle(lcd, 1, 50, 14, 53, lcd_hint);

		gdImageStringTTF(lcd, NULL, lcd_hint, text_font, 10, 0, 200, 20, "µ");
		gdImageStringTTF(lcd, NULL, lcd_hint, text_font, 10, 0, 210, 20, "m");
		gdImageStringTTF(lcd, NULL, lcd_hint, text_font, 10, 0, 220, 20, "V");
		gdImageStringTTF(lcd, NULL, lcd_hint, text_font, 10, 0, 230, 20, "A");




		if (dmm->DC) gdImageStringTTF(lcd, NULL, lcd_text, "LiberationSans-Bold.ttf", 10, 0, 20, 20, "DC");
		if (dmm->AC) gdImageStringTTF(lcd, NULL, lcd_text, "LiberationSans-Bold.ttf", 10, 0, 40, 20, "AC");
		gdImageStringTTF(lcd, NULL, lcd_text, "DSEG7Classic-Italic.ttf", 40, 0, 20, 80, dmm->text);
		if (dmm->is_negative) gdImageFilledRectangle(lcd, 1, 50, 14, 53, lcd_text);

		if (dmm->micro) gdImageStringTTF(lcd, NULL, lcd_text, text_font, 10, 0, 200, 20, "µ");
		if (dmm->milli) gdImageStringTTF(lcd, NULL, lcd_text, text_font, 10, 0, 210, 20, "m");
		if (dmm->current_range == VOLTS_DC) gdImageStringTTF(lcd, NULL, lcd_text, text_font, 10, 0, 220, 20, "V");
		if (dmm->current_range == VOLTS_AC) gdImageStringTTF(lcd, NULL, lcd_text, text_font, 10, 0, 220, 20, "V");
		if (dmm->current_range == AMPS_DC) gdImageStringTTF(lcd, NULL, lcd_text, text_font, 10, 0, 230, 20, "A");
		if (dmm->current_range == AMPS_AC) gdImageStringTTF(lcd, NULL, lcd_text, text_font, 10, 0, 230, 20, "A");




		gdImageCopy(img, lcd, 0, 0, 0, 0, WIDTH, HEIGHT);

		gdImageDestroy(lcd);

		int op = 0;
		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				pixels[op++] = 0xFF000000 | gdImageGetPixel(img, x, y);
			}
		}
		gdImageDestroy(img);
		frame.timestamp = cur_time;
		obs_source_output_video(dmm->source, &frame);
		os_sleepto_ns(cur_time += 30000000);
	}

	dmm_terminate(dmm);

	return NULL;
}
