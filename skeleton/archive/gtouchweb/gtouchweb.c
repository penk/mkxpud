/*
 * gcc `pkg-config --cflags --libs webkit-1.0` -o gtouchweb main.c
 */

#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include "midori-scrolledwindow.h"

#define DEFAULT_INTERVAL 50
#define DEFAULT_DECELERATION 0.7
#define DEFAULT_DRAGGING_STOPPED_DELAY 100

static GtkWidget* main_window;
static GtkWidget* uri_entry;
static WebKitWebView* web_view;
static gchar* main_title;
static gint load_progress;

static void
activate_uri_entry_cb (GtkWidget* entry, gpointer data)
{
    const gchar* uri = gtk_entry_get_text (GTK_ENTRY (entry));
    g_assert (uri);

	gchar *http_url;
	if (!strstr (uri, "://")) {
		http_url = g_strdup_printf ("http://%s", uri);
	} else {
		http_url = g_strdup (uri);
	}
    webkit_web_view_open (web_view, http_url);
}

static void
update_title (GtkWindow* window)
{
    GString* string = g_string_new (main_title);
//    g_string_append (string, " - TouchWeb Browser");
    if (load_progress < 100)
        g_string_append_printf (string, " (%d%%)", load_progress);
    gchar* title = g_string_free (string, FALSE);
    gtk_window_set_title (window, title);
    g_free (title);
}

static void
title_change_cb (WebKitWebView* web_view, WebKitWebFrame* web_frame, const gchar* title, gpointer data)
{
    if (main_title)
        g_free (main_title);
    main_title = g_strdup (title);
    update_title (GTK_WINDOW (main_window));
}

static void
progress_change_cb (WebKitWebView* page, gint progress, gpointer data)
{
    load_progress = progress;
    update_title (GTK_WINDOW (main_window));
}

static void
load_commit_cb (WebKitWebView* page, WebKitWebFrame* frame, gpointer data)
{
    const gchar* uri = webkit_web_frame_get_uri(frame);
    if (uri)
        gtk_entry_set_text (GTK_ENTRY (uri_entry), uri);
}

static void
destroy_cb (GtkWidget* widget, gpointer data)
{
    gtk_main_quit ();
}

static void
go_back_cb (GtkWidget* widget, gpointer data)
{
    webkit_web_view_go_back (web_view);
}

static void
go_forward_cb (GtkWidget* widget, gpointer data)
{
    webkit_web_view_go_forward (web_view);
}

static void
reload_cb (GtkWidget* widget, gpointer data)
{
    webkit_web_view_reload (web_view);
}

gboolean 
decidedownload(WebKitWebView *v, WebKitWebFrame *f, WebKitNetworkRequest *r, gchar *m,  WebKitWebPolicyDecision *p) {
	if(!webkit_web_view_can_show_mime_type(v, m)) {
		webkit_web_policy_decision_download(p);
		return TRUE;
	}
	return FALSE;
}

gboolean 
initdownload(WebKitWebView *view, WebKitDownload *o) {
	const char *filename;
	char *uri, *html;

	filename = webkit_download_get_suggested_filename(o);
	if(!strcmp("", filename))
		filename = "index.html";
	uri = g_strconcat("file:///tmp/", filename, NULL);
	webkit_download_set_destination_uri(o, uri);
	g_free(uri);

	webkit_download_get_uri(o);
	webkit_download_start(o);
	
	return TRUE;
}

static GtkWidget*
create_browser ()
{
    //GtkWidget* scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	GtkWidget* scrolled_window = midori_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    web_view = WEBKIT_WEB_VIEW (webkit_web_view_new ());
    gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (web_view));

    g_signal_connect (G_OBJECT (web_view), "title-changed", G_CALLBACK (title_change_cb), web_view);
    g_signal_connect (G_OBJECT (web_view), "load-progress-changed", G_CALLBACK (progress_change_cb), web_view);
    g_signal_connect (G_OBJECT (web_view), "load-committed", G_CALLBACK (load_commit_cb), web_view);

	g_signal_connect(G_OBJECT(WEBKIT_WEB_VIEW (web_view)), "mime-type-policy-decision-requested", G_CALLBACK(decidedownload), NULL);
	g_signal_connect(G_OBJECT(WEBKIT_WEB_VIEW (web_view)), "download-requested", G_CALLBACK(initdownload), NULL);

    return scrolled_window;
}

static GtkWidget*
create_toolbar ()
{
    GtkWidget* toolbar = gtk_toolbar_new ();

    gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar), GTK_ORIENTATION_HORIZONTAL);
    gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH_HORIZ);

    GtkToolItem* item;

    /* the back button */
    item = gtk_tool_button_new_from_stock (GTK_STOCK_GO_BACK);
    g_signal_connect (G_OBJECT (item), "clicked", G_CALLBACK (go_back_cb), NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    /* The forward button */
    item = gtk_tool_button_new_from_stock (GTK_STOCK_GO_FORWARD);
    g_signal_connect (G_OBJECT (item), "clicked", G_CALLBACK (go_forward_cb), NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    /* The URL entry */
    item = gtk_tool_item_new ();
    gtk_tool_item_set_expand (item, TRUE);
    uri_entry = gtk_entry_new ();
    gtk_container_add (GTK_CONTAINER (item), uri_entry);
    g_signal_connect (G_OBJECT (uri_entry), "activate", G_CALLBACK (activate_uri_entry_cb), NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    /* The go button */
    item = gtk_tool_button_new_from_stock (GTK_STOCK_REFRESH);
    g_signal_connect_swapped (G_OBJECT (item), "clicked", G_CALLBACK (reload_cb), NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    return toolbar;
}

static GtkWidget*
create_window ()
{
    GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
    gtk_widget_set_name (window, "TouchWeb");
    g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (destroy_cb), NULL);

    return window;
}

int
main (int argc, char* argv[])
{
    gtk_init (&argc, &argv);

    GtkWidget* vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), create_toolbar (), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), create_browser (), TRUE, TRUE, 0);

    main_window = create_window ();
    gtk_container_add (GTK_CONTAINER (main_window), vbox);

    gchar* uri = (gchar*) (argc > 1 ? argv[1] : "http://www.google.com/ncr");
    webkit_web_view_open (web_view, uri);

    gtk_widget_grab_focus (GTK_WIDGET (web_view));
    gtk_widget_show_all (main_window);
    gtk_main ();

    return 0;
}

