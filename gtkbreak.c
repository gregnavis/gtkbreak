#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <glib.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

/* Suppresses warnings about unused parameters. */
#define unused(x) ((void) (x))

struct break_info {
	int work_sec;
	int break_sec;
};

static GtkWidget *window, *alignment, *timer;
static GdkScreen *screen;

/* the number of breaks */
static int nbreaks;

/* the breaks */
static struct break_info *breaks;

/* the current break */
static int break_idx = 0;

/* remaining time */
static int sec = 0;

static gboolean break_tick(gpointer);

static void update_timer(void)
{
	struct break_info *breakp = &breaks[break_idx];
	char *label, buf[256];

	switch (sec) {
	case 0:
		label = "Press any key to get back to work!";
		break;

	case 1:
		label = "1 second remaining";
		break;

	default:
		snprintf(buf, sizeof(buf), "%d seconds remaining", sec);
		label = buf;
	}


	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(timer),
		1.0 - (gdouble) sec / breakp->break_sec);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(timer),
		label);
}

static gboolean start_break(gpointer data)
{
	(void)data;

	sec = breaks[break_idx].break_sec;
	gtk_widget_show_all(window);
	update_timer();

	g_timeout_add_seconds(1, break_tick, NULL);

	return FALSE;
}

static gboolean break_tick(gpointer data)
{
	(void)data;

	sec--;
	update_timer();
	if (sec == 0) {
		return FALSE;
	}

	return TRUE;
}

static gboolean handle_input(GtkWidget *widget, GdkEvent *event, gpointer *data)
{
	unused(widget);
	unused(event);
	unused(data);

	if (sec == 0) {
		gtk_widget_hide(window);

		if(++break_idx == nbreaks) {
			break_idx = 0;
		}

		g_timeout_add_seconds(breaks[break_idx].work_sec, start_break, NULL);

		return FALSE;
	}

	gdk_keyboard_grab(window->window, TRUE, GDK_CURRENT_TIME);
	return TRUE;
}

int main(int argc, char *argv[])
{
	int res, i;

	if(argc <= 1) {
		printf("Usage: %s [work1 break1] [work2 break2] ...\n", argv[0]);
		res = EXIT_SUCCESS;
		goto out;
	}

	if(argc % 2 != 1) {
		fprintf(stderr, "Error: no break time given\n");
		res = EXIT_FAILURE;
		goto out;
	}

	nbreaks = (argc - 1) / 2;

	breaks = malloc(sizeof(*breaks) * nbreaks);
	if(!breaks) {
		perror("malloc");
		res = EXIT_FAILURE;
		goto out;
	}

	for(i = 0; i < nbreaks; i++) {
		breaks[i].work_sec = atoi(argv[2 * i + 1]);
		breaks[i].break_sec = atoi(argv[2 * i + 2]);

		if(breaks[i].work_sec <= 0 || breaks[i].break_sec <= 0) {
			fprintf(stderr, "Error: time cannot be negative\n");
			res = EXIT_FAILURE;
			goto free_break_info;
		}
	}

	gtk_set_locale();
	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_window_fullscreen(GTK_WINDOW(window));
	screen = gtk_window_get_screen(GTK_WINDOW(window));
	gtk_window_set_default_size(GTK_WINDOW(window),
		gdk_screen_get_width(screen),
		gdk_screen_get_height(screen));
	g_signal_connect(window,
		"motion_notify_event",
		G_CALLBACK(handle_input),
		NULL);
	g_signal_connect(window,
		"key_press_event",
		G_CALLBACK(handle_input),
		NULL);
	g_signal_connect(window,
		"button_press_event",
		G_CALLBACK(handle_input),
		NULL);

	alignment = gtk_alignment_new(0.5f, 0.5f, 0.75f, 0.0f);
	timer = gtk_progress_bar_new();
	gtk_container_add(GTK_CONTAINER(alignment), timer);
	gtk_container_add(GTK_CONTAINER(window), alignment);

	g_timeout_add_seconds(breaks[0].work_sec, start_break, NULL);

	gtk_main();

	res = EXIT_SUCCESS;

free_break_info:
	free(breaks);

out:
	return res;
}
