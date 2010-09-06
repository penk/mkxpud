/* See LICENSE file for copyright and license details.
 *
 * uuwm is designed like any other X client as well. It is driven through
 * handling X events. In contrast to other X clients, a window manager selects
 * for SubstructureRedirectMask on the root window, to receive events about
 * window (dis-)appearance.  Only one X connection at a time is allowed to
 * select for this event mask.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag.  Clients are organized in a global
 * linked client list, the focus history is remembered through a global
 * stack list.
 *
 * To understand everything else, start reading main().
 */

/*
 * TODO:
 * - NetWM support for docks
 */

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <time.h>

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_event.h>

#include <glib.h>

typedef struct client_t {
    xcb_window_t win;
    int x, y, w, h;

    int bw;
    int oldbw; /* To be restored on WM exit */

    bool is_floating;

    struct client_t *next;
    struct client_t *snext;
} client_t;

static xcb_connection_t *conn;
static xcb_screen_t *screen;

enum {
    WMProtocols,
    WMDelete,
    WMState,
    NetSupported,
    NetWMName,
    AtomLast,
    NetFirst=NetSupported,
    NetLast=AtomLast
};

static xcb_atom_t atom[AtomLast];
static const char *atom_names[AtomLast] = {
    "WM_PROTOCOLS",
    "WM_DELETE_WINDOW",
    "WM_STATE",
    "_NET_SUPPORTED",
    "_NET_WM_NAME"
};

static int sx, sy, sw, sh; /* X display screen geometry x, y, w, h */
static int wx, wy, ww, wh; /* window area geometry x, y, w, h, docks excluded */

static client_t *clients = NULL;
static client_t *stack = NULL;

/* Allocs size zero-filled bytes or dies if unable to do so. */
static void *
xalloc(size_t size)
{
    void *res = calloc(1, size);
    if (!res)
        err(1, "Unable to alloc %d bytes", size);
    return res;
}

static void
debug(const char *errstr, ...)
{
    static bool _debug_init = false;
    static bool _debug = false;

    if (!_debug_init) {
        _debug_init = true;
        _debug = getenv("DEBUG") != NULL;
    }

    if (!_debug) return;

    fprintf(stderr, "D: ");
    va_list ap;
    va_start(ap, errstr);
    vfprintf(stderr, errstr, ap);
    va_end(ap);
}

static void
checkotherwm()
{
    uint32_t mask = 0;
    xcb_params_cw_t params;
    XCB_AUX_ADD_PARAM(&mask, &params, event_mask,
                      XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT);

    xcb_void_cookie_t c
        = xcb_aux_change_window_attributes_checked(conn, screen->root,
                                                   mask, &params);

    if (xcb_request_check(conn, c))
        errx(1, "another window manager is already running");
}

static void
configure_event(client_t *c)
{
    xcb_configure_notify_event_t e;

    e.response_type = XCB_CONFIGURE_NOTIFY;
    e.event = c->win;
    e.window = c->win;
    e.x = c->x;
    e.y = c->y;
    e.width = c->w;
    e.height = c->h;
    e.border_width = 0;
    e.above_sibling = XCB_NONE;
    e.override_redirect = false;

    xcb_void_cookie_t cookie
        = xcb_send_event(conn, false, c->win,
                         XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char*)&e);
    xcb_generic_error_t *err = xcb_request_check(conn, cookie);
    if (err)
        errx(1, "Unable to send configure event to %x (%d)", c->win,
             err->error_code);
}

static void
configure(xcb_window_t win, uint16_t mask, xcb_params_configure_window_t *params)
{
    debug("configure: win: %x, mask %d\n", win, mask);

    xcb_void_cookie_t c
        = xcb_aux_configure_window(conn, win, mask, params);

    xcb_generic_error_t *err = xcb_request_check(conn, c);
    if (err) {
        debug("configure: xcb_aux_configure_window error: %d\n", err->error_code);
        if (err->error_code != XCB_WINDOW)
            errx(1, "Unable to configure window %x (%d)", win, err->error_code);
        /* BadWindow is ignored as windows may disappear at any time */
        free(err);
    }
}

