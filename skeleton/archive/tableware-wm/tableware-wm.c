/* 
 *  Ping-Hsun Chen <penkia@gmail.com>, 2010
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_property.h>
#include <xcb/xcb_atom.h>

#include <glib.h>

#include "tableware-wm.h"


void check_error(xcb_connection_t *conn, xcb_void_cookie_t cookie, char *err_message) {
        xcb_generic_error_t *error = xcb_request_check(conn, cookie);
        if (error != NULL) {
                fprintf(stderr, "ERROR: %s (X error %d)\n", err_message , error->error_code);
                xcb_disconnect(conn);
                //return -1;
				exit(-1);
        }
}

bool ping_client(xcb_connection_t *conn, xcb_window_t cwin) 
{

	xcb_generic_error_t *error;
	xcb_void_cookie_t cookie;

	xcb_atom_t wm_protocols = xcb_atom_get(conn, "WM_PROTOCOLS");
	xcb_atom_t net_wm_ping = xcb_atom_get(conn, "_NET_WM_PING");

	xcb_client_message_event_t ping;

	ping.response_type = XCB_CLIENT_MESSAGE;
	ping.format = 32;
	ping.window =  cwin;
	ping.type = wm_protocols;
	ping.data.data32[1] = net_wm_ping;

	cookie = xcb_send_event_checked(conn, FALSE, cwin, 
	XCB_EVENT_MASK_NO_EVENT, (const char*) &ping);

	error = xcb_request_check(conn, cookie);
	
	if (error != NULL) return FALSE;
	else return TRUE;

}

void check_list(xcb_connection_t *conn, GList *list)
{
    GList *it = NULL;
	GList *tmp = NULL;
	int window_id;
	char msg[100] = "";

	time_t timestamp;
	time(&timestamp);

    for (it = list; it != NULL; it = it->next) {
		window_id = atoi(it->data);

		if (window_id == 0) continue;

		ping_client(conn, window_id);

		if (!ping_client(conn, window_id)) 
		{ 
			printf("ping %d failed, close it\n", window_id);
			// failed, remove element 
			sprintf(msg, "echo %ld:destroy:%d > /tmp/xpudctrl", timestamp, window_id);
			printf("%ld:destroy:%d", timestamp, window_id);
			//g_file_set_contents("/tmp/xpudctrl", msg, -1, NULL);
			system(msg);

			//FIXME: better way to remove id from list
			tmp = g_list_remove_link(list, it);
		}
    }

}

void print_list(xcb_connection_t *conn, GList *list)
{
    GList *it = NULL;
	int window_id;

	printf("\n\tGList: ");
    for (it = list; it; it = it->next) {
		window_id = atoi(it->data);
		printf("%d ", window_id);
    }
	printf("\n");
}

int setupscreen(xcb_connection_t *conn, xcb_screen_t *screen)
{
	uint32_t values[3];
    xcb_query_tree_reply_t *reply;
    int i, len;
    xcb_window_t *children;
    xcb_get_window_attributes_reply_t *attr;
    

    reply = xcb_query_tree_reply(conn,
		xcb_query_tree(conn, screen->root), 0);
    if (NULL == reply) return -1;

    len = xcb_query_tree_children_length(reply);
    children = xcb_query_tree_children(reply);

    for (i = 0; i < len; i ++)
    {
        attr = xcb_get_window_attributes_reply(
            conn, xcb_get_window_attributes(conn, children[i]), NULL);

	printf("Got already opened window %d\n", children[i]);

        if (!attr)
        {
            fprintf(stderr, "Couldn't get attributes for window %d.\n",
                    children[i]);
            continue;
        }
        if (!attr->override_redirect)
        {
			// got already opened window

			values[0] = 0;
			values[1] = 0;
			values[2] = screen->width_in_pixels;
			values[3] = screen->height_in_pixels;
   			xcb_configure_window(conn, children[i], XCB_CONFIG_WINDOW_X
             | XCB_CONFIG_WINDOW_Y
             | XCB_CONFIG_WINDOW_WIDTH
             | XCB_CONFIG_WINDOW_HEIGHT, values);

			xcb_flush(conn);
        }
        
        free(attr);
    }
 
    xcb_flush(conn);
    free(reply);
    return 0;
}

int main (int argc, char **argv)
{

// variables

	uint32_t values[3];

	xcb_connection_t *conn;
	xcb_screen_t *screen;
	//xcb_drawable_t win;
	xcb_drawable_t root;

	xcb_generic_event_t *ev;
	//xcb_get_geometry_reply_t *geom;
	//xcb_get_window_attributes_reply_t *attr;

	xcb_generic_error_t *error;
	uint32_t mask = 0;
	xcb_void_cookie_t cookie;
	char msg[100] = "";


	time_t timestamp;

// check list
	GList *list = NULL;
	list = g_list_append(list, g_strdup_printf("0"));

// setup xcb

    conn = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(conn)) return 1;

    screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
    root = screen->root;

// setup screen

	//setupscreen(conn, screen);

    mask = XCB_CW_EVENT_MASK;

    values[0] = XCB_EVENT_MASK_STRUCTURE_NOTIFY
		| XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT 
		| XCB_EVENT_MASK_PROPERTY_CHANGE
		| XCB_EVENT_MASK_ENTER_WINDOW 
        | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;

    cookie = xcb_change_window_attributes_checked(conn, root, mask, values);
    error = xcb_request_check(conn, cookie);

    xcb_flush(conn);
    
    if (NULL != error)
    {
        fprintf(stderr, 
                "Another window manager running? Exiting: %d\n",
                error->error_code);

        xcb_disconnect(conn);
        
        return 1;
    }

    for (;;)
    {

		check_list(conn, list);
		//print_list(conn, list);

        ev = xcb_wait_for_event(conn);

	time(&timestamp);

	switch (ev->response_type & ~0x80)
        {
        case XCB_MAP_REQUEST:
        {
            xcb_map_request_event_t *e;
            e = (xcb_map_request_event_t *) ev;

            printf("map request: %d\n", (int)e->window);
			xcb_map_window(conn, e->window);

			sprintf(msg, "echo %ld:map:%d > /tmp/xpudctrl", timestamp, (int)e->window);
			printf("%ld:map:%d", timestamp, (int)e->window);
			//g_file_set_contents("/tmp/xpudctrl", msg, -1, NULL);
			system(msg);

			printf("\t\nGLIST: Add %d\n", e->window);
			list = g_list_append(list, g_strdup_printf("%d", e->window));

			xcb_flush(conn);
        }
        break;

		xcb_flush(conn);
		} // end of switch
	} // end of for

return 0;
}
