#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <glib.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

/* Suppresses warnings about unused parameters. */
#define unused(x) ((void) (x))

/* Available modes. */
enum mode {
	MODE_WORK,
	MODE_BREAK
};

/* The duration of additional dealy that serves as a penalty for pressing
 * keys during a break. */
static const int default_penalty_seconds = 3;

/* A single work-break cycle. */
struct cycle {
	/* The duration of work in seconds. */
	int work_duration;

	/* The duration of a break in seconds. */
	int break_duration;
};

/* The break window that contains the timer. */
static GtkWidget *window;

/* The alignment used to position the timer. */
static GtkWidget *alignment;

/* The timer. */
static GtkWidget *timer;

/* The current screen. */
static GdkScreen *screen;

/* The number of cycles. */
static int cycles_count;

/* The cycles. */
static struct cycle *cycles;

/* The index of the current cycle. */
static int current_cycle_index = 0;

/* Remaining time of work/break in seconds. */
static int remaining_seconds = 0;

/* Remaining time of penalty for pressing keys during a break. */
static int penalty_seconds = 0;

/* The current mode. */
static enum mode mode = MODE_WORK;

static gboolean tick(gpointer);

/* Update the timer in the break window. */
static void update_timer(void)
{
	struct cycle *cyclep = &cycles[current_cycle_index];
	char *label, buf[256];

	if (penalty_seconds) {
		label = "You aren't supposed to be working now! Take a break!";
	} else {
		switch (remaining_seconds) {
		case 0:
			label = "Press any key to get back to work!";
			break;

		case 1:
			label = "1 second remaining";
			break;

		default:
			snprintf(buf, sizeof(buf), "%d seconds remaining", remaining_seconds);
			label = buf;
		}
	}


	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(timer),
		1.0 - (gdouble) remaining_seconds / cyclep->break_duration);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(timer), label);
}

/* Start work. */
static void start_work(void)
{
	mode = MODE_WORK;
	remaining_seconds = cycles[current_cycle_index].work_duration;
	gtk_widget_hide(window);
}

/* Start a break. */
static void start_break(void)
{
	mode = MODE_BREAK;
	remaining_seconds = cycles[current_cycle_index].break_duration;
	penalty_seconds = 0;
	update_timer();
	gtk_widget_show_all(window);
	gdk_keyboard_grab(window->window, TRUE, GDK_CURRENT_TIME);
}

/* Go to the next cycle. */
static void next_cycle(void)
{
	current_cycle_index = (current_cycle_index + 1) % cycles_count;
}


/* Step the timer during work or a break. */
static gboolean tick(gpointer data)
{
	unused(data);

	switch (mode) {
	case MODE_WORK:
		if (remaining_seconds) {
			remaining_seconds--;
		} else {
			start_break();
		}
		break;

	case MODE_BREAK:
		if (penalty_seconds) {
			penalty_seconds--;
		} else if (remaining_seconds) {
			remaining_seconds--;
		}

		update_timer();
		break;
	}

	return TRUE;
}

/* Handle input during a break.
 *
 * When the break is finished then close the window. Otherwise do nothing.
 */
static gboolean handle_input(GtkWidget *widget, GdkEvent *event, gpointer *data)
{
	unused(widget);
	unused(event);
	unused(data);

	if (remaining_seconds == 0) {
		next_cycle();
		start_work();
		return FALSE;
	} else {
		penalty_seconds = default_penalty_seconds;
		update_timer();
	}

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

	cycles_count = (argc - 1) / 2;

	cycles = malloc(sizeof(*cycles) * cycles_count);
	if(!cycles) {
		perror("malloc");
		res = EXIT_FAILURE;
		goto out;
	}

	for(i = 0; i < cycles_count; i++) {
		cycles[i].work_duration = atoi(argv[2 * i + 1]);
		cycles[i].break_duration = atoi(argv[2 * i + 2]);

		if(cycles[i].work_duration <= 0 || cycles[i].break_duration <= 0) {
			fprintf(stderr, "Error: time cannot be negative\n");
			res = EXIT_FAILURE;
			goto free_cycle;
		}
	}

	gtk_set_locale();
	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_window_fullscreen(GTK_WINDOW(window));
	screen = gtk_window_get_screen(GTK_WINDOW(window));
	gtk_window_set_default_size(GTK_WINDOW(window),
		gdk_screen_get_width(screen),
		gdk_screen_get_height(screen));
	gtk_window_resize(GTK_WINDOW(window),
		gdk_screen_get_width(screen),
		gdk_screen_get_height(screen));
	gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
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

	g_timeout_add_seconds(1, tick, NULL);

	gtk_main();

	res = EXIT_SUCCESS;

free_cycle:
	free(cycles);

out:
	return res;
}