static void
arrange_updated(client_t *c, uint16_t m, xcb_params_configure_window_t *p)
{
    if (c->is_floating) {
        if (c->x < wx) { XCB_AUX_ADD_PARAM(&m, p, x, wx); c->x = wx; }
        if (c->y < wy) { XCB_AUX_ADD_PARAM(&m, p, y, wy); c->y = wy; }
        if (c->w > ww) { XCB_AUX_ADD_PARAM(&m, p, width, ww); c->w = ww; }
        if (c->h > wh) { XCB_AUX_ADD_PARAM(&m, p, height, wh); c->h = wh; }
    } else {
        if (c->x != wx) { XCB_AUX_ADD_PARAM(&m, p, x, wx); c->x = wx; }
        if (c->y != wy) { XCB_AUX_ADD_PARAM(&m, p, y, wy); c->y = wy; }
        if (c->w != ww) { XCB_AUX_ADD_PARAM(&m, p, width, ww); c->w = ww; }
        if (c->h != wx) { XCB_AUX_ADD_PARAM(&m, p, height, wh); c->h = wh; }
    }

    if (c->bw != 0) { XCB_AUX_ADD_PARAM(&m, p, border_width, 0); c->bw = 0; }

    /* ICCCM 4.1.5: Do not send synthetic ConfigureNotify if window borders or
     * size have changed */
    if (!(m & ((1<<XCB_AUX_INTERNAL_OFFSETOF(p, width))
               ||(1<<XCB_AUX_INTERNAL_OFFSETOF(p, height))
               ||(1<<XCB_AUX_INTERNAL_OFFSETOF(p, border_width)))))
        configure_event(c);

    if (m)
        configure(c->win, m, p);
}

/*
 * Evaluates client state and adjusts it according to environment.
 *
 * Does not alter stack position of window, only x/y/w/h/border_width.
 */
static void
arrange(client_t *c)
{
    printf("arrange: client %x (is_floating: %d)\n", c, c->is_floating);

    uint16_t m = 0;
    xcb_params_configure_window_t p;
    arrange_updated(c, m, &p);
}


static void
updategeom()
{
    /* Update NetWM-compliant docks */

    /* Adjust windows-occupied area */
    wx = sx;
    wy = sy;
    ww = sw;
    wh = sh;

    /* Rearrange windows */
    client_t *c;
    for (c = clients; c; c = c->next)
        arrange(c);
}

static void
intern_atoms(int count, xcb_atom_t atoms[], const char *atom_names[])
{
    xcb_intern_atom_cookie_t *c = xalloc(sizeof(xcb_intern_atom_cookie_t)*count);

    int i;
    for (i = 0; i < count; ++i)
        c[i] = xcb_intern_atom(conn, false, strlen(atom_names[i]), atom_names[i]);

    for (i = 0; i < count; ++i) {
        xcb_generic_error_t *err;
        xcb_intern_atom_reply_t *r
            = xcb_intern_atom_reply(conn, c[i], &err);
        if (!r)
            errx(1, "Unable to intern atom %s", atom_names[i]);
        atoms[i] = r->atom;
        free(r);
    }

    free(c);
}

