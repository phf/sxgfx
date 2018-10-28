/*
 * Classic Amiga "Boxes" demo under X using gtk.
 *
 * If I am doing anything wrong with the API I'd love to hear about it.
 */

#include <gtk/gtk.h>

#include <stdlib.h>

#define TITLE "Boxes Demo (gtk)"
#define WIDTH 320
#define HEIGHT 200

/* Need surface/context since window's surface is cleared by GTK. */
static cairo_surface_t *surface;
static cairo_t *context;

/* Need ids to remove tick and idle callbacks before quitting. */
static guint tick_id;
static guint idle_id;

/* Current size of window (and surface). */
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
 * Set Cairo source to random 24-bit RGB color.
 */
static void
set_random_rgb(cairo_t *cr)
{
	double r = (random() % 256) / 255.0;
	double g = (random() % 256) / 255.0;
	double b = (random() % 256) / 255.0;
	cairo_set_source_rgb(cr, r, g, b);
}

/*
 * Create new surface/context if possible/necessary.
 */
static void
update_size(GtkWidget *widget, gint w, gint h)
{
	GdkWindow *window = gtk_widget_get_window(widget);
	if (!window) {
		/* no GdkWindow yet (we're "not yet realized" as it were) */
		return;
	}

	if (width == w && height == h && surface && context) {
		/* size didn't change and we have surface/context already */
		return;
	}

	if (context) {
		cairo_destroy(context);
		context = NULL;
	}
	if (surface) {
		cairo_surface_destroy(surface);
		surface = NULL;
	}

	/* documentation says this always returns a valid pointer */
	surface = gdk_window_create_similar_surface(
			window, CAIRO_CONTENT_COLOR, w, h);
	/* documentation says this never returns NULL */
	context = cairo_create(surface);

	width = w;
	height = h;
}

/*
 * Triggered when window is destroyed. Cleans up idle/tick callbacks (which can
 * cause a "hang" otherwise) as well as surface/context.
 */
static void
destroy_cb(GtkWidget *widget, gpointer data)
{
	if (idle_id > 0) {
		g_source_remove(idle_id); /* always returns true */
	}
	if (tick_id > 0) {
		g_source_remove(tick_id); /* always returns true */
	}

	if (context) {
		cairo_destroy(context);
		context = NULL;
	}
	if (surface) {
		cairo_surface_destroy(surface);
		surface = NULL;
	}
}

/*
 * Triggered when window needs redrawing. Copies off-screen surface to window.
 */
static gboolean
draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	if (!surface) {
		return FALSE; /* propagate further */
	}
	cairo_set_source_surface(cr, surface, 0, 0);
	cairo_paint(cr);
	return TRUE; /* don't propagate further */
}

/*
 * Triggered when key is pressed. Forces window to be destroyed, ending the
 * application.
 */
static gboolean
key_press_event_cb(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	gtk_widget_destroy(widget);
	return TRUE; /* don't propagate further */
}

/*
 * Triggered when size changes. Adjusts our surface and context to match new
 * conditions.
 *
 * Handling "size-allocate" is cleaner than handling both "configure-event"
 * (for resize) and window-state-event (for maximize). However, we can get a
 * "size-allocate" *before* the window is realized!
 *
 * Note that GtkAllocation is a typedef for GdkRectangle.
 */
static void
size_allocate_cb(GtkWidget *widget, GtkAllocation *allocation, void *data)
{
	/*
	 * Avoid allocation parameter, see
	 * https://developer.gnome.org/gtk3/stable/GtkWindow.html#gtk-window-get-size
	 */
	gint w, h;
	gtk_window_get_size(GTK_WINDOW(widget), &w, &h);
	update_size(widget, w, h);
}

/*
 * Triggered regularly, possibly on vsync. Invalidates window (if it exists) to
 * force draw signal.
 */
static gboolean
tick_cb(GtkWidget *widget, GdkFrameClock *clock, gpointer data)
{
	GdkWindow *window = gtk_widget_get_window(widget);
	if (window) {
		gdk_window_invalidate_rect(window, NULL, TRUE);
	}
	return G_SOURCE_CONTINUE;
}

/*
 * Triggered regularly, as often as possible. Draws random rectangles into
 * off-screen surface (if it exists).
 */
static gboolean
idle_cb(gpointer data)
{
	if (surface && context) {
		int x, y, w, h;
		set_random_rgb(context);
		x = random() % width;
		y = random() % height;
		w = random() % (width - x + 1);
		h = random() % (height - y + 1);
		cairo_rectangle(context, x, y, w, h);
		cairo_fill(context);
	}
	return G_SOURCE_CONTINUE;
}

/*
 * Set up all the nitty-gritty.
 */
static void
activate_cb(GtkApplication *app, gpointer user_data)
{
	int ok;

	GtkWidget *window = gtk_application_window_new(app);
	if (window == NULL) {
		panic("cannot create window");
	}

	gtk_window_set_title(GTK_WINDOW(window), TITLE);
	gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT);
	gtk_widget_set_size_request(window, WIDTH, HEIGHT); /* minimum size */
	/* TODO gtk_window_set_geometry_hints() might be better? */

	ok = g_signal_connect(window, "destroy", G_CALLBACK(destroy_cb), NULL);
	if (ok <= 0) {
		panic("cannot connect destroy signal");
	}
	ok = g_signal_connect(window, "draw", G_CALLBACK(draw_cb), NULL);
	if (ok <= 0) {
		panic("cannot connect draw signal");
	}
	ok = g_signal_connect(window, "key-press-event",
			G_CALLBACK(key_press_event_cb), NULL);
	if (ok <= 0) {
		panic("cannot connect key-press-event signal");
	}
	ok = g_signal_connect(window, "size-allocate",
			G_CALLBACK(size_allocate_cb), NULL);
	if (ok <= 0) {
		panic("cannot connect size-allocate signal");
	}

	tick_id = gtk_widget_add_tick_callback(window, tick_cb, NULL, NULL);
	if (tick_id <= 0) { /* error return not officially documented */
		panic("cannot add tick callback");
	}
	idle_id = g_idle_add(idle_cb, NULL);
	if (idle_id <= 0) {
		panic("cannot add idle callback");
	}

	gtk_widget_show_all(window);
}

int
main(int argc, char *argv[])
{
	GtkApplication *app = gtk_application_new(
			"com.github.phf.Boxes", G_APPLICATION_FLAGS_NONE);
	if (app == NULL) {
		panic("cannot create application");
	}

	int ok = g_signal_connect(
			app, "activate", G_CALLBACK(activate_cb), NULL);
	if (ok <= 0) {
		panic("cannot connect activate signal");
	}

	int status = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);

	return status;
}
