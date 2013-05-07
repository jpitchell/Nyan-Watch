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


#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "frame_animations.h"


Layer* destinationLayer;

// Initialize layer to be used for animating, and put it on top of the destination layer.
void frame_animation_init(FrameAnimation *animation, Layer *destination, GPoint position, 
		int first_resource_id, int num_frames, bool is_transparent, bool starts_hidden) {

	// General initialization for FrameAnimation struct
	animation->startsHidden = starts_hidden;
	animation->isTransparent = is_transparent;
	animation->firstResourceId = first_resource_id;
	animation->resourceCounter = first_resource_id;
	animation->numFrames = num_frames;
	animation->frameCounter = 0;
	animation->isAnimating = false;

	GRect rect; // to hold GRect of animation layer

	if (is_transparent) {
		rotbmp_pair_init_container(first_resource_id, first_resource_id+1, &animation->transparentImageContainer);
		rect = animation->transparentImageContainer.layer.layer.frame;
		layer_init(&animation->animationLayer, GRect(position.x, position.y, rect.size.w, rect.size.h));

		bitmap_layer_init(&animation->firstLayer, animation->transparentImageContainer.layer.layer.frame);
		bitmap_layer_init(&animation->secondLayer, animation->transparentImageContainer.layer.layer.frame);
		bitmap_layer_set_compositing_mode(&animation->firstLayer, GCompOpOr);
		bitmap_layer_set_compositing_mode(&animation->secondLayer, GCompOpClear);
		bitmap_layer_set_bitmap(&animation->firstLayer, &animation->transparentImageContainer.white_bmp);
		bitmap_layer_set_bitmap(&animation->secondLayer, &animation->transparentImageContainer.black_bmp);

		layer_add_child(&animation->animationLayer, &animation->firstLayer.layer);
		layer_add_child(&animation->animationLayer, &animation->secondLayer.layer);
	}
	else {
		bmp_init_container(first_resource_id, &animation->imageContainer);
		rect = animation->imageContainer.layer.layer.frame;
		layer_init(&animation->animationLayer, GRect(position.x, position.y, rect.size.w, rect.size.h));

		bitmap_layer_init(&animation->firstLayer, animation->imageContainer.layer.layer.frame);
		bitmap_layer_set_bitmap(&animation->firstLayer, &animation->imageContainer.bmp);
		layer_add_child(&animation->animationLayer, &animation->firstLayer.layer);
	}

	layer_add_child(destination, &animation->animationLayer);
	if (animation->startsHidden)
		layer_set_hidden(&animation->animationLayer, true);

	destinationLayer = destination; // grab a pointer to the main window

}

void frame_animation_deinit(FrameAnimation *animation) {
	if (animation->isTransparent) {
		rotbmp_pair_deinit_container(&animation->transparentImageContainer);
	}
	else {
		bmp_deinit_container(&animation->imageContainer);
	}
}

Layer* get_animation_layer(FrameAnimation *animation) {
	return &animation->animationLayer;
}

bool isRunning(FrameAnimation *animation) {
	return animation->isAnimating;
}

void show_current_frame(FrameAnimation *animation) {
	// Show the layer before starting animation
	layer_set_hidden(&animation->animationLayer, false);

	if (animation->isTransparent) {
		rotbmp_pair_deinit_container(&animation->transparentImageContainer);
		rotbmp_pair_init_container(animation->resourceCounter, animation->resourceCounter+1, &animation->transparentImageContainer);
		bitmap_layer_set_bitmap(&animation->firstLayer, &animation->transparentImageContainer.white_bmp);
		bitmap_layer_set_bitmap(&animation->secondLayer, &animation->transparentImageContainer.black_bmp);
	}
	else {
		bmp_deinit_container(&animation->imageContainer);
		bmp_init_container(animation->resourceCounter, &animation->imageContainer);
		bitmap_layer_set_bitmap(&animation->firstLayer, &animation->imageContainer.bmp);
	}

	layer_mark_dirty(destinationLayer); // mark the destination window dirty in order to show the next frame
}

// This function will animate from the first frame to the last, repeating if 'continuous' is true.
void frame_animation_linear(FrameAnimation *animation, AppContextRef ctx, 
		AppTimerHandle timer_handle, uint32_t cookie, int fps, bool continuous) {

	animation->isAnimating = true; // set flag to tell if the animation is running

	int num_frames = animation->numFrames; // temporary variable to store number of frames
	if (continuous)
		num_frames = num_frames - 1; // This is needed to prevent the frame from showing empty space

	int millis_from_fps = 1000 / fps; // Get frame delay in milliseconds from desired FPS


	show_current_frame(animation);

	if (animation->frameCounter < num_frames) {
		if (animation->isTransparent)
			animation->resourceCounter = animation->resourceCounter + 2;
		else
			animation->resourceCounter++;
		animation->frameCounter++;
	}
	else {
		animation->resourceCounter = animation->firstResourceId;
		animation->frameCounter = 0;
		// If not continuous, we need to stop the timer and hide the layer
		if (!continuous || animation->scheduleStop) {
			app_timer_cancel_event(ctx, timer_handle);
			animation->isAnimating = false;
			animation->scheduleStop = false;
			if (animation->startsHidden)
				layer_set_hidden(&animation->animationLayer, true);
			else
				show_current_frame(animation); // Show the first frame again, then stop
			return;
		}
	}

	timer_handle = app_timer_send_event(ctx, millis_from_fps, cookie);
}

// This function will animate frames in forward then reverse order. 
// First Frame --> Last Frame, then from Last Frame --> First Frame
void frame_animation_alternating(FrameAnimation *animation, AppContextRef ctx, 
		AppTimerHandle timer_handle,  uint32_t cookie, int fps, bool continuous) {

	animation->isAnimating = true; // set flag to tell if the animation is running

	static int extra; // Either -1 or +1 depending on continuous loop or not
	if (continuous) 
		extra = -1;
	else 
		extra = 1;

	int num_frames = animation->numFrames - 1;

	int millis_from_fps = 1000 / fps; // Get frame delay in milliseconds from desired FPS
	
	show_current_frame(animation);

	// Forwards
	if (animation->frameCounter < num_frames) {
		if (animation->isTransparent)
			animation->resourceCounter = animation->resourceCounter + 2; // Decrement offset by size of image frame, ready for next frame
		else
			animation->resourceCounter++;
		animation->frameCounter++;
	}
	// Backwards
	else if ((animation->frameCounter >= num_frames) && animation->frameCounter < ((num_frames * 2) + extra)) {
		if (animation->isTransparent)
			animation->resourceCounter = animation->resourceCounter - 2; // Increment offset by size of image frame, to cycle back through image
		else
			animation->resourceCounter--;
		animation->frameCounter++;
	}
	else {
		animation->resourceCounter = animation->firstResourceId;
		animation->frameCounter = 0;
		// If not continuous, we need to stop the timer and hide the layer
		if (!continuous || animation->scheduleStop) {
			app_timer_cancel_event(ctx, timer_handle);
			animation->isAnimating = false;
			animation->scheduleStop = false;
			if (animation->startsHidden)
				layer_set_hidden(&animation->animationLayer, true);
			else
				show_current_frame(animation); // Show the first frame again, then stop
			return;
		}
	}

	timer_handle = app_timer_send_event(ctx, millis_from_fps, cookie);
}

// This function will stop the animation that is passed to it
// It will wait until the current animation cycle is complete before stoppings
void frame_animation_stop(FrameAnimation *animation) {
	animation->scheduleStop = true;
}