/**
 * SECTION:midori-scrolledwindow
 * @short_description: Implements drag scrolling and kinetic scrolling
 * @see_also: #GtkScrolledWindow
 *
 * A scrolled window derived from #GtkScrolledWindow that implements drag scrolling and kinetic scrolling.
 * Can be used as a drop-in replacement for the existing #GtkScrolledWindow. 
 *
 * If a direct child of the #MidoriScrolledWindow has its own window (InputOnly is enough for events),
 * it is automatically activated when added as a child. All motion events in that area will be used to
 * scroll.
 *
 * If some descendant widgets capture button press, button release and/or motion nofity events, an user
 * can not scroll the area by pressing those widgets (unless the widget is activated). #GtkButton is a typical
 * example of that. Usually that is the desired behaviour.
 *
 * Any widget can be registered to provide pointer events for the #MidoriScrolledWindow by using
 * the #midori_scrolled_window_activate_scrolling function.
 * 
 **/

typedef struct _MidoriEventHandlerState MidoriEventHandlerState;
typedef gboolean (*MidoriEventHandler)(GdkEvent* event, MidoriEventHandlerState* state, gpointer user_data);

typedef struct {
	MidoriEventHandler event_handler;
	gpointer user_data;
} EventHandlerData;

struct _MidoriEventHandlerState {
	GList* current_event_handler;
};

static GList* event_handlers = NULL;

static void event_handler_func(GdkEvent* event, gpointer data);

static void midori_event_handler_append(MidoriEventHandler event_handler, gpointer user_data) {
	EventHandlerData* data;

	data = g_new0(EventHandlerData, 1);
	data->event_handler = event_handler;
	data->user_data = user_data;
	event_handlers = g_list_append(event_handlers, data);

	gdk_event_handler_set((GdkEventFunc)event_handler_func, NULL, NULL);
}

static void midori_event_handler_next(GdkEvent* event, MidoriEventHandlerState* state) {
	EventHandlerData* data;
	gboolean stop_propagating;
	
	state->current_event_handler = g_list_next(state->current_event_handler);
	if (state->current_event_handler) {
		data = (EventHandlerData*)state->current_event_handler->data;
		stop_propagating = data->event_handler(event, state, data->user_data);
		if (!stop_propagating && state->current_event_handler) {
			g_critical("midori_event_handler: handler returned false without calling event_handler_next first");		
		}
	} else {
		gtk_main_do_event(event);
	}
}

static void event_handler_func(GdkEvent* event, gpointer user_data) {
	MidoriEventHandlerState* state;
	EventHandlerData* data;
	gboolean stop_propagating;

	state = g_new0(MidoriEventHandlerState, 1);
	state->current_event_handler = g_list_first(event_handlers);
	if (state->current_event_handler) {
		data = (EventHandlerData*)state->current_event_handler->data;
		stop_propagating = data->event_handler(event, state, data->user_data);
		if (!stop_propagating && state->current_event_handler) {
			g_critical("midori_event_handler: handler returned false without calling event_handler_next first");		
		}
	} else {
		gtk_main_do_event(event);
	}

	g_free(state);
}

static GdkWindow* current_gdk_window;
static MidoriScrolledWindow* current_scrolled_window;
static GtkWidget* current_widget;
static gboolean synthetized_crossing_event;

static GTree* activated_widgets;

struct _MidoriScrolledWindowPrivate {
	/* Settings */
	guint interval;
	gdouble deceleration;
	gboolean drag_scrolling;
	gboolean kinetic_scrolling;
	guint32 dragging_stopped_delay;
	gboolean scrolling_hints;
	
	/* Temporary variables */
	gboolean dragged;
	gboolean press_received;
	GdkWindow* synthetic_crossing_event_window;

	/* Disabling twice happening scrolling adjustment */
	GtkAdjustment* hadjustment;
	GtkWidget* viewport;

	/* Motion scrolling */
	gint start_x;
	gint start_y;
	gint previous_x;
	gint previous_y;
	gint farest_x;
	gint farest_y;
	guint32 start_time;
	guint32 previous_time;
	guint32 farest_time;
	gboolean going_right;
	gboolean going_down;
	
	/* Kinetic scrolling */
	guint scrolling_timeout_id;
	gdouble horizontal_speed;
	gdouble vertical_speed;
	gdouble horizontal_deceleration;
	gdouble vertical_deceleration;
	
	/* Internal scrollbars */
	GdkWindow* vertical_scrollbar_window;
	GdkWindow* horizontal_scrollbar_window;
	gint vertical_scrollbar_size;
	gint horizontal_scrollbar_size;
	guint hide_scrollbars_timeout_id;
	GdkGC* hilight_gc;
	GdkGC* shadow_gc;
};