static void
setup()
{
    /* init screen */
    sx = 0;
    sy = 0;
    sw = screen->width_in_pixels;
    sh = screen->height_in_pixels;

    updategeom();

    intern_atoms(sizeof(atom)/sizeof(atom[0]), atom, atom_names);

    /* FIXME: busy cursor is nice
    wa.cursor = cursor = XCreateFontCursor(dpy, XC_watch);
    XCB_AUX_ADD_PARAM(&masp, &params, cursor, ...)
    */

    /* expose NetWM support */
    xcb_void_cookie_t c
        = xcb_change_property_checked(conn, XCB_PROP_MODE_REPLACE,
                                      screen->root, atom[NetSupported],
                                      ATOM, 32, NetLast - NetFirst,
                                      atom + NetFirst);

    if (xcb_request_check(conn, c))
        errx(1, "Unable to register myself as NetWM-compliant WM.");

    /* select for events */
    uint32_t mask = 0;
    xcb_params_cw_t params;
    XCB_AUX_ADD_PARAM(&mask, &params, event_mask,
                      XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                      XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                      XCB_EVENT_MASK_ENTER_WINDOW |
                      XCB_EVENT_MASK_LEAVE_WINDOW |
                      XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                      XCB_EVENT_MASK_PROPERTY_CHANGE);

    xcb_void_cookie_t c2
        = xcb_aux_change_window_attributes_checked(conn, screen->root,
                                                   mask, (const void*)&params);

    xcb_generic_error_t *e = xcb_request_check(conn, c2);
    if (e)
        errx(1, "Unable to register event listener for root window: %d.",
             e->error_code);
}

static xcb_window_t
get_transient_for(xcb_window_t win)
{
    xcb_get_property_cookie_t cookie = xcb_get_wm_transient_for(conn, win);

    xcb_get_property_reply_t *transient_reply
        = xcb_get_property_reply(conn, cookie, NULL);

    if (!transient_reply)
        return XCB_NONE;

    xcb_window_t transient_for = XCB_NONE;
    if (xcb_get_wm_transient_for_from_reply(&transient_for, transient_reply))
        return transient_for;

    return XCB_NONE;
}

static client_t *
getclient(xcb_window_t w)
{
    client_t *c = clients;
    while (c && c->win != w)
        c = c->next;
    return c;
}

static void
attach(client_t *c)
{
    c->next = clients;
    clients = c;
}

static void
detach(client_t *c)
{
    client_t **tc = &clients;

    while (*tc && *tc != c)
        tc = &(*tc)->next;

    if (*tc)
        *tc = c->next;
}

static void
attachstack(client_t *c)
{
    debug("attachstack: %x (%x)\n", c, c->win);

    c->snext = stack;
    stack = c;
}

static void
detachstack(client_t *c)
{
    client_t **tc = &stack;

    while (*tc && *tc != c)
        tc = &(*tc)->snext;

    if (*tc)
        *tc = c->snext;
}

static bool
setclientstate(client_t *c, long state)
{
    long data[] = {state, XCB_NONE};

    xcb_void_cookie_t cookie =
        xcb_change_property_checked(
            conn, XCB_PROP_MODE_REPLACE, c->win, atom[WMState],
            atom[WMState], 32, 2, (const void*)data);

    xcb_generic_error_t *err = xcb_request_check(conn, cookie);
    if (err) {
        warnx("Unable to set client state (%d).", err->error_code);
        free(err);
        return false;
    }

    return true;
}

static bool
set_focus(uint8_t revert_to, xcb_window_t focus)
{
    debug("set_focus: win: %x\n", focus);

    xcb_void_cookie_t c
        = xcb_set_input_focus_checked(conn, revert_to, focus, XCB_CURRENT_TIME);
    xcb_generic_error_t *err = xcb_request_check(conn, c);
    if (err) {
        debug("set_focus: error in xcb_set_focus (%d)\n", err->error_code);
        /* Errors are ignored, as windows may disappear at any time */
        free(err);
        return false;
    }

    debug("set_focus: ok\n");
    return true;
}

static void
focus(client_t *c)
{
    xcb_window_t win;

    debug("focus: focusing %p (%x)\n", c, c ? c->win : -1);

    if (!c)
        c = stack;
    else {
        detachstack(c);
        attachstack(c);
    }

    if (c)
        win = c->win;
    else
        win = screen->root;

    set_focus(XCB_INPUT_FOCUS_POINTER_ROOT, win);
}

