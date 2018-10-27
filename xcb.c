/*
 * Classic Amiga "Boxes" demo under X using xcb.
 *
 * If I am doing anything wrong with the API I'd love to hear about it.
 */

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TITLE "Boxes Demo (xcb)"
#define WIDTH 320
#define HEIGHT 200

static xcb_connection_t *conn;
static xcb_window_t win;
static xcb_gcontext_t gc;

static uint16_t width = WIDTH;
static uint16_t height = HEIGHT;

#define WM_PRO "WM_PROTOCOLS"
#define WM_DEL "WM_DELETE_WINDOW"

static xcb_atom_t protocols_atom;
static xcb_atom_t delete_window_atom;

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
 * Panic if given cookie doesn't check out.
 */
static void
ensure(xcb_void_cookie_t cookie, const char *msg)
{
	xcb_generic_error_t *error;
	if ((error = xcb_request_check(conn, cookie))) {
		free(error);
		panic(msg);
	}
}

/*
 * Create random 24-bit RGB color.
 *
 * Note that doing this "right" requires xcb_alloc_color() which forces memory
 * allocation. Since we make LOTS of colors it's cheaper to "fake" it. Besides
 * the xcb_alloc_color(3) man page encourages this for "TrueColor" displays.
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
	uint32_t values[1];
	xcb_void_cookie_t cookie;
	xcb_rectangle_t rect;

	values[0] = random_color();
	cookie = xcb_change_gc_checked(conn, gc, XCB_GC_FOREGROUND, values);
	ensure(cookie, "cannot change graphics context");

	rect.x = random() % width;
	rect.y = random() % height;
	rect.width = random() % (width - rect.x + 1);
	rect.height = random() % (height - rect.y + 1);
	cookie = xcb_poly_fill_rectangle_checked(conn, win, gc, 1, &rect);
	ensure(cookie, "cannot draw rectangle");

	int s = xcb_flush(conn); /* TODO why flush here? */
	if (s <= 0) {
		panic("failed to flush");
	}
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
handle_client_message(xcb_generic_event_t *e)
{
	xcb_client_message_event_t *client = (xcb_client_message_event_t *)e;
	if (client->type == protocols_atom && client->format == 32 &&
			client->data.data32[0] == delete_window_atom) {
		return 1;
	}
	return 0;
}

static int
handle_expose(void)
{
	/*
	 * I couldn't find a better way of doing this, there doesn't appear to
	 * be a simple "tell me the size changed" event that actually works.
	 */
	xcb_get_geometry_reply_t *geo = xcb_get_geometry_reply(
			conn, xcb_get_geometry(conn, win), NULL);
	if (geo) {
		width = geo->width;
		height = geo->height;
	}
	free(geo);
	return 0;
}

static int
handle_default(xcb_generic_event_t *e)
{
	printf("log: event %p type %d ignored\n", (void *)e, e->response_type);
	return 0;
}

/*
 * Handle queued events.
 */
static int
handle_events(void)
{
	int done = 0;
	xcb_generic_event_t *e;
	while ((e = xcb_poll_for_event(conn))) {
		switch (e->response_type & 0x7f) {
		case XCB_KEY_PRESS:
			done = handle_key_press();
			break;
		case XCB_CLIENT_MESSAGE:
			done = handle_client_message(e);
			break;
		case XCB_EXPOSE:
			done = handle_expose();
			break;
		default:
			done = handle_default(e);
			break;
		}
		free(e);
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
 * Set up all the nitty-gritty.
 */
static void
setup(void)
{
	int screen_number;
	const xcb_setup_t *setup;
	const xcb_screen_t *scr;
	uint32_t mask;
	uint32_t values[2];
	xcb_void_cookie_t cookie;

	/* open connection to server, always returns != NULL */
	conn = xcb_connect(NULL, &screen_number);
	if (xcb_connection_has_error(conn)) {
		panic("cannot connect to server");
	}

	/* should not go wrong, accessor for connection data */
	setup = xcb_get_setup(conn);

	/* find screen */
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
	for (int i = 0; i < screen_number && iter.rem; i++) {
		xcb_screen_next(&iter);
	}
	if (iter.rem == 0) {
		panic("cannot find screen");
	}
	scr = iter.data;

	/* create window */
	win = xcb_generate_id(conn);
	mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	values[0] = scr->black_pixel;
	values[1] = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_EXPOSURE;

	cookie = xcb_create_window_checked(conn, XCB_COPY_FROM_PARENT, win,
			scr->root, 0, 0, WIDTH, HEIGHT, 1,
			XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT,
			mask, values);
	ensure(cookie, "cannot create window");

	/* create graphics context */
	gc = xcb_generate_id(conn);
	cookie = xcb_create_gc_checked(conn, gc, win, 0, NULL);
	ensure(cookie, "cannot create graphics context");

	/* set minimum size hints */
	xcb_size_hints_t hints = {0}; // C99 6.7.8.21
	xcb_icccm_size_hints_set_min_size(&hints, WIDTH, HEIGHT);

	cookie = xcb_icccm_set_wm_size_hints_checked(
			conn, win, XCB_ATOM_WM_NORMAL_HINTS, &hints);
	ensure(cookie, "cannot set size hints");

	/* set window title */
	cookie = xcb_change_property_checked(conn, XCB_PROP_MODE_REPLACE, win,
			XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(TITLE),
			TITLE);
	ensure(cookie, "cannot change window title");

	cookie = xcb_change_property_checked(conn, XCB_PROP_MODE_REPLACE, win,
			XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8,
			strlen(TITLE), TITLE);
	ensure(cookie, "cannot change icon title");

	/* set up an event if the window is closed */
	xcb_intern_atom_reply_t *pro = xcb_intern_atom_reply(conn,
			xcb_intern_atom(conn, 1, strlen(WM_PRO), WM_PRO), NULL);
	xcb_intern_atom_reply_t *del = xcb_intern_atom_reply(conn,
			xcb_intern_atom(conn, 0, strlen(WM_DEL), WM_DEL), NULL);
	if (pro == NULL || del == NULL) {
		panic("cannot atomize window close");
	}

	protocols_atom = pro->atom;
	delete_window_atom = del->atom;

	free(pro);
	free(del);

	cookie = xcb_change_property_checked(conn, XCB_PROP_MODE_REPLACE, win,
			protocols_atom, XCB_ATOM_ATOM, 32, 1,
			&delete_window_atom);
	ensure(cookie, "cannot change window close");

	/* map the window */
	cookie = xcb_map_window_checked(conn, win);
	ensure(cookie, "cannot map window");

	/* flush everything we did, just in case */
	int s = xcb_flush(conn);
	if (s <= 0) {
		panic("failed to flush");
	}
}

/*
 * Clean up. Easy.
 */
static void
cleanup(void)
{
	/* close connection to server, conn == NULL is no-op */
	xcb_disconnect(conn);
}

int
main(void)
{
	setup();
	demo();
	cleanup();
}