static gint compare_pointers(gconstpointer a, gconstpointer b, gpointer user_data) {
	return a - b;
}

static void disable_hadjustment(MidoriScrolledWindow* scrolled_window) {
	GtkAdjustment* hadjustment;
	GtkWidget* viewport;

	if ((hadjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrolled_window))) &&
	    scrolled_window->priv->hadjustment != hadjustment) {
		scrolled_window->priv->hadjustment = hadjustment;
		scrolled_window->priv->viewport = NULL;
		viewport = GTK_WIDGET(scrolled_window);
		while (GTK_IS_BIN(viewport)) {
			viewport = gtk_bin_get_child(GTK_BIN(viewport));
			if (GTK_IS_VIEWPORT(viewport)) {
				scrolled_window->priv->viewport = viewport;
				break;
			}
		}
	}
	g_signal_handlers_block_matched(scrolled_window->priv->hadjustment, G_SIGNAL_MATCH_DATA, 0, 0, 0, 0, scrolled_window->priv->viewport);
}

static void enable_hadjustment(MidoriScrolledWindow* scrolled_window) {
	g_signal_handlers_unblock_matched(scrolled_window->priv->hadjustment, G_SIGNAL_MATCH_DATA, 0, 0, 0, 0, scrolled_window->priv->viewport);
}

static gboolean on_expose_event(GtkWidget* widget, GdkEventExpose* event, MidoriScrolledWindow* scrolled_window) {
	gboolean ret = FALSE;

	if (GTK_WIDGET_DRAWABLE(widget)) {
		if (event->window == scrolled_window->priv->horizontal_scrollbar_window) {
			if (scrolled_window->priv->horizontal_scrollbar_size) {
				gdk_draw_line(event->window, scrolled_window->priv->hilight_gc, 0, 0, scrolled_window->priv->horizontal_scrollbar_size - 1, 0);
				gdk_draw_line(event->window, scrolled_window->priv->hilight_gc, 0, 1, 0, 9);
				gdk_draw_line(event->window, scrolled_window->priv->shadow_gc, scrolled_window->priv->horizontal_scrollbar_size - 1, 1, scrolled_window->priv->horizontal_scrollbar_size - 1, 9);		
				gdk_draw_line(event->window, scrolled_window->priv->shadow_gc, 0, 9, scrolled_window->priv->horizontal_scrollbar_size - 1, 9);
			}
			
			ret = TRUE;
		} else if (event->window == scrolled_window->priv->vertical_scrollbar_window) {
			if (scrolled_window->priv->vertical_scrollbar_size) {
				gdk_draw_line(event->window, scrolled_window->priv->hilight_gc, 0, 0, 9, 0);
				gdk_draw_line(event->window, scrolled_window->priv->hilight_gc, 0, 1, 0, scrolled_window->priv->vertical_scrollbar_size - 1);
				gdk_draw_line(event->window, scrolled_window->priv->shadow_gc, 9, 1, 9, scrolled_window->priv->vertical_scrollbar_size - 1);		
				gdk_draw_line(event->window, scrolled_window->priv->shadow_gc, 0, scrolled_window->priv->vertical_scrollbar_size - 1, 9, scrolled_window->priv->vertical_scrollbar_size - 1);
			}

			ret = TRUE;		
		}
	}
  
	return ret;
}

static gboolean adjust_scrollbar(MidoriScrolledWindow* scrolled_window, GdkWindow* scrollbar_window, GtkAdjustment* adjustment, gint* previous_size, gboolean horizontal) {
	GtkWidget* widget;
	gint x, y;
	gint size;
	double position;
	GtkWidget* window;

	if (adjustment->page_size >= adjustment->upper - adjustment->lower) {
		*previous_size = 0;
		
		return FALSE;
	}

	widget = GTK_WIDGET(scrolled_window);
	size = ((double)adjustment->page_size) / (adjustment->upper - adjustment->lower) * (horizontal ? widget->allocation.height : widget->allocation.width);
	if (size != *previous_size) {
		*previous_size = size;
		if (horizontal) {
			gdk_window_resize(scrollbar_window, 10, size);
			gdk_window_clear(scrollbar_window);
			gdk_draw_line(scrollbar_window, scrolled_window->priv->hilight_gc, 0, 0, 9, 0);
			gdk_draw_line(scrollbar_window, scrolled_window->priv->hilight_gc, 0, 1, 0, size - 1);
			gdk_draw_line(scrollbar_window, scrolled_window->priv->shadow_gc, 9, 1, 9, size - 1);		
			gdk_draw_line(scrollbar_window, scrolled_window->priv->shadow_gc, 0, size - 1, 9, size - 1);
		} else {
			gdk_window_resize(scrollbar_window, size, 10);
			gdk_window_clear(scrollbar_window);
			gdk_draw_line(scrollbar_window, scrolled_window->priv->hilight_gc, 0, 0, size - 1, 0);
			gdk_draw_line(scrollbar_window, scrolled_window->priv->hilight_gc, 0, 1, 0, 9);
			gdk_draw_line(scrollbar_window, scrolled_window->priv->shadow_gc, size - 1, 1, size - 1, 9);		
			gdk_draw_line(scrollbar_window, scrolled_window->priv->shadow_gc, 0, 9, size - 1, 9);
		}
	}

	position = (adjustment->value - adjustment->lower) / (adjustment->upper - adjustment->lower);
	window = gtk_widget_get_toplevel(widget);
	if (horizontal) {
		gtk_widget_translate_coordinates(widget, window, widget->allocation.width - 20, position * widget->allocation.height, &x, &y);
		gdk_window_move(scrollbar_window, x, y);	
	} else {
		gtk_widget_translate_coordinates(widget, window, position * widget->allocation.width, widget->allocation.height - 20, &x, &y);
		gdk_window_move(scrollbar_window, x, y);	
	}
	
	return TRUE;
}

