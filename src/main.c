#include <SDL2/SDL_render.h>
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

void render (void);

int main (int argc, char* argv[]) {

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
	}

	SDL_DestroyTexture(state.texture);
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);

	return 0;
}

void render () {
	for (int x = 0; x < SCREEN_WIDTH; x++) {
		for (int y = 0; y < SCREEN_HEIGHT; y++) {
			state.pixels[(y * SCREEN_WIDTH) + x] = 0xFF12BB;
		}
	}
}
