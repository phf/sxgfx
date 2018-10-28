/*
 * Classic Amiga "Boxes" demo under X using sdl.
 *
 * If I am doing anything wrong with the API I'd love to hear about it.
 */

#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>

#define TITLE "Boxes Demo (sdl)"
#define WIDTH 320
#define HEIGHT 200

static SDL_Window *window;
static SDL_Renderer *renderer;

static int width = WIDTH;
static int height = HEIGHT;

/*
 * Print message and exit immediately.
 */
static void
panic(const char *msg)
{
	fprintf(stderr, "panic: %s\n", msg);
	exit(EXIT_FAILURE);
}

/*
 * Set up all the nitty-gritty.
 */
static void
setup(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		panic("cannot initialize sdl");
	}

	window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, width, height,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (window == NULL) {
		panic("cannot create window");
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		panic("cannot create renderer");
	}
}

/*
 * Clean up. Easy.
 */
static void
cleanup(void)
{
	if (renderer != NULL) {
		SDL_DestroyRenderer(renderer);
	}
	if (window != NULL) {
		SDL_DestroyWindow(window);
	}
	SDL_Quit();
}

static void
set_black(void)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

static void
set_random(void)
{
	int r = random() % 256;
	int g = random() % 256;
	int b = random() % 256;
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);
}

/*
 * Draw rectangles and handle events until done.
 */
static void
demo(void)
{
	int done = 0;
	set_black();
	SDL_RenderClear(renderer);
	while (!done) {
		/*
		 * Some crazy tearing artifacts here, at least on my ancient
		 * RADEON card. Alas doing VSYNC makes it worse in a way, so
		 * I didn't bother with it.
		 */
		set_random();
		SDL_Rect rect;
		rect.x = random() % width;
		rect.y = random() % height;
		rect.w = random() % (width - rect.x + 1);
		rect.h = random() % (height - rect.y + 1);
		SDL_RenderFillRect(renderer, &rect);
		SDL_RenderPresent(renderer);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
			case SDL_KEYDOWN:
				done = 1;
				break;
			case SDL_WINDOWEVENT:
				if (event.window.event ==
						SDL_WINDOWEVENT_SIZE_CHANGED) {
					width = event.window.data1;
					height = event.window.data2;
				}
				break;
			}
		}
	}
}

int
main(void)
{
	setup();
	demo();
	cleanup();
}
