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

#ifndef TABLEWARE_H
#define TABLEWARE_H

#include <Xlib.h>
#include <xcb/xcb.h>
#include <stdbool.h>

#define PLUGIN_NAME         "TableWare Plugin"
#define MIME_TYPES_HANDLED  "application/x-tableware:tableware:Plugin to test browser XEmbed capabilities"
#define PLUGIN_DESCRIPTION  "Plugin that tests XEmbed capabilities"

typedef struct {
    uint16 mode;
    int childwindow;
    bool state;
	char *class;
	bool swallowed;

    NPWindow *window;
    uint32 x, y;
    uint32 width, height;

    NPP instance;
    NPBool pluginsHidden;

} PluginInstance;

NPError TablewareWindow(NPP instance, NPWindow* window);
void reparent_window(xcb_window_t parent, xcb_window_t child);

#endif /* TABLEWARE_H */