static gboolean hide_scrollbars_timeout(gpointer data) {
	MidoriScrolledWindow* scrolled_window = MIDORI_SCROLLED_WINDOW(data);

	gdk_threads_enter();
	gdk_window_hide(scrolled_window->priv->vertical_scrollbar_window);	
	gdk_window_hide(scrolled_window->priv->horizontal_scrollbar_window);	

	scrolled_window->priv->hide_scrollbars_timeout_id = 0;
	gdk_threads_leave();

	return FALSE;
}

static gdouble calculate_timeout_scroll_values(gdouble old_value, gdouble upper_limit, gdouble* scrolling_speed_pointer, gdouble deceleration, gdouble* other_deceleration, gdouble normal_deceleration) {
	gdouble new_value = old_value;
	
	if (*scrolling_speed_pointer > deceleration ||
	    *scrolling_speed_pointer < -deceleration) {
		if (old_value + *scrolling_speed_pointer <= 0.0) {
			new_value = -1.0;
			*scrolling_speed_pointer = 0.0;
			*other_deceleration = normal_deceleration;
		} else if (old_value + *scrolling_speed_pointer >= upper_limit) {
			new_value = upper_limit;
			*scrolling_speed_pointer = 0.0;
			*other_deceleration = normal_deceleration;
		} else {
			new_value = old_value + *scrolling_speed_pointer;
		}
		if (*scrolling_speed_pointer > deceleration) {
			*scrolling_speed_pointer -= deceleration;
		} else if (*scrolling_speed_pointer < -deceleration) {
			*scrolling_speed_pointer += deceleration;
		}
	}
	
	return new_value;
}

static void do_timeout_scroll(MidoriScrolledWindow* scrolled_window) {
	GtkAdjustment* hadjustment;
	GtkAdjustment* vadjustment;
	gdouble hvalue;
	gdouble vvalue;
	
	hadjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
	vadjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
	hvalue = calculate_timeout_scroll_values(hadjustment->value, hadjustment->upper - hadjustment->page_size,
	                        		 &scrolled_window->priv->horizontal_speed,
						 scrolled_window->priv->horizontal_deceleration,
						 &scrolled_window->priv->vertical_deceleration,
						 scrolled_window->priv->deceleration);
	vvalue = calculate_timeout_scroll_values(vadjustment->value, vadjustment->upper - vadjustment->page_size,
	                			 &scrolled_window->priv->vertical_speed,
						 scrolled_window->priv->vertical_deceleration,
						 &scrolled_window->priv->horizontal_deceleration,
						 scrolled_window->priv->deceleration);
	if (vvalue != vadjustment->value) {
		if (hvalue != hadjustment->value) {
			disable_hadjustment(scrolled_window);
			gtk_adjustment_set_value(hadjustment, hvalue);
			enable_hadjustment(scrolled_window);
		}
		gtk_adjustment_set_value(vadjustment, vvalue);
	} else if (hvalue != hadjustment->value) {
		gtk_adjustment_set_value(hadjustment, hvalue);
	}

	adjust_scrollbar(scrolled_window, scrolled_window->priv->horizontal_scrollbar_window,
	                 gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrolled_window)),
			 &scrolled_window->priv->horizontal_scrollbar_size, FALSE);
	adjust_scrollbar(scrolled_window, scrolled_window->priv->vertical_scrollbar_window,
	                 gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window)),
			 &scrolled_window->priv->vertical_scrollbar_size, TRUE);
}

static gboolean timeout_scroll(gpointer data) {
	gboolean ret = TRUE;
	MidoriScrolledWindow* scrolled_window = MIDORI_SCROLLED_WINDOW(data);

	gdk_threads_enter();
	do_timeout_scroll(scrolled_window);

	if (scrolled_window->priv->vertical_speed < scrolled_window->priv->deceleration &&
	    scrolled_window->priv->vertical_speed > -scrolled_window->priv->deceleration &&
	    scrolled_window->priv->horizontal_speed < scrolled_window->priv->deceleration &&
	    scrolled_window->priv->horizontal_speed > -scrolled_window->priv->deceleration) {
		scrolled_window->priv->scrolling_timeout_id = 0;
		if (!scrolled_window->priv->hide_scrollbars_timeout_id) {
			scrolled_window->priv->hide_scrollbars_timeout_id = g_timeout_add(500, hide_scrollbars_timeout, scrolled_window);
		}

		ret = FALSE;
	}
	gdk_threads_leave();
	
	return ret;
}

static gdouble calculate_motion_scroll_values(gdouble old_value, gdouble upper_limit, gint current_coordinate, gint previous_coordinate) {
	gdouble new_value = old_value;
	gint movement;
	
	movement = current_coordinate - previous_coordinate;

	if (old_value - movement < upper_limit) {
		new_value = old_value - movement;
	} else {
		new_value = upper_limit;			
	}
	
	return new_value;
}

