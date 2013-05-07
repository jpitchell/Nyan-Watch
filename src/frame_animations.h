/*
 * Pebble Frame Animations - Frame by Frame
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


typedef struct FrameAnimation 
{
	Layer animationLayer;
	BitmapLayer firstLayer;
	BitmapLayer secondLayer;
	RotBmpPairContainer transparentImageContainer;
	BmpContainer imageContainer;
	bool startsHidden : 1;
	bool isTransparent : 1;
	bool isAnimating : 1;
	bool scheduleStop : 1;
	int firstResourceId;
	int resourceCounter;
	int frameCounter;
	int numFrames;
} FrameAnimation;

void frame_animation_init(FrameAnimation *animation, Layer *destination, GPoint position, int first_resource_id, int num_frames, bool is_transparent, bool starts_hidden);
void frame_animation_linear(FrameAnimation *animation, AppContextRef ctx, AppTimerHandle timer_handle, uint32_t cookie, int fps, bool continuous);
void frame_animation_alternating(FrameAnimation *animation, AppContextRef ctx, AppTimerHandle timer_handle,  uint32_t cookie, int fps, bool continuous);
void frame_animation_stop(FrameAnimation *animation);
void frame_animation_deinit(FrameAnimation *animation);
Layer* get_animation_layer(FrameAnimation *animation);
bool isRunning(FrameAnimation *animation);

