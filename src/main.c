#include <SDL2/SDL_error.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <string.h>

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

typedef uint16_t	WORD;
typedef uint32_t	DWORD;
typedef uint64_t	QWORD;

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

typedef struct {
	u32 width;
	u32 height;
	u32* pixels;
} Image;

void load_image (Image* image, const char* filename);

#define BLACK		0x000000
#define WHITE		0xFFFFFF
#define RED			0x880000
#define CYAN		0xAAFFEE
#define PURPLE		0xCC44CC
#define GREEN		0x00CC55
#define BLUE		0x0000AA
#define YELLOW		0xEEEE77
#define ORANGE		0xDD8855
#define BROWN		0x664400
#define LIGHT_RED	0xFF7777
#define DARK_GREY	0x333333
#define GREY		0x777777
#define LIGHT_GREEN	0xAAFF66
#define LIGHT_BLUE	0x0088FF
#define LIGHT_GREY	0xBBBBBB

#define ALPHA_COLOR 0xFFFF00FF

void render (void);

void fill_rect (usize x, usize y, usize width, usize height, u32 color);

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

	Image image;
	load_image(&image, "./res/font.bmp");

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

		//fill_rect(10, 10, 100, 100, WHITE);
		/*for (int i = 0; i < (image.width * image.height); i++) {
			//printf("%X\n", image.pixels[i]);
			back_buffer->pixels[i] = image.pixels[i];
		}*/

		for (usize x = 0; x < image.width; x++) {
			for (usize y = 0; y < image.height; y++) {
				if (image.pixels[image.width * y + x] == ALPHA_COLOR) {
					printf("Hit alpha color\n");
					//back_buffer->pixels[back_buffer->width * y + x] = 0xFF00FF00;
					continue;
				}
				back_buffer->pixels[back_buffer->width * y + x] = image.pixels[image.width * y + x];
			}
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

		swap_buffers();

		SDL_RenderPresent(state.renderer);
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
	
	//clear_buffer(front_buffer);
	clear_buffer(back_buffer);
}

// BMP file format
//
// Section
//		Address: Bytes		Name
//
// Header
//		0:		2		"BM" Magic number
//		2:		4		file size
//		6:		4		junk
//	   10:		4		Starting address of image data
// BITMAP HEADER
//		14:		4		Header Size
//		18:		4		width(signed)
//		22:		4		height(signed)
//		26:		2		Number of color planes
//		28:		2		Bits Per pixel
//		[...]
//	[Optional Color palette, Not present in 32 bit bitmaps]
//
//	Data: X Pixels
void load_image (Image* image, const char* filename) {
	u32 image_data_address;
	i32 width, height;
	u32 pixel_count;
	u16 bit_depth;
	u8 byte_depth;

	u32* pixels;

	printf("Loading bitmap file: %s\n", filename);

	FILE *file;
	file = fopen(filename, "rb");

	if (file) {
		if(fgetc(file) == 'B' && fgetc(file) == 'M') {
			printf("BM read; bitmap file confirmed.\n");
			fseek(file, 8, SEEK_CUR);
			fread(&image_data_address, 4, 1, file);
			fseek(file, 4, SEEK_CUR);
			fread(&width, 4, 1, file);
			fread(&height, 4, 1, file);
			fseek(file, 2, SEEK_CUR);
			fread(&bit_depth, 2, 1, file);

			ASSERT(bit_depth == 32, "(%s) Bit Depth expected %d is %d", filename, 32, bit_depth);

			printf("image data address:\t%d\nwidth:\t\t\t%d pix\nheight:\t\t\t%d pix\nbit depth:\t\t%d bpp\n", image_data_address, width, height, bit_depth);

			pixel_count = width * height;
			byte_depth = bit_depth / 8;
			pixels = malloc(pixel_count * byte_depth);

			if (pixels) {
				fseek(file, image_data_address, SEEK_SET);
				int pixels_read = fread(pixels, byte_depth, pixel_count, file);
				printf("Read %d pixels\n", pixels_read);

				if (pixels_read == pixel_count) {
					image->width = width;
					image->height = height;
					image->pixels = pixels;
				} else {
					printf("(%s) Read pixel count incorrect. Is %d expected %d", filename, pixels_read, pixel_count);
					free(pixels);
				}
			} else {
				printf("(%s) Failed to allocated %d pixels.\n", filename, pixel_count);
			}
		} else {
			printf("(%s) frist two bytes of file are not \"BM\"", filename);
		}

		fclose(file);
	}
}

void render () {
	for (usize x = 0; x < SCREEN_WIDTH; x++) {
		for (usize y = 0; y < SCREEN_HEIGHT; y++) {
			u32 pixel = front_buffer->pixels[(y * front_buffer->width) + x];
			state.pixels[(y * SCREEN_WIDTH) + x] = pixel;
		}
	}
}

void fill_rect (usize x, usize y, usize width, usize height, u32 color) {
	for (usize i = 0; i < width; i++) {
		for (usize j = 0; j < height; j++) {
			//back_buffer->pixels[((y + j) * width) + (x + i)] = color;
			back_buffer->pixels[(back_buffer->width * (y + j)) + (x + i)] = color;
		}
	}
}