static void do_motion_scroll(MidoriScrolledWindow* scrolled_window, GtkWidget* widget, gint x, gint y, guint32 timestamp) {
	GtkAdjustment* hadjustment;
	GtkAdjustment* vadjustment;
	gdouble hvalue;
	gdouble vvalue;
	
	if (scrolled_window->priv->dragged ||
	    gtk_drag_check_threshold(widget, scrolled_window->priv->start_x, scrolled_window->priv->start_y, x, y)) {
		if (timestamp - scrolled_window->priv->previous_time > scrolled_window->priv->dragging_stopped_delay ||
		    !scrolled_window->priv->dragged) {
			scrolled_window->priv->dragged = TRUE;
			scrolled_window->priv->going_right = scrolled_window->priv->start_x < x;
			scrolled_window->priv->going_down = scrolled_window->priv->start_y < y;
			scrolled_window->priv->start_x = scrolled_window->priv->farest_x = x;
			scrolled_window->priv->start_y = scrolled_window->priv->farest_y = y;
			scrolled_window->priv->start_time = scrolled_window->priv->farest_time = timestamp;			
		} else {
			if ((scrolled_window->priv->going_right && x > scrolled_window->priv->farest_x) ||
			    (!scrolled_window->priv->going_right && x < scrolled_window->priv->farest_x)) {
				scrolled_window->priv->farest_x = x;
				scrolled_window->priv->farest_time = timestamp;
			}
			if ((scrolled_window->priv->going_down && y > scrolled_window->priv->farest_y) ||
			    (!scrolled_window->priv->going_down && y < scrolled_window->priv->farest_y)) {
				scrolled_window->priv->farest_y = y;
				scrolled_window->priv->farest_time = timestamp;
			}
			if (gtk_drag_check_threshold(widget, scrolled_window->priv->farest_x, scrolled_window->priv->farest_y, x, y)) {
				scrolled_window->priv->start_x = scrolled_window->priv->farest_x;
				scrolled_window->priv->farest_x = x;
				scrolled_window->priv->start_y = scrolled_window->priv->farest_y;
				scrolled_window->priv->farest_y = y;
				scrolled_window->priv->start_time = scrolled_window->priv->farest_time;			
				scrolled_window->priv->farest_time = timestamp;			
				scrolled_window->priv->going_right = scrolled_window->priv->start_x < x;
				scrolled_window->priv->going_down = scrolled_window->priv->start_y < y;
			}
		}

		hadjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
		vadjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
		hvalue = calculate_motion_scroll_values(hadjustment->value, hadjustment->upper - hadjustment->page_size, x, scrolled_window->priv->previous_x);
		vvalue = calculate_motion_scroll_values(vadjustment->value, vadjustment->upper - vadjustment->page_size, y, scrolled_window->priv->previous_y);
		if (vvalue != vadjustment->value) {
			if (hvalue != hadjustment->value) {
				disable_hadjustment(scrolled_window);
				gtk_adjustment_set_value(hadjustment, hvalue);
				enable_hadjustment(scrolled_window);
			}
			gtk_adjustment_set_value(vadjustment, vvalue);
		} else if (hvalue != hadjustment->value) {
			gtk_adjustment_set_value(hadjustment, hvalue);
		}	
	}

	scrolled_window->priv->previous_y = y;
	scrolled_window->priv->previous_x = x;
	scrolled_window->priv->previous_time = timestamp;

	adjust_scrollbar(scrolled_window, scrolled_window->priv->horizontal_scrollbar_window,
	                 gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrolled_window)),
			 &scrolled_window->priv->horizontal_scrollbar_size, FALSE);
	adjust_scrollbar(scrolled_window, scrolled_window->priv->vertical_scrollbar_window,
	                 gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window)),
			 &scrolled_window->priv->vertical_scrollbar_size, TRUE);
}

static gboolean button_press_event(GtkWidget* widget, GdkEventButton* event, MidoriScrolledWindow* scrolled_window) {
	gint x;
	gint y;
	GdkModifierType mask;

	if (!scrolled_window->priv->drag_scrolling) {

		return FALSE;
	}

	if (event->button != 1)
        	return FALSE;

	scrolled_window->priv->press_received = TRUE;

	if (event->time - scrolled_window->priv->previous_time < scrolled_window->priv->dragging_stopped_delay &&
	    gtk_drag_check_threshold(widget, scrolled_window->priv->previous_x, scrolled_window->priv->previous_y, x, y)) {
		if (scrolled_window->priv->scrolling_timeout_id) {
			g_source_remove(scrolled_window->priv->scrolling_timeout_id);
			scrolled_window->priv->scrolling_timeout_id = 0;
		}
		gdk_window_get_pointer(GTK_WIDGET(scrolled_window)->window, &x, &y, &mask);
/*		do_motion_scroll(scrolled_window, widget, x, y, event->time); */
	} else {
		if (scrolled_window->priv->scrolling_timeout_id) {
			g_source_remove(scrolled_window->priv->scrolling_timeout_id);
			scrolled_window->priv->scrolling_timeout_id = 0;
			scrolled_window->priv->previous_time = 0;
		} else {
			scrolled_window->priv->dragged = FALSE;
			scrolled_window->priv->previous_time = event->time;
		}
		gdk_window_get_pointer(GTK_WIDGET(scrolled_window)->window, &x, &y, &mask);
		scrolled_window->priv->start_x = scrolled_window->priv->previous_x = scrolled_window->priv->farest_x = x;
		scrolled_window->priv->start_y = scrolled_window->priv->previous_y = scrolled_window->priv->farest_y = y;
		scrolled_window->priv->start_time  = event->time;
	}

	if (scrolled_window->priv->scrolling_hints && !GTK_SCROLLED_WINDOW(scrolled_window)->hscrollbar_visible &&
	    adjust_scrollbar(scrolled_window, scrolled_window->priv->horizontal_scrollbar_window,
		             gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrolled_window)),
	                     &scrolled_window->priv->horizontal_scrollbar_size, FALSE)) {
		gdk_window_raise(scrolled_window->priv->horizontal_scrollbar_window);
		gdk_window_show(scrolled_window->priv->horizontal_scrollbar_window);
	}
	if (scrolled_window->priv->scrolling_hints && !GTK_SCROLLED_WINDOW(scrolled_window)->vscrollbar_visible &&
	    adjust_scrollbar(scrolled_window, scrolled_window->priv->vertical_scrollbar_window,
		             gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window)),
	                     &scrolled_window->priv->vertical_scrollbar_size, TRUE)) {
	 	gdk_window_raise(scrolled_window->priv->vertical_scrollbar_window);
		gdk_window_show(scrolled_window->priv->vertical_scrollbar_window);				 
	}
	if (scrolled_window->priv->hide_scrollbars_timeout_id) {
		g_source_remove(scrolled_window->priv->hide_scrollbars_timeout_id);
		scrolled_window->priv->hide_scrollbars_timeout_id = 0;
	}

	return FALSE;
}

