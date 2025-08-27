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
const char *seg_font = "DSEG7Classic-BoldItalic.ttf";
const char *dmm_font = "DMM.ttf";

const char shadow_offset = 2;


#define COORD(X, Y) ((X) + offset) * WIDTH, ((Y) + offset) * WIDTH
#define SIZE(X) (X) * WIDTH


void main_image(gdImagePtr img, struct dmm *dmm, float offset, int color) {

	if ((dmm == NULL) || (dmm->current_range & VOLTS) || (dmm->current_range & AMPS)) {
		if ((dmm == NULL) || (dmm->current_range & 0xF000) == DC)
			gdImageStringTTF(img, NULL, color, text_font, SIZE(0.035), 0, COORD(0.08, 0.06), "DC");
		if ((dmm == NULL) || (dmm->current_range & 0xF000) == AC)
			gdImageStringTTF(img, NULL, color, text_font, SIZE(0.035), 0, COORD(0.15, 0.06), "AC");
	}

	if ((dmm == NULL) || 0) gdImageStringTTF(img, NULL, color, dmm_font, SIZE(0.070), 0, COORD(0.30, 0.062), "H");

	if ((dmm == NULL) || ((dmm->current_range & 0xFFF) == DIODE))
		gdImageStringTTF(img, NULL, color, dmm_font, SIZE(0.055), 0, COORD(0.55, 0.062), "D");
	if ((dmm == NULL) || (dmm->current_range & (CONT))) 
		gdImageStringTTF(img, NULL, color, dmm_font, SIZE(0.070), 0, COORD(0.63, 0.062), "C");
	if ((dmm == NULL) || 0) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.060), 0, COORD(0.69, 0.062), "Δ");
	if ((dmm == NULL) || dmm->micro) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.035), 0, COORD(0.8, 0.06), "µ");
	if ((dmm == NULL) || dmm->milli) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.035), 0, COORD(0.83, 0.06), "m");

	if ((dmm == NULL) || ((dmm->current_range & (DIODE | VOLTS))))
		gdImageStringTTF(img, NULL, color, text_font, SIZE(0.035), 0, COORD(0.87, 0.06), "V");
	if ((dmm == NULL) || ((dmm->current_range & 0xFFF) == AMPS)) 
		gdImageStringTTF(img, NULL, color, text_font, SIZE(0.035), 0, COORD(0.90, 0.06), "A");

	if ((dmm == NULL) || 0) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.040), 0, COORD(0.835, 0.1), "°");
	if ((dmm == NULL) || 0) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.040), 0, COORD(0.87, 0.1), "°");

	if ((dmm == NULL) || 0) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.040), 0, COORD(0.84, 0.12), "C");
	if ((dmm == NULL) || 0) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.040), 0, COORD(0.88, 0.12), "F");

	if ((dmm == NULL) || (dmm->current_range & 0xFFF) == DUTY) 
		gdImageStringTTF(img, NULL, color, text_font, SIZE(0.050), 0, COORD(0.92, 0.11), "%");

	if ((dmm == NULL) || dmm->mega) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.040), 0, COORD(0.84, 0.16), "M");
	if ((dmm == NULL) || dmm->kilo) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.040), 0, COORD(0.88, 0.16), "k");

	if ((dmm == NULL) || (dmm->current_range & (CONT | OHMS))) 
		gdImageStringTTF(img, NULL, color, text_font, SIZE(0.040), 0, COORD(0.92, 0.16), "Ω");

	if ((dmm == NULL) || (dmm->current_range & 0xFFF) == HZ) 
		gdImageStringTTF(img, NULL, color, text_font, SIZE(0.040), 0, COORD(0.90, 0.20), "Hz");

	if ((dmm == NULL) || dmm->millif) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.040), 0, COORD(0.82, 0.24), "m");
	if ((dmm == NULL) || dmm->microf) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.040), 0, COORD(0.87, 0.24), "µ");
	if ((dmm == NULL) || dmm->nanof) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.040), 0, COORD(0.905, 0.24), "n");
	if ((dmm == NULL) || (dmm->current_range & 0xFFF) == FARADS) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.040), 0, COORD(0.935, 0.24), "F");


	if ((dmm == NULL) || dmm->autorange) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.040), 0, COORD(0.28, 0.35), "AUTO");
	if ((dmm == NULL) || dmm->manurange) gdImageStringTTF(img, NULL, color, text_font, SIZE(0.040), 0, COORD(0.45, 0.35), "MANU");


	if ((dmm == NULL) || dmm->is_negative) gdImageFilledRectangle(img, COORD(0.01, 0.175), COORD(0.07, 0.195), color);

	if (dmm == NULL) {
		gdImageStringTTF(img, NULL, color, seg_font, SIZE(0.14), 0, COORD(0.08, 0.28), "8.8.8.8.8");
	} else {
		gdImageStringTTF(img, NULL, color, seg_font, SIZE(0.14), 0, COORD(0.08, 0.28), dmm->text);
	}
}


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
		if ((time(NULL) - dmm->last_update) < 4) {
			if (dmm->fade < 255) {
				dmm->fade += 16;
			}
		} else {
			if (dmm->fade > 0) {
				dmm->fade -= 16;
			}
		}

		if (dmm->fade > 255) dmm->fade = 255;
		if (dmm->fade < 0) dmm->fade = 0;

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


		main_image(lcd, dmm, 0.01, lcd_shadow);
		main_image(lcd, NULL, 0, lcd_hint);
		main_image(lcd, dmm, 0, lcd_text);


		gdImageCopy(img, lcd, 0, 0, 0, 0, WIDTH, HEIGHT);

		gdImageDestroy(lcd);

		int op = 0;
		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				pixels[op++] = (dmm->fade << 24) | gdImageGetPixel(img, x, y);
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
