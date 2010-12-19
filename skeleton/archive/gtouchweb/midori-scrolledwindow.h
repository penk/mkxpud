/*
 *  Midori Scrolled Window - adapted from Miaouw by Nadav Wiener (2009)
 *  Miaouw - The Miaouw Library for Maemo Development
 *  Copyright (C) 2007 Henrik Hedberg <hhedberg@innologies.fi>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef MIDORI_SCROLLED_WINDOW_H
#define MIDORI_SCROLLED_WINDOW_H

#include <gtk/gtk.h>
#include <gtk/gtkscrolledwindow.h>

typedef struct _MidoriScrolledWindow MidoriScrolledWindow;
typedef struct _MidoriScrolledWindowClass MidoriScrolledWindowClass;
typedef struct _MidoriScrolledWindowPrivate MidoriScrolledWindowPrivate;

struct _MidoriScrolledWindow {
  GtkScrolledWindow parent;

  /* private */
  MidoriScrolledWindowPrivate* priv;
};

struct _MidoriScrolledWindowClass {
  GtkScrolledWindowClass parent;
  /* class members */
};

GType midori_scrolled_window_get_type (void);

#define MIDORI_TYPE_SCROLLED_WINDOW (midori_scrolled_window_get_type())
#define MIDORI_SCROLLED_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), MIDORI_TYPE_SCROLLED_WINDOW, MidoriScrolledWindow))
#define MIDORI_SCROLLED_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), MIDORI_TYPE_SCROLLED_WINDOW, MidoriScrolledWindowClass))
#define MIDORI_IS_SCROLLED_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), MIDORI_TYPE_SCROLLED_WINDOW))
#define MIDORI_IS_SCROLLED_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), MIDORI_TYPE_SCROLLED_WINDOW))
#define MIDORI_SCROLLED_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), MIDORI_TYPE_SCROLLED_WINDOW, MidoriScrolledWindowClass))

GtkWidget* midori_scrolled_window_new(GtkAdjustment* hadjustment, GtkAdjustment* vadjustment);

void midori_scrolled_window_activate_scrolling(MidoriScrolledWindow* scrolled_window, GtkWidget* widget);
gboolean midori_scrolled_window_is_drag_scrolled(MidoriScrolledWindow* scrolled_window);
void midori_scrolled_window_set_drag_scrolling(MidoriScrolledWindow* scrolled_window, gboolean panning);
void midori_scrolled_window_set_scrolling_hints(MidoriScrolledWindow* scrolled_window, gboolean enabled);

#endif