#define ABSOLUTE_VALUE(x) ((x) < 0 ? -(x) : (x))

static gboolean button_release_event(GtkWidget* widget, GdkEventButton* event, MidoriScrolledWindow* scrolled_window) {
	gint x;
	gint y;
	GdkModifierType mask;

	gdk_window_get_pointer(GTK_WIDGET(scrolled_window)->window, &x, &y, &mask);
	if (scrolled_window->priv->press_received &&
	    gtk_drag_check_threshold(widget, scrolled_window->priv->start_x, scrolled_window->priv->start_y, x, y)) {
		scrolled_window->priv->dragged = TRUE;
	}
	
	if (scrolled_window->priv->press_received && scrolled_window->priv->kinetic_scrolling &&
	    event->time - scrolled_window->priv->previous_time < scrolled_window->priv->dragging_stopped_delay) {
		scrolled_window->priv->vertical_speed = (gdouble)(scrolled_window->priv->start_y - y) / (event->time - scrolled_window->priv->start_time) * scrolled_window->priv->interval;
		scrolled_window->priv->horizontal_speed = (gdouble)(scrolled_window->priv->start_x - x) / (event->time - scrolled_window->priv->start_time) * scrolled_window->priv->interval;
		if (ABSOLUTE_VALUE(scrolled_window->priv->vertical_speed) > ABSOLUTE_VALUE(scrolled_window->priv->horizontal_speed)) {
			scrolled_window->priv->vertical_deceleration = scrolled_window->priv->deceleration;
			scrolled_window->priv->horizontal_deceleration = scrolled_window->priv->deceleration * ABSOLUTE_VALUE(scrolled_window->priv->horizontal_speed / scrolled_window->priv->vertical_speed);
		} else {
			scrolled_window->priv->horizontal_deceleration = scrolled_window->priv->deceleration;
			scrolled_window->priv->vertical_deceleration = scrolled_window->priv->deceleration * ABSOLUTE_VALUE(scrolled_window->priv->vertical_speed / scrolled_window->priv->horizontal_speed);
		}
		scrolled_window->priv->scrolling_timeout_id = g_timeout_add(scrolled_window->priv->interval, timeout_scroll, scrolled_window);

		do_timeout_scroll(scrolled_window);
	} else if (!scrolled_window->priv->hide_scrollbars_timeout_id) {
		scrolled_window->priv->hide_scrollbars_timeout_id = g_timeout_add(500, hide_scrollbars_timeout, scrolled_window);
	}
	scrolled_window->priv->previous_x = x;
	scrolled_window->priv->previous_y = y;
	scrolled_window->priv->previous_time = event->time;

    	scrolled_window->priv->press_received = FALSE;

	return FALSE;
}

static gboolean motion_notify_event(GtkWidget* widget, GdkEventMotion* event, MidoriScrolledWindow* scrolled_window) {
	gint x;
	gint y;
	GdkModifierType mask;

	if (scrolled_window->priv->press_received) {
		gdk_window_get_pointer(GTK_WIDGET(scrolled_window)->window, &x, &y, &mask);
		do_motion_scroll(scrolled_window, widget, x, y, event->time);
	}

	return FALSE;
}

