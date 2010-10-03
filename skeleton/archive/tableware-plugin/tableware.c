/***********************************************************************

DiamondX plugin
Example XEmbed-aware Mozilla browser plugin by Adobe.

Copyright (c) 2007 Adobe Systems Incorporated

 

Permission is hereby granted, free of charge, to any person obtaining a 
copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation 
the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the 
Software is furnished to do so, subject to the following conditions:

 

The above copyright notice and this permission notice shall be included in 
all copies or substantial portions of the Software.

 

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
THE AUTHORS OR

COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE. *
***********************************************************************/

#include "npapi.h"
#include "npupp.h"
#include "npruntime.h"

#include "tableware.h"
#include <dbus/dbus.h>
#include <xcb/xcb.h>
#include <stdbool.h>

NPError TablewareSetWindow(NPP instance, NPWindow* window)
{
    PluginInstance* This;
	Display *display;
    NPSetWindowCallbackStruct *ws_info;

    int xembedSupported = 0;

    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    This = (PluginInstance*) instance->pdata;

    if (This == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    ws_info = (NPSetWindowCallbackStruct *)window->ws_info;

    /* Mozilla likes to re-run its greatest hits */
    if ((window == This->window) &&
        (window->x == This->x) &&
        (window->y == This->y) &&
        (window->width == This->width) &&
        (window->height == This->height)) {
        //printf("  (window re-run; returning)\n");
        return NPERR_NO_ERROR;
    }

    NPN_GetValue(instance, NPNVSupportsXEmbedBool, &xembedSupported);
    if (!xembedSupported)
    {
        printf("TableWare: XEmbed not supported\n");
        return NPERR_GENERIC_ERROR;
    }

    This->window = window;
    This->x = window->x;
    This->y = window->y;
    This->width = window->width;
    This->height = window->height;

		char *displayname;
		display = XOpenDisplay(displayname);

/*
	This->window = XCreateSimpleWindow(display, (Window) window->window, 0, 0,
					       window->width, window->height, 0, 0, 0);
	XReparentWindow(display, This->window, (Window)window->window, 0, 0);
			printf("reparent just created %d into %d\n", This->window, (Window)window->window);
*/
			if( (!This->state) ){ 
				reparent_window(window->window, This->childwindow);
			}
			This->state=TRUE;


		XCloseDisplay(display);
/*
    NPObject *plugin = NULL;

    if( NPERR_NO_ERROR == NPN_GetValue(instance, NPNVWindowNPObject, &plugin) )
    {
        NPString script;
        script.utf8characters = "alert('NPN_Evaluate() test, document = ' + this);";
        script.utf8length = strlen(script.utf8characters);

        NPVariant result;

        if (NPN_Evaluate(instance, plugin, &script, &result))  
		{ 
			NPN_ReleaseVariantValue(&result);
		}
	}
	NPN_ReleaseObject(plugin);
*/

    return NPERR_NO_ERROR;
}

void reparent_window(xcb_window_t parent, xcb_window_t child) {

    xcb_connection_t *conn;
	xcb_void_cookie_t cookie;

    conn = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(conn)) return 1;

	printf("embed %d into %d\n", child, parent);

	cookie = xcb_reparent_window_checked(conn, child, parent, 0, 0);

	if (xcb_request_check(conn, cookie) != NULL) {
		printf("Could not reparent the window\n");
	}

	xcb_flush(conn);

}
