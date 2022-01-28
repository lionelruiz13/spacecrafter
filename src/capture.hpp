#ifndef CAPTURE_HPP_
#define CAPTURE_HPP_

enum class Capture : int {
    FRAME_START,
    INIT_VULKAN,
    INIT_GENERAL,
    INIT_APPDRAW,
    INIT_SPLASH,
    INIT_FONT,
    INIT_CORE,
    INIT_UI,
    APP_INIT_BEGIN,
    APP_INIT_END,
    APP_MAINLOOP_START,
    APP_MAINLOOP_END,
    SCRIPT_UPDATE,
    NETWORK_UPDATE,
    UI_UPDATE,
    MEDIA_AUDIO_UPDATE,
    MEDIA_IMAGE_UPDATE,
    MEDIA_PLAYER_UPDATE,
    FADER_UPDATE,
    EXECUTOR_UPDATE,
    FRAME_ACQUIRE,
    DRAW_RESOURCE_READY,
    EXECUTOR_DRAW,
    UI_DRAW,
    MEDIA_DRAW,
    FOREGROUND_DRAW,
    ASYNC_FRAME_SUBMIT,
};

#define CAPTURE_FLAG_NAMES {"FRAME_START", "INIT_VULKAN", "INIT_GENERAL", "INIT_APPDRAW", "INIT_SPLASH", "INIT_FONT", "INIT_CORE", "INIT_UI", "APP_INIT_BEGIN", "APP_INIT_END", "APP_MAINLOOP_START", "APP_MAINLOOP_END", "SCRIPT_UPDATE", "NETWORK_UPDATE", "UI_UPDATE", "MEDIA_AUDIO_UPDATE", "MEDIA_IMAGE_UPDATE", "MEDIA_PLAYER_UPDATE", "FADER_UPDATE", "EXECUTOR_UPDATE", "FRAME_ACQUIRE", "DRAW_RESOURCE_READY", "EXECUTOR_DRAW", "UI_DRAW", "MEDIA_DRAW", "FOREGROUND_DRAW", "ASYNC_FRAME_SUBMIT"}

#endif /* end of include guard: CAPTURE_HPP_ */