static gboolean event_handler(GdkEvent* event, MidoriEventHandlerState* state, gpointer user_data) {
	gboolean stop_propagating;
	GdkEventCrossing crossing;
	
	stop_propagating = FALSE;

	if (event->type == GDK_BUTTON_PRESS) {

		gdk_window_get_user_data(event->button.window, (gpointer)&current_widget);

		if ((current_scrolled_window = g_tree_lookup(activated_widgets, current_widget))) {
			current_gdk_window = event->button.window;
	
		stop_propagating = button_press_event(current_widget, &event->button, current_scrolled_window);
		} else {
			current_gdk_window = NULL;
		}

	} else if (event->any.window == current_gdk_window) {

		if (event->type == GDK_MOTION_NOTIFY) {
			if (current_scrolled_window->priv->dragged) {
				stop_propagating = motion_notify_event(current_widget, &event->motion, current_scrolled_window);
			} else {
				stop_propagating = motion_notify_event(current_widget, &event->motion, current_scrolled_window);
				if (current_scrolled_window->priv->dragged) {
					crossing.type = GDK_LEAVE_NOTIFY;
					crossing.window = event->motion.window;
					crossing.send_event = event->motion.send_event;
					crossing.subwindow = GTK_WIDGET(current_scrolled_window)->window;
					crossing.time = event->motion.time;
					crossing.x = event->motion.x;
					crossing.y = event->motion.y;
					crossing.x_root = event->motion.x_root;
					crossing.y_root = event->motion.y_root;
					crossing.mode = GDK_CROSSING_GRAB;
					crossing.detail = GDK_NOTIFY_ANCESTOR;
					crossing.focus = TRUE;
					crossing.state = event->motion.state;

					gtk_main_do_event((GdkEvent*)&crossing);
					synthetized_crossing_event = TRUE;
				}
			}
		} else if ((event->type == GDK_ENTER_NOTIFY || event->type == GDK_LEAVE_NOTIFY) &&
	        	   synthetized_crossing_event) {

			stop_propagating = TRUE;
		} else if (event->type == GDK_BUTTON_RELEASE) {

			stop_propagating = button_release_event(current_widget, &event->button, current_scrolled_window);   
		}
	}

	
	if (!stop_propagating) {
		midori_event_handler_next(event, state);
	}

	if (event->type == GDK_BUTTON_RELEASE && event->button.window == current_gdk_window) {

		crossing.type = GDK_ENTER_NOTIFY;
		crossing.window = event->button.window;
		crossing.send_event = event->button.send_event;
		crossing.subwindow = GTK_WIDGET(current_scrolled_window)->window;
		crossing.time = event->button.time;
		crossing.x = event->button.x;
		crossing.y = event->button.y;
		crossing.x_root = event->button.x_root;
		crossing.y_root = event->button.y_root;
		crossing.mode = GDK_CROSSING_UNGRAB;
		crossing.detail = GDK_NOTIFY_ANCESTOR;
		crossing.focus = TRUE;
		crossing.state = event->button.state;

		gtk_main_do_event((GdkEvent*)&crossing);
		synthetized_crossing_event = FALSE;
	}
	
	return stop_propagating;
}

static void add(GtkContainer* container, GtkWidget* widget) {
	GtkContainerClass* container_class;
	
	midori_scrolled_window_activate_scrolling(MIDORI_SCROLLED_WINDOW(container), widget);
	
	container_class = GTK_CONTAINER_CLASS(g_type_class_peek_parent(g_type_class_peek(MIDORI_TYPE_SCROLLED_WINDOW)));
	if (container_class->add) {
		container_class->add(container, widget);
	}
	
}

static void realize(GtkWidget* widget) {
	GtkWidgetClass* widget_class;
	MidoriScrolledWindow* scrolled_window;
	GdkWindowAttr attr;
	GdkColor color;

	widget_class = GTK_WIDGET_CLASS(g_type_class_peek_parent(g_type_class_peek(MIDORI_TYPE_SCROLLED_WINDOW)));
	if (widget_class->realize) {
		widget_class->realize(widget);
	}

	widget->window = gtk_widget_get_parent_window (widget);
	g_object_ref (widget->window);
	
	scrolled_window = MIDORI_SCROLLED_WINDOW(widget);
	
	attr.height = attr.width = 10;
	attr.event_mask = GDK_EXPOSURE_MASK;
	attr.wclass = GDK_INPUT_OUTPUT;
	attr.window_type = GDK_WINDOW_CHILD;
	attr.override_redirect = TRUE;
	scrolled_window->priv->vertical_scrollbar_window = gdk_window_new(widget->window, &attr, 0);
	scrolled_window->priv->horizontal_scrollbar_window = gdk_window_new(widget->window, &attr, 0);
	
	gdk_window_set_user_data(scrolled_window->priv->vertical_scrollbar_window, widget);
	gdk_window_set_user_data(scrolled_window->priv->horizontal_scrollbar_window, widget);
	g_signal_connect(widget, "expose-event", G_CALLBACK(on_expose_event), scrolled_window);

	color.red = color.green = color.blue = 0x9999;
	gdk_rgb_find_color(gdk_colormap_get_system(), &color);
	gdk_window_set_background(scrolled_window->priv->vertical_scrollbar_window, &color);
	gdk_window_set_background(scrolled_window->priv->horizontal_scrollbar_window, &color);	
	
	scrolled_window->priv->hilight_gc = gdk_gc_new(widget->window);
	color.red = color.green = color.blue = 0xcccc;
	gdk_gc_set_rgb_fg_color(scrolled_window->priv->hilight_gc, &color);
	scrolled_window->priv->shadow_gc = gdk_gc_new(widget->window);
	color.red = color.green = color.blue = 0x6666;
	gdk_gc_set_rgb_fg_color(scrolled_window->priv->shadow_gc, &color);

	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
}

static void dispose(GObject* object) {
	MidoriScrolledWindow* scrolled_window;
	MidoriScrolledWindowClass* klass;  

	scrolled_window = MIDORI_SCROLLED_WINDOW(object);
	if (scrolled_window->priv->scrolling_timeout_id) {
		g_source_remove(scrolled_window->priv->scrolling_timeout_id);
		scrolled_window->priv->scrolling_timeout_id = 0;
	}
	if (scrolled_window->priv->hide_scrollbars_timeout_id) {
		g_source_remove(scrolled_window->priv->hide_scrollbars_timeout_id);
		scrolled_window->priv->hide_scrollbars_timeout_id = 0;
	}

	klass = MIDORI_SCROLLED_WINDOW_GET_CLASS(object);
	G_OBJECT_CLASS(g_type_class_peek_parent(klass))->dispose(object);
}

