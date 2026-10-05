#include "msgloop.h"

#include <cstdio>

#if defined(OPENW3D_SDL3)
#include <SDL3/SDL.h>

int main()
{
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Pumping must return with an empty queue and must leave queued input for
    // its consumers, including when called repeatedly during loading screens.
    Windows_Message_Handler();

    SDL_VirtualJoystickDesc description{};
    SDL_INIT_INTERFACE(&description);
    description.naxes = 1;
    const SDL_JoystickID id = SDL_AttachVirtualJoystick(&description);
    SDL_Joystick *joystick = id ? SDL_OpenJoystick(id) : nullptr;
    if (!joystick || !SDL_SetJoystickVirtualAxis(joystick, 0, 16384)) {
        std::fprintf(stderr, "Virtual joystick setup failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Virtual input is applied only when SDL updates device state. This catches
    // a missing pump, even on headless CI machines without physical devices.
    Windows_Message_Handler();
    const bool input_updated = SDL_GetJoystickAxis(joystick, 0) == 16384;
    SDL_CloseJoystick(joystick);
    SDL_DetachVirtualJoystick(id);
    if (!input_updated) {
        std::fprintf(stderr, "The message pump did not update SDL input state.\n");
        SDL_Quit();
        return 1;
    }

    SDL_Event event{};
    event.type = SDL_EVENT_USER;
    for (int code = 1; code <= 2; ++code) {
        event.user.code = code;
        if (!SDL_PushEvent(&event)) {
            std::fprintf(stderr, "SDL_PushEvent failed: %s\n", SDL_GetError());
            SDL_Quit();
            return 1;
        }
    }
    Windows_Message_Handler();
    Windows_Message_Handler();

    SDL_Event queued[2]{};
    const int count = SDL_PeepEvents(queued, 2, SDL_GETEVENT, SDL_EVENT_USER, SDL_EVENT_USER);
    SDL_Quit();
    if (count != 2 || queued[0].user.code != 1 || queued[1].user.code != 2) {
        std::fprintf(stderr, "The message pump consumed or reordered SDL events.\n");
        return 1;
    }
    return 0;
}

#elif defined(OPENW3D_WIN32)
namespace {
int intercepted = 0;
int dispatched = 0;

bool Intercept_Message(MSG &msg)
{
    if (msg.message == WM_APP) {
        ++intercepted;
        return true;
    }
    return false;
}

LRESULT CALLBACK Window_Procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_APP || message == WM_APP + 1) {
        ++dispatched;
        return 0;
    }
    return DefWindowProcA(window, message, wparam, lparam);
}
}

int main()
{
    WNDCLASSA window_class{};
    window_class.lpfnWndProc = Window_Procedure;
    window_class.hInstance = GetModuleHandleA(nullptr);
    window_class.lpszClassName = "OpenW3DMessageLoopTest";
    if (!RegisterClassA(&window_class)) {
        return 1;
    }
    HWND window = CreateWindowExA(0, window_class.lpszClassName, "", 0,
        0, 0, 0, 0, HWND_MESSAGE, nullptr, window_class.hInstance, nullptr);
    if (!window) {
        return 1;
    }

    Message_Intercept_Handler = Intercept_Message;
    const bool posted = PostMessageA(window, WM_APP, 0, 0) && PostMessageA(window, WM_APP + 1, 0, 0);
    Windows_Message_Handler();
    Windows_Message_Handler();
    Message_Intercept_Handler = nullptr;
    DestroyWindow(window);
    UnregisterClassA(window_class.lpszClassName, window_class.hInstance);
    if (!posted || intercepted != 1 || dispatched != 1) {
        std::fprintf(stderr, "Win32 message interception or dispatch failed.\n");
        return 1;
    }
    return 0;
}
#endif
