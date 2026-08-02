/* xdrag - hold the left button and MOVE the pointer across the spacecrafter
 * window: a real mouse-drag look-around.
 *
 * Why this exists (INTENT §11.133, task F25): `Core::dragView` is the ONE other
 * caller of `Camera::lookRel` (B37: mouse-drag look-around, unscriptable - there
 * is no command for it on any of the four channels), so making lookRel the exact
 * counterpart of `Navigator::updateMove` changes what a drag does on the drawn
 * path. That change has to be measured on the live app rather than argued, and
 * a drag is the only way to reach it: `UI::handleClic` sets `is_dragging` on the
 * left button DOWN and `UI::handleMove` calls `Core::dragView` for every motion
 * event after it, so press + motion + release is the whole chain. XTEST fake
 * events enter the X server as ordinary core-pointer events, exactly like a
 * human's - the same argument xclick.c and xkey.c are built on.
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
 * usage:  xdrag <win-name-prefix> <x1> <y1> <x2> <y2> [<W>x<H>] [steps]
 *         (client-local pixels; left button held from (x1,y1) to (x2,y2))
 * Exit 0 on success, 2 when no matching window is found.
 *
 * Build:  gcc -O1 -o xdrag xdrag.c -lX11 /usr/lib/x86_64-linux-gnu/libXtst.so.6
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
    if (argc < 5) {
        fprintf(stderr, "usage: %s <win-name-prefix> <x1> <y1> <x2> <y2> [<W>x<H>] [steps]\n", argv[0]);
        return 1;
    }
    int want_w = 0, want_h = 0;
    if (argc > 6 && sscanf(argv[6], "%dx%d", &want_w, &want_h) != 2) {
        fprintf(stderr, "xdrag: bad size '%s'\n", argv[6]);
        return 1;
    }
    int steps = (argc > 7) ? atoi(argv[7]) : 8;
    if (steps < 1) steps = 1;
    Display *d = XOpenDisplay(NULL);
    if (!d) { fprintf(stderr, "xdrag: cannot open display\n"); return 1; }
    Window root = DefaultRootWindow(d);
    Window win = find_named(d, root, argv[1]);
    if (!win) { fprintf(stderr, "xdrag: window '%s*' not found\n", argv[1]); XCloseDisplay(d); return 2; }
    if (want_w) {
        Window client = find_sized(d, win, want_w, want_h);
        if (!client) { fprintf(stderr, "xdrag: no %dx%d window inside '%s*'\n", want_w, want_h, argv[1]); XCloseDisplay(d); return 2; }
        win = client;
    }
    int x1 = atoi(argv[2]), y1 = atoi(argv[3]), x2 = atoi(argv[4]), y2 = atoi(argv[5]);
    Window child;
    int rx = 0, ry = 0;
    XRaiseWindow(d, win);
    XSetInputFocus(d, win, RevertToParent, CurrentTime);
    XFlush(d);
    usleep(200000);
    XTranslateCoordinates(d, win, root, x1, y1, &rx, &ry, &child);
    XTestFakeMotionEvent(d, DefaultScreen(d), rx, ry, 0);
    XFlush(d);
    usleep(200000);
    XTestFakeButtonEvent(d, 1, 1, 0);   /* left button DOWN -> UI::is_dragging */
    XFlush(d);
    usleep(200000);
    for (int i = 1; i <= steps; ++i) {
        int x = x1 + (x2 - x1) * i / steps;
        int y = y1 + (y2 - y1) * i / steps;
        XTranslateCoordinates(d, win, root, x, y, &rx, &ry, &child);
        XTestFakeMotionEvent(d, DefaultScreen(d), rx, ry, 0);
        XFlush(d);
        usleep(60000);
        /* Report where the pointer ACTUALLY is: a fake motion to a coordinate
         * outside the visible root goes nowhere, and then "nothing moved" is a
         * dead instrument rather than a finding. */
        {
            Window r2, c2; int qrx, qry, qwx, qwy; unsigned int mask;
            if (XQueryPointer(d, root, &r2, &c2, &qrx, &qry, &qwx, &qwy, &mask))
                fprintf(stderr, "  step %d: asked root (%d,%d) -> pointer at (%d,%d) mask 0x%x\n",
                        i, rx, ry, qrx, qry, mask);
        }
    }
    XTestFakeButtonEvent(d, 1, 0, 0);
    XFlush(d);
    usleep(200000);
    printf("dragged client-local (%d,%d)->(%d,%d) in %d steps on window 0x%lx\n",
           x1, y1, x2, y2, steps, (unsigned long)win);
    XCloseDisplay(d);
    return 0;
}
