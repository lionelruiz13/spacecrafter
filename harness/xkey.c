/* xkey - hold a REAL X11 key down on the spacecrafter window for N ms.
 *
 * Why this exists (INTENT 11.108, task F4 check (i)): the interactive
 * navigation ramps (arrow keys -> Core::turnLeft/Right/Up/Down -> vzm ->
 * Core::updateMove) have NO command-interface entry, so the only honest way to
 * establish what they reach is to press the key and look (§11.36's precedent:
 * "command-unreachable" was only established by looking). XTEST fake key
 * events enter the X server as ordinary core-keyboard events, so SDL delivers
 * them to the app exactly like a human keystroke and the whole chain runs.
 *
 * The window is resolved exactly as xclick does it - the CLIENT window of the
 * expected size, not the WM frame (§11.106(a)'s recorded trap) - and focus is
 * set before the press, because a key event without focus goes nowhere.
 *
 * usage:  xkey <win-name-prefix> <keysym-name> <hold-ms> [<W>x<H>]
 *         e.g. xkey spacecrafter Left 1500 1024x1024
 * Exit 0 on success, 2 when no matching window is found, 3 on a bad keysym.
 *
 * Build:  gcc -O1 -o xkey xkey.c -lX11 /usr/lib/x86_64-linux-gnu/libXtst.so.6
 * (XTest.h is not installed on this host; the entry point is declared here,
 *  its signature is the stable libXtst ABI - same convention as xclick.c.)
 */
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern int XTestFakeKeyEvent(Display *, unsigned int keycode, int is_press, unsigned long delay);

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
    if (argc < 4) {
        fprintf(stderr, "usage: %s <win-name-prefix> <keysym-name> <hold-ms> [<W>x<H>]\n", argv[0]);
        return 1;
    }
    int want_w = 0, want_h = 0;
    if (argc > 4 && sscanf(argv[4], "%dx%d", &want_w, &want_h) != 2) {
        fprintf(stderr, "xkey: bad size '%s'\n", argv[4]);
        return 1;
    }
    Display *d = XOpenDisplay(NULL);
    if (!d) {
        fprintf(stderr, "xkey: cannot open display\n");
        return 1;
    }
    KeySym ks = XStringToKeysym(argv[2]);
    if (ks == NoSymbol) {
        fprintf(stderr, "xkey: unknown keysym '%s'\n", argv[2]);
        XCloseDisplay(d);
        return 3;
    }
    KeyCode kc = XKeysymToKeycode(d, ks);
    if (!kc) {
        fprintf(stderr, "xkey: keysym '%s' has no keycode on this layout\n", argv[2]);
        XCloseDisplay(d);
        return 3;
    }
    Window root = DefaultRootWindow(d);
    Window win = find_named(d, root, argv[1]);
    if (!win) {
        fprintf(stderr, "xkey: window '%s*' not found\n", argv[1]);
        XCloseDisplay(d);
        return 2;
    }
    if (want_w) {
        Window client = find_sized(d, win, want_w, want_h);
        if (!client) {
            fprintf(stderr, "xkey: no %dx%d window inside '%s*'\n", want_w, want_h, argv[1]);
            XCloseDisplay(d);
            return 2;
        }
        win = client;
    }
    XRaiseWindow(d, win);
    XSetInputFocus(d, win, RevertToParent, CurrentTime);
    XFlush(d);
    usleep(200000);
    XTestFakeKeyEvent(d, kc, 1, 0);
    XFlush(d);
    usleep((useconds_t)atoi(argv[3]) * 1000);
    XTestFakeKeyEvent(d, kc, 0, 0);
    XFlush(d);
    usleep(150000);
    printf("held %s (keycode %u) for %s ms on window 0x%lx\n", argv[2], kc, argv[3], (unsigned long)win);
    XCloseDisplay(d);
    return 0;
}
