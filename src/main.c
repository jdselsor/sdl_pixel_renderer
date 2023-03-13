#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#define ASSERT(_e, ...) if(!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1); }

#define SCREEN_WIDTH 384
#define SCREEN_HEIGHT 216

typedef float f32;
typedef double f64;

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;
typedef size_t		usize;
typedef ssize_t		isize;

struct {
	SDL_Window *window;
	SDL_Texture *texture;
	SDL_Renderer *renderer;

	u32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];

	bool quit;
} state;

typedef struct {
	u32 width;
	u32 height;
	u32* pixels;
} FrameBuffer;

FrameBuffer buffers[2];

FrameBuffer* front_buffer;
FrameBuffer* back_buffer;

void create_frame_buffer (FrameBuffer* buffer, u32 width, u32 height);
void clear_buffer (FrameBuffer* buffer);
void write_to_buffer (FrameBuffer* buffer, u32 x, u32 y, u32 color);
void free_buffer (FrameBuffer* buffer);

void swap_buffers (void);

void render (void);

int main (int argc, char* argv[]) {

	// GAME LOOP IMPLMENTATION IDEA
	// f64 previous = getCurrentTime();
	// f64 lag = 0.0;
	// while (true) {
	//     f64 current = getCurrentTime();
	//     f64 delta = current - previous;
	//     previous = current;
	//     lag += delta;
	//
	//     processInputs();
	//     
	//     while (lag >= MS_PER_UPDATE) { // TICK SPEED
	//         update();
	//         lag -= MS_PER_UPDATE;
	//     }
	//
	//     render ();
	// }

	ASSERT(
			!SDL_Init(SDL_INIT_VIDEO), 
			"SDL failed to initialize: %s\n", 
			SDL_GetError()
	);

	state.window = SDL_CreateWindow (
		"DEMO", 
		SDL_WINDOWPOS_CENTERED_DISPLAY(0), 
		SDL_WINDOWPOS_CENTERED_DISPLAY(0),
		1280, 
		720,
		SDL_WINDOW_ALLOW_HIGHDPI
	);

	ASSERT(state.window, "failed to create SDL window: %s\n", SDL_GetError());

	state.renderer = SDL_CreateRenderer(
		state.window, 
		-1, 
		SDL_RENDERER_PRESENTVSYNC
	);

	ASSERT(
		state.renderer, 
		"failed to create SDL renderer: %s\n", 
		SDL_GetError()
	);		
	
	state.texture = SDL_CreateTexture(
		state.renderer, 
		SDL_PIXELFORMAT_ABGR8888, 
		SDL_TEXTUREACCESS_STREAMING, 
		SCREEN_WIDTH, 
		SCREEN_HEIGHT
	);

	ASSERT(state.texture, "failed to create SDL texture: %s\n", SDL_GetError());

	create_frame_buffer(&buffers[0], SCREEN_WIDTH, SCREEN_HEIGHT);
	create_frame_buffer(&buffers[1], SCREEN_WIDTH, SCREEN_HEIGHT);

	front_buffer = &buffers[0];
	back_buffer = &buffers[1];

	while (!state.quit) {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					state.quit = true;
					break;
			}
		}

		memset(state.pixels, 0, sizeof (state.pixels));

		for(int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++) {
			//front_buffer->pixels[i] = 0xFF00FF;
			back_buffer->pixels[i] = 0x00FF00;
		}

		render();

		SDL_UpdateTexture(state.texture, NULL, state.pixels, SCREEN_WIDTH * 4);

		SDL_RenderCopyEx(
				state.renderer, 
				state.texture, 
				NULL, 
				NULL, 
				0.0, 
				NULL, 
				SDL_FLIP_VERTICAL
		);

		SDL_RenderPresent(state.renderer);

		swap_buffers();
	}

	free_buffer(&buffers[0]);
	free_buffer(&buffers[1]);

	SDL_DestroyTexture(state.texture);
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);

	return 0;
}

void create_frame_buffer (FrameBuffer* buffer, u32 width, u32 height) {
	buffer->width = width;
	buffer->height = height;

	buffer->pixels = (u32*) malloc((buffer->width * buffer->height) * sizeof(u32));
	clear_buffer(buffer);
}

void clear_buffer(FrameBuffer* buffer) {
	for (int i = 0; i < buffer->width * buffer->height; i++) {
		buffer->pixels[i] = 0x000000;
	}
}

// implment a bounds check.
void write_to_buffer (FrameBuffer* buffer, u32 x, u32 y, u32 color) {
	buffer->pixels[(y * buffer->width) + x] = color;
}

void free_buffer(FrameBuffer* buffer) {
	free(buffer->pixels);
}

void swap_buffers () {

	FrameBuffer* t = front_buffer;
	front_buffer = back_buffer;
	back_buffer = t;
}

void render () {
	for (int x = 0; x < SCREEN_WIDTH; x++) {
		for (int y = 0; y < SCREEN_HEIGHT; y++) {
			u32 pixel = front_buffer->pixels[(y * front_buffer->width) + x];
			state.pixels[(y * SCREEN_WIDTH) + x] = pixel;
		}
	}
}