static void class_init(gpointer klass, gpointer data) {
	G_OBJECT_CLASS(klass)->dispose = dispose;
	GTK_WIDGET_CLASS(klass)->realize = realize;
	GTK_CONTAINER_CLASS(klass)->add = add;

	activated_widgets = g_tree_new((GCompareFunc)compare_pointers);
	current_gdk_window = NULL;

	midori_event_handler_append(event_handler, NULL);
}

static void object_init(GTypeInstance* instance, gpointer klass) {
	MidoriScrolledWindow* scrolled_window = MIDORI_SCROLLED_WINDOW(instance);

	scrolled_window->priv = g_new0(MidoriScrolledWindowPrivate, 1);
	scrolled_window->priv->interval = DEFAULT_INTERVAL;
	scrolled_window->priv->deceleration = DEFAULT_DECELERATION;
	scrolled_window->priv->drag_scrolling = TRUE;
	scrolled_window->priv->kinetic_scrolling = TRUE;
	scrolled_window->priv->dragging_stopped_delay = DEFAULT_DRAGGING_STOPPED_DELAY;
}

GType midori_scrolled_window_get_type() {
	static GType type = 0;
	static const GTypeInfo info = {
		sizeof (MidoriScrolledWindowClass),
		NULL,   /* base_init */
		NULL,   /* base_finalize */
		class_init,
		NULL,   /* class_finalize */
		NULL,   /* class_data */
		sizeof (MidoriScrolledWindow),
		0,
		object_init,
		NULL
	};

	if (!type) {
		type = g_type_register_static(GTK_TYPE_SCROLLED_WINDOW, "MidoriScrolledWindow", &info, 0);
	}

	return type;
}

/**
 * midori_scrolled_window_new:
 * @hadjustment: a horizontal #GtkAdjustment
 * @vadjustment: a vertical #GtkAdjustment
 *
 * Similar function than the #gtk_scrolled_window_new.
 **/

GtkWidget* midori_scrolled_window_new(GtkAdjustment* hadjustment, GtkAdjustment* vadjustment) {
	if (hadjustment) {
	
		g_return_val_if_fail (GTK_IS_ADJUSTMENT (hadjustment), NULL);
	}
	if (vadjustment) {
	
		g_return_val_if_fail (GTK_IS_ADJUSTMENT (vadjustment), NULL);
	}
	
	return gtk_widget_new(MIDORI_TYPE_SCROLLED_WINDOW, "hadjustment", hadjustment, "vadjustment", vadjustment,
	                      "hscrollbar-policy", GTK_POLICY_NEVER, "vscrollbar-policy", GTK_POLICY_NEVER, NULL);
}

/**
 * midori_scrolled_window_activate_scrolling:
 * @scrolled_window: a #MidoriScrolledWindow
 * @widget: a #GtkWidget of which area is made active event source for drag and kinetic scrolling.
 *
 * Activates the widget so that pointer motion events inside the widget are used to scroll the #MidoriScrolledWindow.
 * The widget can be a child of the #MidoriScrolledWindow or even a separate widget ("touchpad" style).
 *
 * The direct child of the #MidoriScrolledWindow (typically #GtkViewport) is activated automatically when added.
 * This function has to be
 * used if indirect descendant widgets are stopping propagation of the button press and release as well as motion events
 * (for example GtkButton is doing so) but scrolling should be possible inside their area too.
 *
 * This function adds #GDK_BUTTON_PRESS_MASK, #GDK_BUTTON_RELEASE_MASK, #GDK_POINTER_MOTION_MASK, and
 * #GDK_MOTION_HINT_MAKS into the widgets event mask.
 */

void midori_scrolled_window_activate_scrolling(MidoriScrolledWindow* scrolled_window, GtkWidget* widget) {
	gtk_widget_add_events(widget, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
	g_tree_insert(activated_widgets, widget, scrolled_window);
}

void midori_scrolled_window_set_drag_scrolling(MidoriScrolledWindow* scrolled_window, gboolean drag_scrolling) {
	g_return_if_fail(MIDORI_IS_SCROLLED_WINDOW(scrolled_window));

	if (scrolled_window->priv->drag_scrolling && !drag_scrolling) {
		if (scrolled_window->priv->scrolling_timeout_id) {
			g_source_remove(scrolled_window->priv->scrolling_timeout_id);
			scrolled_window->priv->scrolling_timeout_id = 0;
			scrolled_window->priv->previous_time = 0;
		}

		gdk_window_hide(scrolled_window->priv->vertical_scrollbar_window);	
		gdk_window_hide(scrolled_window->priv->horizontal_scrollbar_window);
		if (scrolled_window->priv->hide_scrollbars_timeout_id) {
			g_source_remove(scrolled_window->priv->hide_scrollbars_timeout_id);
			scrolled_window->priv->hide_scrollbars_timeout_id = 0;
		}

		scrolled_window->priv->press_received = FALSE;
	}

	scrolled_window->priv->drag_scrolling = drag_scrolling;
}

gboolean midori_scrolled_window_is_dragged(MidoriScrolledWindow* scrolled_window) {
	g_return_val_if_fail(MIDORI_IS_SCROLLED_WINDOW(scrolled_window), FALSE);

	return scrolled_window->priv->dragged;
}

void midori_scrolled_window_set_scrolling_hints(MidoriScrolledWindow* scrolled_window, gboolean enabled) {
	scrolled_window->priv->scrolling_hints = enabled;
}