static client_t *
raise_transients_for(client_t *stack, client_t *c)
{
    debug("raise_transients: %x (%x) for: %x (%x)\n", stack, stack ? stack->win : 0, c, c->win);
    if (!stack)
        return c;

    debug(" -> recurse\n");
    client_t *last_raised = raise_transients_for(stack->snext, c);
    debug(" <- back\n");

    if (get_transient_for(stack->win) != c->win) {
        debug("window %x (%x) is not a transient for %x (%x)\n", stack, stack->win, c, c->win);
        return last_raised;
    }

    debug("window %x (%x) is a transient for %x (%x).\n", stack, stack->win, c, c->win);

    debug("raising %x (%x) above %x (%x)\n", stack, stack->win, last_raised, last_raised->win);

    uint16_t mask = 0;
    xcb_params_configure_window_t params;
    XCB_AUX_ADD_PARAM(&mask, &params, sibling, last_raised->win);
    XCB_AUX_ADD_PARAM(&mask, &params, stack_mode, XCB_STACK_MODE_ABOVE);
    configure(stack->win, mask, &params);
    focus(stack);

    debug("marking %x (%x) as last raised\n", stack, stack->win);

    return stack;
}

static void
raise_win(client_t *c)
{
    debug("raise: %x (%x)\n", c, c ? c->win : -1);

    uint16_t mask = 0;
    xcb_params_configure_window_t params;
    XCB_AUX_ADD_PARAM(&mask, &params, stack_mode, XCB_STACK_MODE_ABOVE);
    configure(c->win, mask, &params);
    focus(c);

    /* Walk through windows stack from bottom to top, raising the
     transients. Focus the last raised window */

    raise_transients_for(stack, c);
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
	char msg[30] = "";

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
			sprintf(msg, "%ld:destroy:%d", timestamp, window_id);
			printf("%ld:destroy:%d\n", timestamp, window_id);
			g_file_set_contents("/tmp/xpudctrl", msg, -1, NULL);

			//FIXME: better way to remove id from list
			tmp = g_list_remove_link(list, it);
		}
    }

}

GList *list = NULL;

static void
manage(xcb_window_t w)
{
    debug("manage: win %x\n", w);

	printf("manage: win %x\n", w);

    client_t *c = xalloc(sizeof(client_t));
    c->win = w;

//////////////////////////////////////////////////////
	time_t timestamp;
	time(&timestamp);
	char msg[30] = "";
	list = g_list_append(list, g_strdup_printf("0"));
///////////////////////////////////////////////////////


    xcb_window_t transient_for = get_transient_for(w);
    client_t *transient_for_client = getclient(transient_for);
    debug(" transient_for: %x (%x)\n", transient_for_client, transient_for);
    c->is_floating = transient_for != XCB_NONE && transient_for_client != NULL;

    xcb_get_geometry_reply_t *geom
        = xcb_get_geometry_reply(conn, xcb_get_geometry(conn, w), NULL);
    if (!geom)
        goto err;

    /* geometry */
    c->x = geom->x;
    c->y = geom->y;
    c->w = geom->width;
    c->h = geom->height;
    c->bw = c->oldbw = geom->border_width;

    free(geom);

    arrange(c);

    {
        uint32_t mask = 0;
        xcb_params_cw_t params;
        XCB_AUX_ADD_PARAM(&mask, &params, event_mask,
                          XCB_EVENT_MASK_ENTER_WINDOW |
                          XCB_EVENT_MASK_FOCUS_CHANGE |
                          XCB_EVENT_MASK_PROPERTY_CHANGE |
                          XCB_EVENT_MASK_STRUCTURE_NOTIFY);

        xcb_void_cookie_t c
            = xcb_aux_change_window_attributes_checked(conn, w, mask, &params);

        if (xcb_request_check(conn, c)) {
            warnx("Unable to select events for window.");
            goto err;
        }
    }

    attach(c);

    debug("manage: attaching %x to a stack\n", c->win);
    attachstack(c);

    if (xcb_request_check(conn, xcb_map_window_checked(conn, w))) {
        warnx("manage: unable to map window.");
        goto err;
    }

// xpud hack goes from here
//if (! c->is_floating) 
//{
	printf("manage %d\n", c->win);
	sprintf(msg, "%ld:map:%d", timestamp, c->win);
	printf("\t%ld:map:%d\n", timestamp, c->win);
	g_file_set_contents("/tmp/xpudctrl", msg, -1, NULL);

	printf("\tadd %d to glist\n", c->win);
	list = g_list_append(list, g_strdup_printf("%d", c->win));
//}

    raise_win(c);

    debug("manage: win: %x, state: WM_STATE_NORMAL\n", c->win);
    if (!setclientstate(c, XCB_WM_STATE_NORMAL))
        goto err;

    return;
err:
    warn("manage: Error while trying to manage window %x", c->win);
    detach(c);
    if (stack == c) {
        detachstack(c);
        focus(NULL);
    } else
        detachstack(c);
    free(c);
}

