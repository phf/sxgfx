/*
 * Classic Amiga "Boxes" demo under X using, well, just X.
 *
 * If I am doing anything wrong with the API I'd love to hear about it.
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define TITLE "Boxes Demo (x11)"
#define WIDTH 320
#define HEIGHT 200

static Display *display;
static Window window;
static GC context;

static unsigned int width = WIDTH;
static unsigned int height = HEIGHT;

#define WM_DEL "WM_DELETE_WINDOW"

static Atom wm_delete_message;

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
 * Create random 24-bit RGB color.
 *
 * Note that doing this "right" requires XAllocColor() which forces memory
 * allocation. Since we make LOTS of colors it's cheaper to "fake" it. The
 * hack seems to work, but I cannot find documentation that confirms this.
 */
static uint32_t
random_color(void)
{
	uint32_t r = random() % 256;
	uint32_t g = random() % 256;
	uint32_t b = random() % 256;
	return r << 16 | g << 8 | b << 0;
}

/*
 * Draw random rectangle.
 */
static void
draw_random_rectangle(void)
{
	uint32_t color = random_color();
	XSetForeground(display, context, color);

	int x = random() % width;
	int y = random() % height;
	unsigned int w = random() % (width - x + 1);
	unsigned int h = random() % (height - y + 1);

	XFillRectangle(display, window, context, x, y, w, h);
}

/*
 * Event handlers return 1 to stop the demo, 0 to keep running.
 */

static int
handle_key_press(void)
{
	return 1;
}

static int
handle_client_message(XEvent *e)
{
	if (e->xclient.data.l[0] == (int)wm_delete_message) {
		return 1;
	}
	return 0;
}

static int
handle_expose(XEvent *e)
{
	/* We don't care about these but we can't pass NULL below. */
	Window root;
	int x, y;
	unsigned int border, depth;

	if (e->xexpose.count != 0) {
		/* Not the last event in a series so ignore it. */
		return 0;
	}

	Status s = XGetGeometry(display, window, &root, &x, &y, &width, &height,
			&border, &depth);
	if (s == 0) {
		panic("XGetGeometry failed");
	}

	return 0;
}

static int
handle_default(XEvent *e)
{
	printf("log: event %p type %d ignored\n", (void *)e, e->type);
	return 0;
}

/*
 * Handle queued events.
 */
static int
handle_events(void)
{
	int done = 0;
	XEvent event;
	while (XPending(display) > 0) {
		XNextEvent(display, &event);

		switch (event.type) {
		case KeyPress:
			done = handle_key_press();
			break;
		case ClientMessage:
			done = handle_client_message(&event);
			break;
		case Expose:
			done = handle_expose(&event);
			break;
		default:
			done = handle_default(&event);
			break;
		}
	}
	return done;
}

/*
 * Draw rectangles and handle events until done.
 */
static void
demo(void)
{
	int done = 0;
	while (!done) {
		draw_random_rectangle();
		done = handle_events();
	}
}

/*
 * X11 error handlers. I wonder where these are documented?
 */

int
error_handler(Display *d, XErrorEvent *e)
{
	fprintf(stderr, "X11 error: display=%p event=%p\n", (void *)d,
			(void *)e);
	return 0;
}

int
fatal_handler(Display *d)
{
	fprintf(stderr, "X11 fatal: display=%p\n", (void *)d);
	panic("fatal X11 error");
	return 0;
}

/*
 * Set up all the nitty-gritty.
 */
static void
setup(void)
{
	XSetErrorHandler(error_handler);
	XSetIOErrorHandler(fatal_handler);

	display = XOpenDisplay(NULL);
	if (display == NULL) {
		panic("cannot open display");
	}

	int screen = DefaultScreen(display);

	window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0,
			WIDTH, HEIGHT, 5, WhitePixel(display, screen),
			BlackPixel(display, screen));

	wm_delete_message = XInternAtom(display, WM_DEL, False);
	XSetWMProtocols(display, window, &wm_delete_message, 1);

	XSizeHints *sizeHints = XAllocSizeHints();
	if (sizeHints == NULL) {
		panic("cannot allocate sizehints");
	}

	sizeHints->flags = PMinSize;
	sizeHints->min_width = WIDTH;
	sizeHints->min_height = HEIGHT;

	Xutf8SetWMProperties(display, window, TITLE, TITLE, NULL, 0, sizeHints,
			NULL, NULL);
	XFree(sizeHints);

	XSelectInput(display, window, ExposureMask | KeyPressMask);

	context = XCreateGC(display, window, 0, 0);

	XMapRaised(display, window);
}

/*
 * Clean up. Easy.
 */
static void
cleanup(void)
{
	if (display != NULL) {
		XCloseDisplay(display);
	}
}

int
main(void)
{
	setup();
	demo();
	cleanup();
}
