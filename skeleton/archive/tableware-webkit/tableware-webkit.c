#include <gtk/gtk.h>
#include <stdlib.h>
#include <webkit/webkit.h>

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
	//gtk_window_fullscreen(GTK_WINDOW (window));

	g_signal_connect(G_OBJECT(WEBKIT_WEB_VIEW (web_view)), "mime-type-policy-decision-requested", G_CALLBACK(decidedownload), NULL);
	g_signal_connect(G_OBJECT(WEBKIT_WEB_VIEW (web_view)), "download-requested", G_CALLBACK(initdownload), NULL);

	gchar* uri = (gchar*) (argc > 1 ? argv[1] : "http://localhost/usr/share/plate/chrome/content");
	webkit_web_view_open (WEBKIT_WEB_VIEW (web_view), uri);

	gtk_main();
	return 0;
}