static void
unmanage(client_t *c)
{
    debug("unmanage: %x (%x)\n", c, c ? c->win : -1);
	printf("unmanage: %x (%x)\n", c, c ? c->win : -1);

    xcb_request_check(conn, xcb_grab_server_checked(conn));

    if (c->bw != c->oldbw) {
        uint16_t mask = 0;
        xcb_params_configure_window_t params;
        XCB_AUX_ADD_PARAM(&mask, &params, border_width, c->oldbw);
        configure(c->win, mask, &params);
    }

    detach(c);
    if (stack == c) {
        detachstack(c);
        focus(NULL);
    }
    else
        detachstack(c);

    setclientstate(c, XCB_WM_STATE_WITHDRAWN);
    free(c);

    xcb_request_check(conn, xcb_ungrab_server(conn));
}

static void
scan()
{
    debug("scan\n");

    xcb_query_tree_cookie_t c = xcb_query_tree(conn, screen->root);

    xcb_generic_error_t *err;
    xcb_query_tree_reply_t *tree = xcb_query_tree_reply(conn, c, &err);
    if (!tree)
        errx(1, "Unable to query windows hierarchy.");

    int len = xcb_query_tree_children_length(tree);
    xcb_window_t *children = xcb_query_tree_children(tree);

    xcb_get_window_attributes_cookie_t *cookies
        = xalloc(sizeof(xcb_get_window_attributes_cookie_t) * len);
    xcb_get_property_cookie_t *transient_cookies
        = xalloc(sizeof(xcb_get_property_cookie_t) * len);
    xcb_get_property_cookie_t *hints_cookies
        = xalloc(sizeof(xcb_get_property_cookie_t) * len);

    int i;
    for (i = 0; i < len; ++i) {
        cookies[i] = xcb_get_window_attributes(conn, children[i]);
        transient_cookies[i] = xcb_get_wm_transient_for(conn, children[i]);
        hints_cookies[i] = xcb_get_wm_hints(conn, children[i]);
    }

    int ntransients = 0;
    xcb_window_t *transients = xalloc(len * sizeof(xcb_window_t));

    /* Non-transient */
    for (i = 0; i < len; ++i) {
        xcb_get_window_attributes_reply_t *info
            = xcb_get_window_attributes_reply(conn, cookies[i], NULL);
        xcb_get_property_reply_t *transient_reply
            = xcb_get_property_reply(conn, transient_cookies[i], NULL);
        xcb_get_property_reply_t *hints_reply
            = xcb_get_property_reply(conn, hints_cookies[i], NULL);

        debug(" %x: info (%x), transient (%x), hints (%x)\n",
              children[i], info, transient_reply, hints_reply);

        /* Skip windows which can't be queried about */
        if (!info) {
            free(transient_reply);
            free(hints_reply);
            continue;
        }

        debug("  override_redirect: %d\n", !!info->override_redirect);

        /* Skip override-redirect windows */
        if (info->override_redirect) {
            free(info);
            free(transient_reply);
            free(hints_reply);
            continue;
        }

        debug("  map_state: %d\n", info->map_state);

        /* Skip non-viewable windows */
        if (info->map_state != XCB_MAP_STATE_VIEWABLE) {
            free(info);
            free(transient_reply);
            free(hints_reply);
            continue;
        }

        /* Delay transient-for windows for a second loop */
        xcb_window_t transient_for;
        if (xcb_get_wm_transient_for_from_reply(&transient_for, transient_reply)) {
            transients[ntransients++] = children[i];
            free(info);
            free(transient_reply);
            free(hints_reply);
            continue;
        }

        manage(children[i]);

        free(info);
        free(transient_reply);
        free(hints_reply);
    }

    /* transient */
    for (i = 0; i < ntransients; ++i)
        manage(transients[i]);

    free(tree);
    free(transients);
    free(cookies);
    free(transient_cookies);
    free(hints_cookies);

    focus(stack);
}

