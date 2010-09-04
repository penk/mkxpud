enum { _NET_SUPPORTED = 0,
        _NET_SUPPORTING_WM_CHECK,
        _NET_WM_NAME,
        _NET_WM_STATE_FULLSCREEN,
        _NET_WM_STATE,
        _NET_WM_WINDOW_TYPE,
        _NET_WM_WINDOW_TYPE_DOCK,
        _NET_WM_WINDOW_TYPE_DIALOG,
        _NET_WM_WINDOW_TYPE_UTILITY,
        _NET_WM_WINDOW_TYPE_TOOLBAR,
        _NET_WM_WINDOW_TYPE_SPLASH,
        _NET_WM_DESKTOP,
        _NET_WM_STRUT_PARTIAL,
		_NET_WM_PING,
		_NET_CLOSE_WINDOW,
		_NET_WM_STATE_MODAL, 
		_NET_WM_STATE_STICKY, 
		_NET_WM_STATE_MAXIMIZED_VERT, 
		_NET_WM_STATE_MAXIMIZED_HORZ, 
		_NET_WM_STATE_SHADED, 
		_NET_WM_STATE_SKIP_TASKBAR, 
		_NET_WM_STATE_SKIP_PAGER, 
		_NET_WM_STATE_HIDDEN, 
		_NET_WM_STATE_ABOVE, 
		_NET_WM_STATE_BELOW, 
		_NET_WM_STATE_DEMANDS_ATTENTION, 
        WM_PROTOCOLS,
        WM_DELETE_WINDOW,
        UTF8_STRING,
        WM_STATE,
        WM_CLIENT_LEADER,
        _NET_CURRENT_DESKTOP,
        _NET_ACTIVE_WINDOW,
        _NET_WORKAREA
};

#define ATOM_NUM 35

#define REQUEST_ATOM(name) atom_cookies[name] = xcb_intern_atom(conn, 0, strlen(#name), #name);

/* Setup NetWM atoms */
#define GET_ATOM(name) { \
                xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(conn, atom_cookies[name], NULL); \
                if (!reply) { \
                        printf("Could not get atom " #name "\n"); \
                        exit(-1); \
                } \
                atoms[name] = reply->atom; \
                free(reply); \
        }
