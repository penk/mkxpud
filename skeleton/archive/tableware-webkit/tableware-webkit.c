#include <gtk/gtk.h>
#include <stdlib.h>
#include <webkit/webkit.h>

int main (int argc, char *argv[])
{
	GtkWidget *box;

	g_thread_init(NULL);
	gtk_init (&argc, &argv);

	box = gtk_vbox_new (FALSE, 0);

	GtkWidget *web_view = webkit_web_view_new ();

	GtkWidget* scrolled_window = gtk_scrolled_window_new (NULL, NULL);

	GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL);
	gtk_window_set_default_size(GTK_WINDOW (window), 800, 600);

	gtk_container_add (GTK_CONTAINER (window), box);
	gtk_container_add (GTK_CONTAINER (box), scrolled_window);
	gtk_container_add (GTK_CONTAINER (scrolled_window), web_view);

	gtk_widget_show_all (window);
	gtk_window_fullscreen(GTK_WINDOW (window));

	gchar* uri = (gchar*) (argc > 1 ? argv[1] : "http://localhost/usr/share/plate/chrome/content");
	webkit_web_view_open (WEBKIT_WEB_VIEW (web_view), uri);

	gtk_main();
	return 0;
}