static int
configurerequest(void *p, xcb_connection_t *conn, xcb_configure_request_event_t *e)
{
    client_t *c = getclient(e->window);

    if (c) {
        uint16_t m = 0;
        xcb_params_configure_window_t p;
        /* Adjust geometry */
        if (e->value_mask & XCB_CONFIG_WINDOW_X) {
            XCB_AUX_ADD_PARAM(&m, &p, x, e->x);
            c->x = e->x;
        }
        if (e->value_mask & XCB_CONFIG_WINDOW_Y) {
            XCB_AUX_ADD_PARAM(&m, &p, y, e->y);
            c->y = e->y;
        }
        if (e->value_mask & XCB_CONFIG_WINDOW_WIDTH) {
            XCB_AUX_ADD_PARAM(&m, &p, width, e->width);
            c->w = e->width;
        }
        if (e->value_mask & XCB_CONFIG_WINDOW_HEIGHT) {
            XCB_AUX_ADD_PARAM(&m, &p, height, e->height);
            c->h = e->height;
        }
        if (e->value_mask & XCB_CONFIG_WINDOW_BORDER_WIDTH) {
            XCB_AUX_ADD_PARAM(&m, &p, border_width, e->border_width);
            c->bw = e->border_width;
        }

        arrange_updated(c, m, &p);

        /* Respects only XRaiseWindow */
        if (e->value_mask & XCB_CONFIG_WINDOW_STACK_MODE
            && (!(e->value_mask & XCB_CONFIG_WINDOW_SIBLING)
                || e->sibling == XCB_NONE))
            raise_win(c);
    } else {
        /* Not our business, just pass it through */

        /* Note: e->value_mask is passed as is to request */
        xcb_params_configure_window_t params;
        params.x = e->x;
        params.y = e->y;
        params.width = e->width;
        params.height = e->height;
        params.border_width = e->border_width;
        params.sibling = e->sibling;
        params.stack_mode = e->stack_mode;

        configure(e->window, e->value_mask, &params);
    }
    return 0;
}

static int
configurenotify(void *p, xcb_connection_t *conn, xcb_configure_notify_event_t *e)
{
    if (e->window == screen->root) {
        if (e->width != sw || e->height != sh) {
            sw = e->width;
            sh = e->height;
            updategeom();
        }
    }
    return 0;
}

static int
destroynotify(void *p, xcb_connection_t *conn, xcb_destroy_notify_event_t *e)
{
    client_t *c = getclient(e->window);
    if(c)
        unmanage(c);
    return 0;
}

static int
focusin(void *p, xcb_connection_t *conn, xcb_focus_in_event_t *e)
{
    debug("focusin: %x\n", e->event);
    /* there are some broken focus acquiring clients */
    if (stack && e->event != stack->win) {
        debug("focusin: setting focus back to top of stack: %x\n", stack->win);
        set_focus(XCB_INPUT_FOCUS_POINTER_ROOT, stack->win);
    }
    return 0;
}

static int
maprequest(void *p, xcb_connection_t *conn, xcb_map_request_event_t *e)
{
    xcb_get_window_attributes_cookie_t c
        = xcb_get_window_attributes(conn, e->window);
    xcb_get_window_attributes_reply_t *i
        = xcb_get_window_attributes_reply(conn, c, NULL);

    if (i && !i->override_redirect)
        if (!getclient(e->window))
            manage(e->window);

    free(i);
    return 0;
}

