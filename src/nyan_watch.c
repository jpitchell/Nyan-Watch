/*
 * Copyright (C) 2013 Jeff Pitchell
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
* This library is for frame-by-frame animation within a layer. 
*
* PLEASE SEE THE README FOR DETAILED DOCUMENTATION ON HOW TO USE THIS LIBRARY
*/


#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "frame_animations.h"

#define MY_UUID { 0x64, 0x1A, 0x52, 0xB4, 0xC9, 0xF1, 0x44, 0x61, 0x82, 0x5F, 0xB8, 0xCE, 0x2B, 0x39, 0xE5, 0x61 }
PBL_APP_INFO(MY_UUID,
             "Nyan Watch", "Jeff Pitchell",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
FrameAnimation gif_animation;
AppTimerHandle timer_handle;
TextLayer text_time_layer;
TextLayer date_layer;

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
    (void)ctx;
    (void)handle;
    
    if (cookie == 1) {

      // Animate frames with parameters: 20 frames per second, repeat, cookie=1
      frame_animation_linear(&gif_animation, ctx, handle, 1, 20, true);
      // If you have no idea what this function is, have a look at the README in the src folder!

  }
}

void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t) {

  (void)ctx;

  PblTm *tickTime = t->tick_time;

  static char time_text[] = "00:00:00 AM";
  static char amPmText[] = "PM AM";
  static char date_text[] = "00/00/0000";

  string_format_time(amPmText, sizeof(amPmText), "%p", tickTime);
  string_format_time(date_text, sizeof(date_text), "%D", tickTime);

  const char *timeFormat = clock_is_24h_style() ? "%R" : "%I:%M";
  string_format_time(time_text, sizeof(time_text), timeFormat, tickTime);

  strcat(time_text, " ");
  strcat(time_text, amPmText);
  text_layer_set_text(&text_time_layer, time_text);
  text_layer_set_text(&date_layer, date_text);
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Main");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);
  resource_init_current_app(&APP_RESOURCES);

  frame_animation_init(&gif_animation, &window.layer, GPoint(0,0), RESOURCE_ID_FRAME_1, 12, false, true);
  timer_handle = app_timer_send_event(ctx, 100, 1);


  text_layer_init(&text_time_layer, GRect(0, 5, 144, 30));
  text_layer_set_text_color(&text_time_layer, GColorWhite);
  text_layer_set_background_color(&text_time_layer, GColorClear);
  text_layer_set_text_alignment(&text_time_layer, GTextAlignmentCenter);
  text_layer_set_font(&text_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(&window.layer, &text_time_layer.layer);

  text_layer_init(&date_layer, GRect(0, 130, 144, 30));
  text_layer_set_text_color(&date_layer, GColorWhite);
  text_layer_set_background_color(&date_layer, GColorClear);
  text_layer_set_text_alignment(&date_layer, GTextAlignmentCenter);
  text_layer_set_font(&date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(&window.layer, &date_layer.layer);

}


void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  frame_animation_deinit(&gif_animation);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .timer_handler = &handle_timer,

    .tick_info = {
      .tick_handler = &handle_second_tick,
      .tick_units = SECOND_UNIT
    }

  };
  app_event_loop(params, &handlers);
}
