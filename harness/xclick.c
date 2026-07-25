/* xclick - deliver a REAL X11 pointer click to the spacecrafter window.
 *
 * Why this exists (INTENT 11.106, task F2 check 2): the pointer/click channel
 * of selection (UI::handleClic -> Core::findAndSelect, ui.cpp:479) has NO
 * command-interface entry point - it is reachable only from an SDL mouse
 * event.  Driving it from the TCP channel is therefore impossible, and
 * asserting the pick FUNCTION directly would test one layer below the
 * terminal observable.  XTEST fake events enter the X server as ordinary
 * core-pointer events, so SDL delivers them to the app exactly like a human
 * click: the whole chain (SDL event -> UI::handleClic -> Core::findAndSelect
 * -> cleverFind / the new-path pick -> Core::selectObject) is exercised.
 *
 * THE CLIENT WINDOW, not the frame: a reparenting WM gives the frame the
 * app's WM_NAME too, and the frame is BIGGER than the client (measured on
 * this host: frame 1074x1111 for a 1024x1024 client).  Clicking in frame
 * coordinates silently shifts every position by the border/titlebar - a
 * ~44 px y error that a click at the middle of a large disc still "passes".
 * So the caller passes the expected client size (the app's own
 * `Swapchain : (w, h)` log line) and this tool picks the window of exactly
 * that size inside the named subtree.
 *
 * usage:  xclick <win-name-prefix> <x> <y> [<W>x<H>]   (client-local pixels)
 *         xclick <win-name-prefix> --geom  [<W>x<H>]   (print "x y w h")
 * Exit 0 on success, 2 when no matching window is found.
 *
 * Build:  gcc -O1 -o xclick xclick.c -lX11 /usr/lib/x86_64-linux-gnu/libXtst.so.6
 * (XTest.h is not installed on this host, so the two entry points are
 *  declared here; their signatures are the stable libXtst ABI.)
 */
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern int XTestFakeMotionEvent(Display *, int screen, int x, int y, unsigned long delay);
extern int XTestFakeButtonEvent(Display *, unsigned int button, int is_press, unsigned long delay);

static Window find_named(Display *d, Window w, const char *prefix)
{
    char *name = NULL;
    Window found = 0;
    if (XFetchName(d, w, &name) && name) {
        if (strncmp(name, prefix, strlen(prefix)) == 0)
            found = w;
        XFree(name);
    }
    if (found)
        return found;
    Window root, parent, *children = NULL;
    unsigned int n = 0;
    if (XQueryTree(d, w, &root, &parent, &children, &n)) {
        for (unsigned int i = 0; i < n && !found; ++i)
            found = find_named(d, children[i], prefix);
        if (children)
            XFree(children);
    }
    return found;
}

/* First viewable window of exactly (want_w, want_h) in this subtree. */
static Window find_sized(Display *d, Window w, int want_w, int want_h)
{
    XWindowAttributes a;
    if (XGetWindowAttributes(d, w, &a) && a.map_state == IsViewable
        && a.width == want_w && a.height == want_h)
        return w;
    Window root, parent, *children = NULL;
    unsigned int n = 0;
    Window found = 0;
    if (XQueryTree(d, w, &root, &parent, &children, &n)) {
        for (unsigned int i = 0; i < n && !found; ++i)
            found = find_sized(d, children[i], want_w, want_h);
        if (children)
            XFree(children);
    }
    return found;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "usage: %s <win-name-prefix> <x> <y> [<W>x<H>] | <win-name-prefix> --geom [<W>x<H>]\n",
                argv[0]);
        return 1;
    }
    int geom = (strcmp(argv[2], "--geom") == 0);
    const char *sizearg = geom ? (argc > 3 ? argv[3] : NULL) : (argc > 4 ? argv[4] : NULL);
    int want_w = 0, want_h = 0;
    if (sizearg && sscanf(sizearg, "%dx%d", &want_w, &want_h) != 2) {
        fprintf(stderr, "xclick: bad size '%s'\n", sizearg);
        return 1;
    }
    Display *d = XOpenDisplay(NULL);
    if (!d) {
        fprintf(stderr, "xclick: cannot open display\n");
        return 1;
    }
    Window root = DefaultRootWindow(d);
    Window win = find_named(d, root, argv[1]);
    if (!win) {
        fprintf(stderr, "xclick: window '%s*' not found\n", argv[1]);
        XCloseDisplay(d);
        return 2;
    }
    if (want_w) {
        Window client = find_sized(d, win, want_w, want_h);
        if (!client) {
            fprintf(stderr, "xclick: no %dx%d window inside '%s*'\n", want_w, want_h, argv[1]);
            XCloseDisplay(d);
            return 2;
        }
        win = client;
    }
    int rx = 0, ry = 0;
    Window child;
    if (geom) {
        XWindowAttributes a;
        XGetWindowAttributes(d, win, &a);
        XTranslateCoordinates(d, win, root, 0, 0, &rx, &ry, &child);
        printf("%d %d %d %d\n", rx, ry, a.width, a.height);
        XCloseDisplay(d);
        return 0;
    }
    int x = atoi(argv[2]), y = atoi(argv[3]);
    XTranslateCoordinates(d, win, root, x, y, &rx, &ry, &child);
    /* Raise + focus so the click is not intercepted by a stacking window. */
    XRaiseWindow(d, win);
    XSetInputFocus(d, win, RevertToParent, CurrentTime);
    XFlush(d);
    usleep(150000);
    XTestFakeMotionEvent(d, DefaultScreen(d), rx, ry, 0);
    XFlush(d);
    usleep(150000);
    XTestFakeButtonEvent(d, 1, 1, 0);
    XFlush(d);
    usleep(80000);
    XTestFakeButtonEvent(d, 1, 0, 0);
    XFlush(d);
    usleep(80000);
    printf("clicked client-local (%d,%d) -> root (%d,%d)\n", x, y, rx, ry);
    XCloseDisplay(d);
    return 0;
}