static int
mapnotify(void *p, xcb_connection_t *conn, xcb_map_notify_event_t *e)
{
    debug("mapnotify: win: %x\n", e->window);
    /* If newly mapped window is at top of stack, set the focus. It can't be
     * done at manage() as window is not visible yet there */
    if (stack && e->window == stack->win) {
        debug("mapnotify: focusing %x\n", e->window);
        set_focus(XCB_INPUT_FOCUS_POINTER_ROOT, e->window);
    } else {
        debug("mapnotify: not focusing %x.\n", stack ? stack->win : -1);
    }

    return 0;
}

static void
check_refloat(client_t *c)
{
    xcb_get_property_cookie_t cookie = xcb_get_wm_transient_for(conn, c->win);

    xcb_get_property_reply_t* transient_reply
        = xcb_get_property_reply(conn, cookie, NULL);

    xcb_window_t transient_for;
    if (xcb_get_wm_transient_for_from_reply(&transient_for, transient_reply)) {
        bool oldisfloating = c->is_floating;
        c->is_floating = getclient(transient_for) != NULL;
        if (c->is_floating != oldisfloating)
            arrange(c);
    }
}

static int
propertynotify(void *p, xcb_connection_t *conn, xcb_property_notify_event_t *e)
{
    client_t *c;

    if ((e->window == screen->root) && (e->atom == WM_NAME))
        return 0; /* ignore */
    if (e->state == XCB_PROPERTY_DELETE)
        return 0; /* ignore */
    if ((c = getclient(e->window))) {
        if (e->atom == WM_TRANSIENT_FOR)
            check_refloat(c);
    }
    return 0;
}

static int
unmapnotify(void *p, xcb_connection_t *conn, xcb_unmap_notify_event_t *e)
{
    client_t *c = getclient(e->window);
    if (c)
        unmanage(c);
    return 0;
}

static void
run()
{
    xcb_event_handlers_t eh;
    memset(&eh, 0, sizeof(eh)); /* Not necessary with xcb-util > 0.3.4 */
    xcb_event_handlers_init(conn, &eh);

    xcb_event_set_configure_request_handler(&eh, configurerequest, NULL);
    xcb_event_set_configure_notify_handler(&eh, configurenotify, NULL);
    xcb_event_set_destroy_notify_handler(&eh, destroynotify, NULL);
    xcb_event_set_focus_in_handler(&eh, focusin, NULL);
    xcb_event_set_map_request_handler(&eh, maprequest, NULL);
    xcb_event_set_map_notify_handler(&eh, mapnotify, NULL);
    xcb_event_set_property_notify_handler(&eh, propertynotify, NULL);
    xcb_event_set_unmap_notify_handler(&eh, unmapnotify, NULL);

    xcb_generic_event_t *e;
    while ((e = xcb_wait_for_event(conn))) {
		check_list(conn, list);
        xcb_event_handle(&eh, e);
        free(e);
    }
}

static void
cleanup()
{
    debug("cleanup: starting");
    while (stack)
        unmanage(stack);
    /* FIXME */
    //XFreeCursor(dpy, cursor);

    set_focus(XCB_INPUT_FOCUS_POINTER_ROOT, XCB_INPUT_FOCUS_POINTER_ROOT);
}

int
main(int argc, char *argv[])
{
    if (argc == 2 && !strcmp("-v", argv[1]))
        errx(0, "uuwm-"VERSION", © 2006-2009 uuwm engineers, see LICENSE for details");
    else if (argc != 1)
        errx(1, "usage: uuwm [-v]");

    int default_screen;
    conn = xcb_connect(NULL, &default_screen);
    if (xcb_connection_has_error(conn))
        errx(1, "uuwm: cannot open display %s", getenv("DISPLAY") ? getenv("DISPLAY") : "<NULL>");
    if (!(screen = xcb_aux_get_screen(conn, default_screen)))
        errx(1, "uuwm: cannot obtain default screen");

    checkotherwm();
    setup();
    scan();
    run();
    cleanup();

    xcb_disconnect(conn);
    return 0;
}
