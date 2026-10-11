#include <SDL2/SDL.h>
#include <stdio.h>
/* print every scancode with its SDL name and the reverse lookup of that name */
int main(void) {
    SDL_version v; SDL_GetVersion(&v);
    printf("# SDL %d.%d.%d SDL_NUM_SCANCODES=%d\n", v.major, v.minor, v.patch, SDL_NUM_SCANCODES);
    for (int s = 0; s < SDL_NUM_SCANCODES; ++s) {
        const char *n = SDL_GetScancodeName((SDL_Scancode)s);
        if (!n || !*n) continue;
        printf("%d\t%s\t%d\n", s, n, (int)SDL_GetScancodeFromName(n));
    }
    return 0;
}
