/* SDL2 stand-in. Win32, WGL, and XInput. Not a full SDL. */
#ifndef MIN_LIB_BUILD
#error "sdl_dumb.c is part of MIN_LIB_BUILD"
#endif

#include "minlib/minlib_internal.h"

#include <SDL.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <mmsystem.h>
#include <xinput.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WINDOWS 8
#define EVENT_QUEUE 256
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

typedef HGLRC(WINAPI *wgl_create_attribs_fn)(HDC hdc, HGLRC share, const int *attribs);
typedef BOOL(WINAPI *wgl_swap_interval_fn)(int interval);

struct SDL_Window {
    HWND hwnd;
    HDC hdc;
    HGLRC glrc;
    int w;
    int h;
    int fullscreen;
    int pixel_set;
    DWORD saved_style;
    RECT saved_rect;
};

struct SDL_Joystick {
    int user_index;
    int instance_id;
};

struct SDL_GameController {
    struct SDL_Joystick joy;
};

struct SDL_Haptic {
    int user_index;
};

struct SDL_mutex {
    CRITICAL_SECTION cs;
};

static char error_buf[512];
static int sdl_ready;
static int class_ready;
static int timer_hires;
static HINSTANCE instance;
static HCURSOR arrow_cursor;
static const wchar_t *class_name = L"OpenOMFMinLib";

static SDL_Window *windows[MAX_WINDOWS];
static SDL_Event events[EVENT_QUEUE];
static int ev_head;
static int ev_count;
static Uint8 keystate[SDL_NUM_SCANCODES];
static int cursor_shown = 1;
static int mouse_x;
static int mouse_y;
static int have_mouse;

static SDL_Scancode base_map[256];
static SDL_Scancode ext_map[256];
static int maps_ready;

static LARGE_INTEGER qpf;
static LARGE_INTEGER qpc0;
static int qpc_ready;

static int gl_red = 8;
static int gl_green = 8;
static int gl_blue = 8;
static int gl_alpha = 8;
static int gl_major = 3;
static int gl_minor = 3;
static int gl_profile = SDL_GL_CONTEXT_PROFILE_CORE;
static int gl_loaded;
static wgl_swap_interval_fn swap_interval_proc;

static int pads_ready;
static int pad_mask;
static Uint64 rumble_until[4];

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void sdl_set_error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(error_buf, sizeof(error_buf), fmt != NULL ? fmt : "", ap);
    va_end(ap);
}

const char *SDL_GetError(void) {
    return error_buf;
}

SDL_mutex *SDL_CreateMutex(void) {
    SDL_mutex *mutex = (SDL_mutex *)malloc(sizeof(SDL_mutex));
    if(mutex == NULL) {
        return NULL;
    }
    InitializeCriticalSection(&mutex->cs);
    return mutex;
}

void SDL_DestroyMutex(SDL_mutex *mutex) {
    if(mutex == NULL) {
        return;
    }
    DeleteCriticalSection(&mutex->cs);
    free(mutex);
}

int SDL_LockMutex(SDL_mutex *mutex) {
    if(mutex == NULL) {
        return -1;
    }
    EnterCriticalSection(&mutex->cs);
    return 0;
}

int SDL_UnlockMutex(SDL_mutex *mutex) {
    if(mutex == NULL) {
        return -1;
    }
    LeaveCriticalSection(&mutex->cs);
    return 0;
}

static wchar_t *utf8_to_wide(const char *text) {
    int n;
    wchar_t *out;
    if(text == NULL) {
        text = "";
    }
    n = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    if(n <= 0) {
        return NULL;
    }
    out = (wchar_t *)malloc((size_t)n * sizeof(wchar_t));
    if(out == NULL) {
        return NULL;
    }
    MultiByteToWideChar(CP_UTF8, 0, text, -1, out, n);
    return out;
}

static char *wide_to_utf8(const wchar_t *text) {
    int n;
    char *out;
    if(text == NULL) {
        text = L"";
    }
    n = WideCharToMultiByte(CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL);
    if(n <= 0) {
        return NULL;
    }
    out = (char *)malloc((size_t)n);
    if(out == NULL) {
        return NULL;
    }
    WideCharToMultiByte(CP_UTF8, 0, text, -1, out, n, NULL, NULL);
    return out;
}

static void qpc_init(void) {
    if(qpc_ready) {
        return;
    }
    QueryPerformanceFrequency(&qpf);
    QueryPerformanceCounter(&qpc0);
    if(qpf.QuadPart == 0) {
        qpf.QuadPart = 1;
    }
    qpc_ready = 1;
}

Uint64 SDL_GetTicks64(void) {
    LARGE_INTEGER now;
    qpc_init();
    QueryPerformanceCounter(&now);
    return (Uint64)((now.QuadPart - qpc0.QuadPart) * 1000 / qpf.QuadPart);
}

Uint64 SDL_GetPerformanceCounter(void) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (Uint64)now.QuadPart;
}

Uint64 SDL_GetPerformanceFrequency(void) {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return (Uint64)freq.QuadPart;
}

void SDL_Delay(Uint32 ms) {
    Sleep(ms);
}

static void push_event(const SDL_Event *event) {
    int tail;
    if(ev_count >= EVENT_QUEUE) {
        return;
    }
    tail = (ev_head + ev_count) % EVENT_QUEUE;
    events[tail] = *event;
    ev_count++;
}

static void fill_maps(void) {
    int i;
    const char *qwerty = "QWERTYUIOP";
    const char *asdf = "ASDFGHJKL";
    const char *zxcv = "ZXCVBNM";
    if(maps_ready) {
        return;
    }
    for(i = 0; qwerty[i] != 0; i++) {
        base_map[0x10 + i] = (SDL_Scancode)(SDL_SCANCODE_A + (qwerty[i] - 'A'));
    }
    for(i = 0; asdf[i] != 0; i++) {
        base_map[0x1E + i] = (SDL_Scancode)(SDL_SCANCODE_A + (asdf[i] - 'A'));
    }
    for(i = 0; zxcv[i] != 0; i++) {
        base_map[0x2C + i] = (SDL_Scancode)(SDL_SCANCODE_A + (zxcv[i] - 'A'));
    }
    for(i = 0; i < 9; i++) {
        base_map[0x02 + i] = (SDL_Scancode)(SDL_SCANCODE_1 + i);
    }
    base_map[0x0B] = SDL_SCANCODE_0;
    for(i = 0; i < 10; i++) {
        base_map[0x3B + i] = (SDL_Scancode)(SDL_SCANCODE_F1 + i);
    }
    base_map[0x57] = SDL_SCANCODE_F11;
    base_map[0x58] = SDL_SCANCODE_F12;
    base_map[0x01] = SDL_SCANCODE_ESCAPE;
    base_map[0x0C] = SDL_SCANCODE_MINUS;
    base_map[0x0D] = SDL_SCANCODE_EQUALS;
    base_map[0x0E] = SDL_SCANCODE_BACKSPACE;
    base_map[0x0F] = SDL_SCANCODE_TAB;
    base_map[0x1A] = SDL_SCANCODE_LEFTBRACKET;
    base_map[0x1B] = SDL_SCANCODE_RIGHTBRACKET;
    base_map[0x1C] = SDL_SCANCODE_RETURN;
    base_map[0x1D] = SDL_SCANCODE_LCTRL;
    base_map[0x27] = SDL_SCANCODE_SEMICOLON;
    base_map[0x28] = SDL_SCANCODE_APOSTROPHE;
    base_map[0x29] = SDL_SCANCODE_GRAVE;
    base_map[0x2A] = SDL_SCANCODE_LSHIFT;
    base_map[0x2B] = SDL_SCANCODE_BACKSLASH;
    base_map[0x33] = SDL_SCANCODE_COMMA;
    base_map[0x34] = SDL_SCANCODE_PERIOD;
    base_map[0x35] = SDL_SCANCODE_SLASH;
    base_map[0x36] = SDL_SCANCODE_RSHIFT;
    base_map[0x37] = SDL_SCANCODE_KP_MULTIPLY;
    base_map[0x38] = SDL_SCANCODE_LALT;
    base_map[0x39] = SDL_SCANCODE_SPACE;
    base_map[0x3A] = SDL_SCANCODE_CAPSLOCK;
    base_map[0x47] = SDL_SCANCODE_KP_7;
    base_map[0x48] = SDL_SCANCODE_KP_8;
    base_map[0x49] = SDL_SCANCODE_KP_9;
    base_map[0x4A] = SDL_SCANCODE_KP_MINUS;
    base_map[0x4B] = SDL_SCANCODE_KP_4;
    base_map[0x4C] = SDL_SCANCODE_KP_5;
    base_map[0x4D] = SDL_SCANCODE_KP_6;
    base_map[0x4E] = SDL_SCANCODE_KP_PLUS;
    base_map[0x4F] = SDL_SCANCODE_KP_1;
    base_map[0x50] = SDL_SCANCODE_KP_2;
    base_map[0x51] = SDL_SCANCODE_KP_3;
    base_map[0x52] = SDL_SCANCODE_KP_0;
    base_map[0x53] = SDL_SCANCODE_KP_PERIOD;

    ext_map[0x1C] = SDL_SCANCODE_KP_ENTER;
    ext_map[0x1D] = SDL_SCANCODE_RCTRL;
    ext_map[0x35] = SDL_SCANCODE_KP_DIVIDE;
    ext_map[0x38] = SDL_SCANCODE_RALT;
    ext_map[0x47] = SDL_SCANCODE_HOME;
    ext_map[0x48] = SDL_SCANCODE_UP;
    ext_map[0x49] = SDL_SCANCODE_PAGEUP;
    ext_map[0x4B] = SDL_SCANCODE_LEFT;
    ext_map[0x4D] = SDL_SCANCODE_RIGHT;
    ext_map[0x4F] = SDL_SCANCODE_END;
    ext_map[0x50] = SDL_SCANCODE_DOWN;
    ext_map[0x51] = SDL_SCANCODE_PAGEDOWN;
    ext_map[0x53] = SDL_SCANCODE_DELETE;
    ext_map[0x5B] = SDL_SCANCODE_LGUI;
    ext_map[0x5C] = SDL_SCANCODE_RGUI;
    maps_ready = 1;
}

static SDL_Scancode map_scancode(unsigned scan, int extended) {
    fill_maps();
    if(scan >= 256) {
        return SDL_SCANCODE_UNKNOWN;
    }
    if(extended && ext_map[scan] != SDL_SCANCODE_UNKNOWN) {
        return ext_map[scan];
    }
    return base_map[scan];
}

static SDL_Keycode scancode_to_keycode(SDL_Scancode sc) {
    if(sc == SDL_SCANCODE_RETURN) {
        return SDLK_RETURN;
    }
    if(sc == SDL_SCANCODE_ESCAPE) {
        return SDLK_ESCAPE;
    }
    if(sc == SDL_SCANCODE_BACKSPACE) {
        return SDLK_BACKSPACE;
    }
    if(sc == SDL_SCANCODE_TAB) {
        return SDLK_TAB;
    }
    if(sc == SDL_SCANCODE_SPACE) {
        return SDLK_SPACE;
    }
    if(sc == SDL_SCANCODE_GRAVE) {
        return SDLK_BACKQUOTE;
    }
    if(sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z) {
        return (SDL_Keycode)('a' + (sc - SDL_SCANCODE_A));
    }
    if(sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_9) {
        return (SDL_Keycode)('1' + (sc - SDL_SCANCODE_1));
    }
    if(sc == SDL_SCANCODE_0) {
        return SDLK_0;
    }
    return (SDL_Keycode)(sc | SDLK_SCANCODE_MASK);
}

static Uint16 current_mod(void) {
    Uint16 mod = 0;
    if(GetKeyState(VK_LSHIFT) & 0x8000) {
        mod |= KMOD_LSHIFT;
    }
    if(GetKeyState(VK_RSHIFT) & 0x8000) {
        mod |= KMOD_RSHIFT;
    }
    if(GetKeyState(VK_LCONTROL) & 0x8000) {
        mod |= KMOD_LCTRL;
    }
    if(GetKeyState(VK_RCONTROL) & 0x8000) {
        mod |= KMOD_RCTRL;
    }
    if(GetKeyState(VK_LMENU) & 0x8000) {
        mod |= KMOD_LALT;
    }
    if(GetKeyState(VK_RMENU) & 0x8000) {
        mod |= KMOD_RALT;
    }
    return mod;
}

static void push_key(HWND hwnd, LPARAM lp, int down) {
    unsigned scan = (unsigned)((lp >> 16) & 0xFF);
    int extended = (lp & (1 << 24)) != 0;
    SDL_Scancode code = map_scancode(scan, extended);
    SDL_Event ev;
    if(code == SDL_SCANCODE_UNKNOWN || code >= SDL_NUM_SCANCODES) {
        return;
    }
    keystate[code] = down ? 1 : 0;
    memset(&ev, 0, sizeof(ev));
    ev.type = down ? SDL_KEYDOWN : SDL_KEYUP;
    ev.key.windowID = (Uint32)(UINT_PTR)hwnd;
    ev.key.state = down ? 1 : 0;
    ev.key.repeat = (down && (lp & (1 << 30))) ? 1 : 0;
    ev.key.keysym.scancode = code;
    ev.key.keysym.sym = scancode_to_keycode(code);
    ev.key.keysym.mod = current_mod();
    push_event(&ev);
}

static void push_text(HWND hwnd, wchar_t ch) {
    SDL_Event ev;
    char utf8[8];
    int n;
    wchar_t tmp[2];
    if(ch < 32) {
        return;
    }
    tmp[0] = ch;
    tmp[1] = 0;
    n = WideCharToMultiByte(CP_UTF8, 0, tmp, -1, utf8, (int)sizeof(utf8), NULL, NULL);
    if(n <= 1) {
        return;
    }
    memset(&ev, 0, sizeof(ev));
    ev.type = SDL_TEXTINPUT;
    ev.text.windowID = (Uint32)(UINT_PTR)hwnd;
    memcpy(ev.text.text, utf8, (size_t)n);
    push_event(&ev);
}

static void push_window(HWND hwnd, Uint8 event_id) {
    SDL_Event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = SDL_WINDOWEVENT;
    ev.window.windowID = (Uint32)(UINT_PTR)hwnd;
    ev.window.event = event_id;
    push_event(&ev);
}

static int register_class(void) {
    WNDCLASSW wc;
    if(class_ready) {
        return 1;
    }
    instance = GetModuleHandleW(NULL);
    arrow_cursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = instance;
    wc.hCursor = arrow_cursor;
    wc.lpszClassName = class_name;
    if(RegisterClassW(&wc) == 0) {
        return 0;
    }
    class_ready = 1;
    return 1;
}

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    SDL_Window *window = (SDL_Window *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    switch(msg) {
        case WM_NCCREATE: {
            CREATESTRUCTW *cs = (CREATESTRUCTW *)lParam;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
            break;
        }
        case WM_CLOSE: {
            SDL_Event ev;
            memset(&ev, 0, sizeof(ev));
            ev.type = SDL_QUIT;
            push_event(&ev);
        }
            return 0;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            push_key(hwnd, lParam, 1);
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            push_key(hwnd, lParam, 0);
            break;
        case WM_CHAR:
            push_text(hwnd, (wchar_t)wParam);
            break;
        case WM_MOUSEMOVE: {
            SDL_Event ev;
            int x = (int)(short)LOWORD(lParam);
            int y = (int)(short)HIWORD(lParam);
            memset(&ev, 0, sizeof(ev));
            ev.type = SDL_MOUSEMOTION;
            ev.motion.windowID = (Uint32)(UINT_PTR)hwnd;
            ev.motion.x = x;
            ev.motion.y = y;
            if(have_mouse) {
                ev.motion.xrel = x - mouse_x;
                ev.motion.yrel = y - mouse_y;
            }
            mouse_x = x;
            mouse_y = y;
            have_mouse = 1;
            push_event(&ev);
            break;
        }
        case WM_SIZE:
            if(window != NULL) {
                RECT rc;
                if(GetClientRect(hwnd, &rc)) {
                    window->w = (int)(rc.right - rc.left);
                    window->h = (int)(rc.bottom - rc.top);
                }
            }
            if(wParam == SIZE_MINIMIZED) {
                push_window(hwnd, SDL_WINDOWEVENT_MINIMIZED);
            } else if(wParam == SIZE_MAXIMIZED) {
                push_window(hwnd, SDL_WINDOWEVENT_MAXIMIZED);
            } else if(wParam == SIZE_RESTORED) {
                push_window(hwnd, SDL_WINDOWEVENT_RESTORED);
            }
            break;
        case WM_SHOWWINDOW:
            push_window(hwnd, wParam ? SDL_WINDOWEVENT_SHOWN : SDL_WINDOWEVENT_HIDDEN);
            break;
        case WM_KILLFOCUS:
            memset(keystate, 0, sizeof(keystate));
            break;
        case WM_SETCURSOR:
            if(LOWORD(lParam) == HTCLIENT) {
                SetCursor(cursor_shown ? arrow_cursor : NULL);
                return TRUE;
            }
            break;
        default:
            break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static void track_window(SDL_Window *window) {
    int i;
    for(i = 0; i < MAX_WINDOWS; i++) {
        if(windows[i] == NULL) {
            windows[i] = window;
            return;
        }
    }
}

static void untrack_window(SDL_Window *window) {
    int i;
    for(i = 0; i < MAX_WINDOWS; i++) {
        if(windows[i] == window) {
            windows[i] = NULL;
        }
    }
}

int SDL_Init(Uint32 flags) {
    (void)flags;
    if(sdl_ready) {
        return 0;
    }
    SetProcessDPIAware();
    timeBeginPeriod(1);
    timer_hires = 1;
    qpc_init();
    if(!register_class()) {
        sdl_set_error("RegisterClassW failed (%lu)", GetLastError());
        return -1;
    }
    sdl_ready = 1;
    return 0;
}

int SDL_InitSubSystem(Uint32 flags) {
    if((flags & (SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER)) != 0) {
        int user;
        pad_mask = 0;
        for(user = 0; user < 4; user++) {
            XINPUT_STATE state;
            if(XInputGetState((DWORD)user, &state) == ERROR_SUCCESS) {
                pad_mask |= 1 << user;
            }
        }
        pads_ready = 1;
    }
    return 0;
}

void SDL_QuitSubSystem(Uint32 flags) {
    (void)flags;
}

void SDL_Quit(void) {
    int i;
    for(i = 0; i < MAX_WINDOWS; i++) {
        if(windows[i] != NULL) {
            SDL_DestroyWindow(windows[i]);
        }
    }
    if(timer_hires) {
        timeEndPeriod(1);
        timer_hires = 0;
    }
    if(class_ready) {
        UnregisterClassW(class_name, instance);
        class_ready = 0;
    }
    sdl_ready = 0;
}

void SDL_GetVersion(SDL_version *ver) {
    if(ver == NULL) {
        return;
    }
    ver->major = 2;
    ver->minor = 30;
    ver->patch = 0;
}

const char *SDL_GetPlatform(void) {
    return "Windows";
}

SDL_Window *SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags) {
    SDL_Window *window;
    wchar_t *title_w;
    DWORD style;
    RECT rect;
    int width;
    int height;
    int px;
    int py;
    if(!register_class()) {
        sdl_set_error("RegisterClassW failed (%lu)", GetLastError());
        return NULL;
    }
    window = (SDL_Window *)calloc(1, sizeof(SDL_Window));
    if(window == NULL) {
        sdl_set_error("out of memory");
        return NULL;
    }
    window->w = w;
    window->h = h;
    style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    if((flags & SDL_WINDOW_HIDDEN) == 0) {
        style |= WS_VISIBLE;
    }
    rect.left = 0;
    rect.top = 0;
    rect.right = w;
    rect.bottom = h;
    AdjustWindowRect(&rect, style, FALSE);
    width = (int)(rect.right - rect.left);
    height = (int)(rect.bottom - rect.top);
    px = x;
    py = y;
    if(x == (int)SDL_WINDOWPOS_CENTERED || y == (int)SDL_WINDOWPOS_CENTERED) {
        RECT work;
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
        if(x == (int)SDL_WINDOWPOS_CENTERED) {
            px = work.left + ((work.right - work.left) - width) / 2;
        }
        if(y == (int)SDL_WINDOWPOS_CENTERED) {
            py = work.top + ((work.bottom - work.top) - height) / 2;
        }
    }
    title_w = utf8_to_wide(title != NULL ? title : "");
    window->hwnd = CreateWindowExW(0, class_name, title_w != NULL ? title_w : L"", style, px, py, width, height, NULL,
                                   NULL, instance, window);
    free(title_w);
    if(window->hwnd == NULL) {
        sdl_set_error("CreateWindowExW failed (%lu)", GetLastError());
        free(window);
        return NULL;
    }
    window->hdc = GetDC(window->hwnd);
    track_window(window);
    if((flags & SDL_WINDOW_FULLSCREEN) != 0) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }
    return window;
}

void SDL_DestroyWindow(SDL_Window *window) {
    if(window == NULL) {
        return;
    }
    if(window->glrc != NULL) {
        if(wglGetCurrentContext() == window->glrc) {
            wglMakeCurrent(NULL, NULL);
        }
        wglDeleteContext(window->glrc);
        window->glrc = NULL;
    }
    if(window->hwnd != NULL) {
        DestroyWindow(window->hwnd);
        window->hwnd = NULL;
        window->hdc = NULL;
    }
    untrack_window(window);
    free(window);
}

void SDL_SetWindowSize(SDL_Window *window, int w, int h) {
    RECT rect;
    DWORD style;
    if(window == NULL || window->hwnd == NULL) {
        return;
    }
    window->w = w;
    window->h = h;
    if(window->fullscreen) {
        return;
    }
    style = (DWORD)GetWindowLongPtrW(window->hwnd, GWL_STYLE);
    rect.left = 0;
    rect.top = 0;
    rect.right = w;
    rect.bottom = h;
    AdjustWindowRect(&rect, style, FALSE);
    SetWindowPos(window->hwnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

int SDL_SetWindowFullscreen(SDL_Window *window, Uint32 flags) {
    int want;
    MONITORINFO info;
    if(window == NULL || window->hwnd == NULL) {
        return -1;
    }
    want = (flags & SDL_WINDOW_FULLSCREEN) != 0;
    if(want == window->fullscreen) {
        return 0;
    }
    if(want) {
        HMONITOR monitor = MonitorFromWindow(window->hwnd, MONITOR_DEFAULTTONEAREST);
        info.cbSize = sizeof(info);
        if(!GetMonitorInfoW(monitor, &info)) {
            sdl_set_error("GetMonitorInfoW failed");
            return -1;
        }
        window->saved_style = (DWORD)GetWindowLongPtrW(window->hwnd, GWL_STYLE);
        GetWindowRect(window->hwnd, &window->saved_rect);
        SetWindowLongPtrW(window->hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(window->hwnd, HWND_TOP, info.rcMonitor.left, info.rcMonitor.top,
                     info.rcMonitor.right - info.rcMonitor.left, info.rcMonitor.bottom - info.rcMonitor.top,
                     SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        window->fullscreen = 1;
    } else {
        SetWindowLongPtrW(window->hwnd, GWL_STYLE, window->saved_style | WS_VISIBLE);
        SetWindowPos(window->hwnd, HWND_NOTOPMOST, window->saved_rect.left, window->saved_rect.top,
                     window->saved_rect.right - window->saved_rect.left,
                     window->saved_rect.bottom - window->saved_rect.top, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        window->fullscreen = 0;
    }
    return 0;
}

void SDL_DisableScreenSaver(void) {
    SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
}

int SDL_GL_SetAttribute(int attr, int value) {
    switch(attr) {
        case SDL_GL_RED_SIZE:
            gl_red = value;
            return 0;
        case SDL_GL_GREEN_SIZE:
            gl_green = value;
            return 0;
        case SDL_GL_BLUE_SIZE:
            gl_blue = value;
            return 0;
        case SDL_GL_ALPHA_SIZE:
            gl_alpha = value;
            return 0;
        case SDL_GL_CONTEXT_MAJOR_VERSION:
            gl_major = value;
            return 0;
        case SDL_GL_CONTEXT_MINOR_VERSION:
            gl_minor = value;
            return 0;
        case SDL_GL_CONTEXT_PROFILE_MASK:
            gl_profile = value;
            return 0;
        default:
            sdl_set_error("unsupported GL attribute %d", attr);
            return -1;
    }
}

static int proc_ok(PROC proc) {
    return proc != NULL && proc != (PROC)1 && proc != (PROC)2 && proc != (PROC)3 && proc != (PROC)-1;
}

static int set_pixel_format(SDL_Window *window) {
    PIXELFORMATDESCRIPTOR pfd;
    int format;
    if(window->pixel_set) {
        return 1;
    }
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = (BYTE)(gl_red + gl_green + gl_blue);
    pfd.cRedBits = (BYTE)gl_red;
    pfd.cGreenBits = (BYTE)gl_green;
    pfd.cBlueBits = (BYTE)gl_blue;
    pfd.cAlphaBits = (BYTE)gl_alpha;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;
    format = ChoosePixelFormat(window->hdc, &pfd);
    if(format == 0 || !SetPixelFormat(window->hdc, format, &pfd)) {
        sdl_set_error("SetPixelFormat failed (%lu)", GetLastError());
        return 0;
    }
    window->pixel_set = 1;
    return 1;
}

SDL_GLContext SDL_GL_CreateContext(SDL_Window *window) {
    HGLRC dummy;
    HGLRC real;
    wgl_create_attribs_fn create_attribs;
    int attribs[8];
    if(window == NULL || window->hdc == NULL) {
        sdl_set_error("no window for GL context");
        return NULL;
    }
    if(!set_pixel_format(window)) {
        return NULL;
    }
    if(window->glrc != NULL) {
        if(wglGetCurrentContext() == window->glrc) {
            wglMakeCurrent(NULL, NULL);
        }
        wglDeleteContext(window->glrc);
        window->glrc = NULL;
    }
    dummy = wglCreateContext(window->hdc);
    if(dummy == NULL || !wglMakeCurrent(window->hdc, dummy)) {
        sdl_set_error("wglCreateContext failed (%lu)", GetLastError());
        if(dummy != NULL) {
            wglDeleteContext(dummy);
        }
        return NULL;
    }
    create_attribs = (wgl_create_attribs_fn)wglGetProcAddress("wglCreateContextAttribsARB");
    if(!proc_ok((PROC)create_attribs)) {
        if(gl_major >= 3) {
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(dummy);
            sdl_set_error("wglCreateContextAttribsARB is required for OpenGL %d.%d", gl_major, gl_minor);
            return NULL;
        }
        window->glrc = dummy;
    } else {
        int n = 0;
        attribs[n++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
        attribs[n++] = gl_major;
        attribs[n++] = WGL_CONTEXT_MINOR_VERSION_ARB;
        attribs[n++] = gl_minor;
        if((gl_profile & SDL_GL_CONTEXT_PROFILE_CORE) != 0) {
            attribs[n++] = WGL_CONTEXT_PROFILE_MASK_ARB;
            attribs[n++] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
        }
        attribs[n] = 0;
        real = create_attribs(window->hdc, NULL, attribs);
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(dummy);
        if(real == NULL || !wglMakeCurrent(window->hdc, real)) {
            if(real != NULL) {
                wglDeleteContext(real);
            }
            sdl_set_error("OpenGL %d.%d context failed (%lu)", gl_major, gl_minor, GetLastError());
            return NULL;
        }
        window->glrc = real;
    }
    if(!gl_loaded) {
        if(!gl_minlib_load()) {
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(window->glrc);
            window->glrc = NULL;
            return NULL;
        }
        gl_loaded = 1;
    }
    return (SDL_GLContext)window->glrc;
}

void SDL_GL_DeleteContext(SDL_GLContext context) {
    HGLRC rc = (HGLRC)context;
    int i;
    if(rc == NULL) {
        return;
    }
    if(wglGetCurrentContext() == rc) {
        wglMakeCurrent(NULL, NULL);
    }
    for(i = 0; i < MAX_WINDOWS; i++) {
        if(windows[i] != NULL && windows[i]->glrc == rc) {
            windows[i]->glrc = NULL;
        }
    }
    wglDeleteContext(rc);
}

void SDL_GL_SwapWindow(SDL_Window *window) {
    if(window == NULL || window->hdc == NULL) {
        return;
    }
    if(window->glrc != NULL) {
        wglMakeCurrent(window->hdc, window->glrc);
    }
    SwapBuffers(window->hdc);
}

int SDL_GL_SetSwapInterval(int interval) {
    if(swap_interval_proc == NULL) {
        swap_interval_proc = (wgl_swap_interval_fn)wglGetProcAddress("wglSwapIntervalEXT");
    }
    if(!proc_ok((PROC)swap_interval_proc)) {
        swap_interval_proc = NULL;
        sdl_set_error("wglSwapIntervalEXT is not available");
        return -1;
    }
    if(!swap_interval_proc(interval)) {
        sdl_set_error("wglSwapIntervalEXT(%d) failed", interval);
        return -1;
    }
    return 0;
}

void SDL_GL_GetDrawableSize(SDL_Window *window, int *w, int *h) {
    RECT rc;
    if(window == NULL || window->hwnd == NULL) {
        if(w != NULL) {
            *w = 0;
        }
        if(h != NULL) {
            *h = 0;
        }
        return;
    }
    if(!GetClientRect(window->hwnd, &rc)) {
        rc.left = 0;
        rc.top = 0;
        rc.right = window->w;
        rc.bottom = window->h;
    }
    if(w != NULL) {
        *w = (int)(rc.right - rc.left);
    }
    if(h != NULL) {
        *h = (int)(rc.bottom - rc.top);
    }
}

static void pump(void) {
    MSG msg;
    while(PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

static int sample_pad_mask(void) {
    int mask = 0;
    int user;
    for(user = 0; user < 4; user++) {
        XINPUT_STATE state;
        if(XInputGetState((DWORD)user, &state) == ERROR_SUCCESS) {
            mask |= 1 << user;
        }
    }
    return mask;
}

static int device_index_for_user(int user, int mask) {
    int index = 0;
    int i;
    for(i = 0; i < user; i++) {
        if((mask & (1 << i)) != 0) {
            index++;
        }
    }
    return index;
}

static void service_joysticks(void) {
    int now;
    int user;
    if(!pads_ready) {
        return;
    }
    now = sample_pad_mask();
    for(user = 0; user < 4; user++) {
        int bit = 1 << user;
        SDL_Event ev;
        if((now & bit) != 0 && (pad_mask & bit) == 0) {
            memset(&ev, 0, sizeof(ev));
            ev.type = SDL_JOYDEVICEADDED;
            ev.jdevice.which = device_index_for_user(user, now);
            push_event(&ev);
        } else if((now & bit) == 0 && (pad_mask & bit) != 0) {
            memset(&ev, 0, sizeof(ev));
            ev.type = SDL_JOYDEVICEREMOVED;
            ev.jdevice.which = user;
            push_event(&ev);
        }
    }
    pad_mask = now;
}

static void service_rumble(void) {
    int user;
    Uint64 now = SDL_GetTicks64();
    for(user = 0; user < 4; user++) {
        if(rumble_until[user] != 0 && now >= rumble_until[user]) {
            XINPUT_VIBRATION vib;
            vib.wLeftMotorSpeed = 0;
            vib.wRightMotorSpeed = 0;
            XInputSetState((DWORD)user, &vib);
            rumble_until[user] = 0;
        }
    }
}

int SDL_PollEvent(SDL_Event *event) {
    pump();
    service_joysticks();
    service_rumble();
    if(ev_count == 0) {
        return 0;
    }
    if(event == NULL) {
        return 1;
    }
    *event = events[ev_head];
    ev_head = (ev_head + 1) % EVENT_QUEUE;
    ev_count--;
    return 1;
}

const Uint8 *SDL_GetKeyboardState(int *numkeys) {
    if(numkeys != NULL) {
        *numkeys = SDL_NUM_SCANCODES;
    }
    return keystate;
}

typedef struct ScancodeName {
    SDL_Scancode code;
    const char *name;
} ScancodeName;

static const ScancodeName scancode_names[] = {
    {SDL_SCANCODE_A,            "A"           },
    {SDL_SCANCODE_B,            "B"           },
    {SDL_SCANCODE_C,            "C"           },
    {SDL_SCANCODE_D,            "D"           },
    {SDL_SCANCODE_E,            "E"           },
    {SDL_SCANCODE_F,            "F"           },
    {SDL_SCANCODE_G,            "G"           },
    {SDL_SCANCODE_H,            "H"           },
    {SDL_SCANCODE_I,            "I"           },
    {SDL_SCANCODE_J,            "J"           },
    {SDL_SCANCODE_K,            "K"           },
    {SDL_SCANCODE_L,            "L"           },
    {SDL_SCANCODE_M,            "M"           },
    {SDL_SCANCODE_N,            "N"           },
    {SDL_SCANCODE_O,            "O"           },
    {SDL_SCANCODE_P,            "P"           },
    {SDL_SCANCODE_Q,            "Q"           },
    {SDL_SCANCODE_R,            "R"           },
    {SDL_SCANCODE_S,            "S"           },
    {SDL_SCANCODE_T,            "T"           },
    {SDL_SCANCODE_U,            "U"           },
    {SDL_SCANCODE_V,            "V"           },
    {SDL_SCANCODE_W,            "W"           },
    {SDL_SCANCODE_X,            "X"           },
    {SDL_SCANCODE_Y,            "Y"           },
    {SDL_SCANCODE_Z,            "Z"           },
    {SDL_SCANCODE_1,            "1"           },
    {SDL_SCANCODE_2,            "2"           },
    {SDL_SCANCODE_3,            "3"           },
    {SDL_SCANCODE_4,            "4"           },
    {SDL_SCANCODE_5,            "5"           },
    {SDL_SCANCODE_6,            "6"           },
    {SDL_SCANCODE_7,            "7"           },
    {SDL_SCANCODE_8,            "8"           },
    {SDL_SCANCODE_9,            "9"           },
    {SDL_SCANCODE_0,            "0"           },
    {SDL_SCANCODE_RETURN,       "Return"      },
    {SDL_SCANCODE_ESCAPE,       "Escape"      },
    {SDL_SCANCODE_BACKSPACE,    "Backspace"   },
    {SDL_SCANCODE_TAB,          "Tab"         },
    {SDL_SCANCODE_SPACE,        "Space"       },
    {SDL_SCANCODE_MINUS,        "-"           },
    {SDL_SCANCODE_EQUALS,       "="           },
    {SDL_SCANCODE_LEFTBRACKET,  "["           },
    {SDL_SCANCODE_RIGHTBRACKET, "]"           },
    {SDL_SCANCODE_BACKSLASH,    "\\"          },
    {SDL_SCANCODE_SEMICOLON,    ";"           },
    {SDL_SCANCODE_APOSTROPHE,   "'"           },
    {SDL_SCANCODE_GRAVE,        "`"           },
    {SDL_SCANCODE_GRAVE,        "Grave"       },
    {SDL_SCANCODE_COMMA,        ","           },
    {SDL_SCANCODE_PERIOD,       "."           },
    {SDL_SCANCODE_SLASH,        "/"           },
    {SDL_SCANCODE_CAPSLOCK,     "CapsLock"    },
    {SDL_SCANCODE_F1,           "F1"          },
    {SDL_SCANCODE_F2,           "F2"          },
    {SDL_SCANCODE_F3,           "F3"          },
    {SDL_SCANCODE_F4,           "F4"          },
    {SDL_SCANCODE_F5,           "F5"          },
    {SDL_SCANCODE_F6,           "F6"          },
    {SDL_SCANCODE_F7,           "F7"          },
    {SDL_SCANCODE_F8,           "F8"          },
    {SDL_SCANCODE_F9,           "F9"          },
    {SDL_SCANCODE_F10,          "F10"         },
    {SDL_SCANCODE_F11,          "F11"         },
    {SDL_SCANCODE_F12,          "F12"         },
    {SDL_SCANCODE_HOME,         "Home"        },
    {SDL_SCANCODE_PAGEUP,       "PageUp"      },
    {SDL_SCANCODE_DELETE,       "Delete"      },
    {SDL_SCANCODE_END,          "End"         },
    {SDL_SCANCODE_PAGEDOWN,     "PageDown"    },
    {SDL_SCANCODE_RIGHT,        "Right"       },
    {SDL_SCANCODE_LEFT,         "Left"        },
    {SDL_SCANCODE_DOWN,         "Down"        },
    {SDL_SCANCODE_UP,           "Up"          },
    {SDL_SCANCODE_KP_DIVIDE,    "Keypad /"    },
    {SDL_SCANCODE_KP_MULTIPLY,  "Keypad *"    },
    {SDL_SCANCODE_KP_MINUS,     "Keypad -"    },
    {SDL_SCANCODE_KP_PLUS,      "Keypad +"    },
    {SDL_SCANCODE_KP_ENTER,     "Keypad Enter"},
    {SDL_SCANCODE_KP_1,         "Keypad 1"    },
    {SDL_SCANCODE_KP_2,         "Keypad 2"    },
    {SDL_SCANCODE_KP_3,         "Keypad 3"    },
    {SDL_SCANCODE_KP_4,         "Keypad 4"    },
    {SDL_SCANCODE_KP_5,         "Keypad 5"    },
    {SDL_SCANCODE_KP_6,         "Keypad 6"    },
    {SDL_SCANCODE_KP_7,         "Keypad 7"    },
    {SDL_SCANCODE_KP_8,         "Keypad 8"    },
    {SDL_SCANCODE_KP_9,         "Keypad 9"    },
    {SDL_SCANCODE_KP_0,         "Keypad 0"    },
    {SDL_SCANCODE_KP_PERIOD,    "Keypad ."    },
    {SDL_SCANCODE_LCTRL,        "Left Ctrl"   },
    {SDL_SCANCODE_LSHIFT,       "Left Shift"  },
    {SDL_SCANCODE_LALT,         "Left Alt"    },
    {SDL_SCANCODE_LGUI,         "Left GUI"    },
    {SDL_SCANCODE_RCTRL,        "Right Ctrl"  },
    {SDL_SCANCODE_RSHIFT,       "Right Shift" },
    {SDL_SCANCODE_RALT,         "Right Alt"   },
    {SDL_SCANCODE_RGUI,         "Right GUI"   },
};

const char *SDL_GetScancodeName(SDL_Scancode scancode) {
    int i;
    int count = (int)(sizeof(scancode_names) / sizeof(scancode_names[0]));
    for(i = 0; i < count; i++) {
        if(scancode_names[i].code == scancode) {
            return scancode_names[i].name;
        }
    }
    return "";
}

SDL_Scancode SDL_GetScancodeFromName(const char *name) {
    int i;
    int count;
    if(name == NULL) {
        return SDL_SCANCODE_UNKNOWN;
    }
    count = (int)(sizeof(scancode_names) / sizeof(scancode_names[0]));
    for(i = 0; i < count; i++) {
        if(_stricmp(scancode_names[i].name, name) == 0) {
            return scancode_names[i].code;
        }
    }
    return SDL_SCANCODE_UNKNOWN;
}

SDL_bool SDL_HasClipboardText(void) {
    return IsClipboardFormatAvailable(CF_UNICODETEXT) ? SDL_TRUE : SDL_FALSE;
}

char *SDL_GetClipboardText(void) {
    char *out = NULL;
    if(OpenClipboard(NULL)) {
        HANDLE data = GetClipboardData(CF_UNICODETEXT);
        if(data != NULL) {
            wchar_t *locked = (wchar_t *)GlobalLock(data);
            if(locked != NULL) {
                out = wide_to_utf8(locked);
                GlobalUnlock(data);
            }
        }
        CloseClipboard();
    }
    if(out == NULL) {
        out = (char *)malloc(1);
        if(out != NULL) {
            out[0] = 0;
        }
    }
    return out;
}

int SDL_ShowCursor(int toggle) {
    if(toggle < 0) {
        return cursor_shown;
    }
    if(toggle && !cursor_shown) {
        while(ShowCursor(TRUE) < 0) {
        }
        cursor_shown = 1;
    } else if(!toggle && cursor_shown) {
        while(ShowCursor(FALSE) >= 0) {
        }
        cursor_shown = 0;
    }
    return cursor_shown;
}

int SDL_ShowSimpleMessageBox(Uint32 flags, const char *title, const char *message, SDL_Window *window) {
    wchar_t *wtitle = utf8_to_wide(title != NULL ? title : "");
    wchar_t *wmsg = utf8_to_wide(message != NULL ? message : "");
    HWND hwnd = NULL;
    (void)flags;
    if(window != NULL) {
        hwnd = window->hwnd;
    }
    MessageBoxW(hwnd, wmsg != NULL ? wmsg : L"", wtitle != NULL ? wtitle : L"", MB_OK | MB_ICONERROR);
    free(wtitle);
    free(wmsg);
    return 0;
}

char *SDL_GetBasePath(void) {
    wchar_t buf[MAX_PATH];
    wchar_t *slash;
    DWORD n = GetModuleFileNameW(NULL, buf, MAX_PATH);
    if(n == 0 || n >= MAX_PATH) {
        sdl_set_error("GetModuleFileNameW failed");
        return NULL;
    }
    slash = wcsrchr(buf, L'\\');
    if(slash != NULL) {
        slash[1] = 0;
    }
    return wide_to_utf8(buf);
}

char *SDL_GetPrefPath(const char *org, const char *app) {
    wchar_t appdata[MAX_PATH];
    wchar_t path[MAX_PATH];
    wchar_t *app_w;
    DWORD n;
    (void)org;
    n = GetEnvironmentVariableW(L"APPDATA", appdata, MAX_PATH);
    if(n == 0 || n >= MAX_PATH) {
        sdl_set_error("APPDATA is not set");
        return NULL;
    }
    app_w = utf8_to_wide(app != NULL ? app : "OpenOMF");
    if(app_w == NULL) {
        return NULL;
    }
    _snwprintf(path, MAX_PATH, L"%s\\%s\\", appdata, app_w);
    path[MAX_PATH - 1] = 0;
    free(app_w);
    CreateDirectoryW(path, NULL);
    return wide_to_utf8(path);
}

static int user_from_device(int device_index) {
    int found = 0;
    int user;
    if(device_index < 0) {
        return -1;
    }
    for(user = 0; user < 4; user++) {
        XINPUT_STATE state;
        if(XInputGetState((DWORD)user, &state) == ERROR_SUCCESS) {
            if(found == device_index) {
                return user;
            }
            found++;
        }
    }
    return -1;
}

int SDL_NumJoysticks(void) {
    int count = 0;
    int user;
    for(user = 0; user < 4; user++) {
        XINPUT_STATE state;
        if(XInputGetState((DWORD)user, &state) == ERROR_SUCCESS) {
            count++;
        }
    }
    return count;
}

static const char *controller_name(void) {
    return "XInput Controller";
}

const char *SDL_JoystickNameForIndex(int device_index) {
    if(user_from_device(device_index) < 0) {
        return NULL;
    }
    return controller_name();
}

const char *SDL_GameControllerNameForIndex(int joystick_index) {
    return SDL_JoystickNameForIndex(joystick_index);
}

SDL_Joystick *SDL_JoystickOpen(int device_index) {
    int user = user_from_device(device_index);
    SDL_Joystick *joy;
    if(user < 0) {
        sdl_set_error("joystick %d is not connected", device_index);
        return NULL;
    }
    joy = (SDL_Joystick *)calloc(1, sizeof(SDL_Joystick));
    if(joy == NULL) {
        return NULL;
    }
    joy->user_index = user;
    joy->instance_id = user;
    return joy;
}

void SDL_JoystickClose(SDL_Joystick *joystick) {
    free(joystick);
}

SDL_bool SDL_JoystickGetAttached(SDL_Joystick *joystick) {
    XINPUT_STATE state;
    if(joystick == NULL) {
        return SDL_FALSE;
    }
    if(XInputGetState((DWORD)joystick->user_index, &state) != ERROR_SUCCESS) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick *joystick) {
    SDL_JoystickGUID guid;
    memset(&guid, 0, sizeof(guid));
    guid.data[0] = 'x';
    guid.data[1] = 'i';
    guid.data[2] = 'n';
    guid.data[3] = 'p';
    if(joystick != NULL) {
        guid.data[15] = (Uint8)joystick->user_index;
    }
    return guid;
}

void SDL_JoystickGetGUIDString(SDL_JoystickGUID guid, char *pszGUID, int cbGUID) {
    int i;
    if(pszGUID == NULL || cbGUID <= 0) {
        return;
    }
    if(cbGUID < 33) {
        pszGUID[0] = 0;
        return;
    }
    for(i = 0; i < 16; i++) {
        sprintf(pszGUID + (i * 2), "%02x", guid.data[i]);
    }
}

int SDL_JoystickNumAxes(SDL_Joystick *joystick) {
    (void)joystick;
    return 6;
}

int SDL_JoystickNumButtons(SDL_Joystick *joystick) {
    (void)joystick;
    return 15;
}

int SDL_JoystickNumBalls(SDL_Joystick *joystick) {
    (void)joystick;
    return 0;
}

int SDL_JoystickNumHats(SDL_Joystick *joystick) {
    (void)joystick;
    return 1;
}

Sint32 SDL_JoystickInstanceID(SDL_Joystick *joystick) {
    if(joystick == NULL) {
        return -1;
    }
    return joystick->instance_id;
}

SDL_GameController *SDL_GameControllerOpen(int joystick_index) {
    int user = user_from_device(joystick_index);
    SDL_GameController *controller;
    if(user < 0) {
        sdl_set_error("controller %d is not connected", joystick_index);
        return NULL;
    }
    controller = (SDL_GameController *)calloc(1, sizeof(SDL_GameController));
    if(controller == NULL) {
        return NULL;
    }
    controller->joy.user_index = user;
    controller->joy.instance_id = user;
    return controller;
}

void SDL_GameControllerClose(SDL_GameController *gamecontroller) {
    free(gamecontroller);
}

SDL_bool SDL_GameControllerGetAttached(SDL_GameController *gamecontroller) {
    if(gamecontroller == NULL) {
        return SDL_FALSE;
    }
    return SDL_JoystickGetAttached(&gamecontroller->joy);
}

SDL_Joystick *SDL_GameControllerGetJoystick(SDL_GameController *gamecontroller) {
    if(gamecontroller == NULL) {
        return NULL;
    }
    return &gamecontroller->joy;
}

static Sint16 negate_axis(SHORT value) {
    int negated = -(int)value;
    if(negated > 32767) {
        negated = 32767;
    }
    if(negated < -32768) {
        negated = -32768;
    }
    return (Sint16)negated;
}

Sint16 SDL_GameControllerGetAxis(SDL_GameController *gamecontroller, SDL_GameControllerAxis axis) {
    XINPUT_STATE state;
    if(gamecontroller == NULL || XInputGetState((DWORD)gamecontroller->joy.user_index, &state) != ERROR_SUCCESS) {
        return 0;
    }
    switch(axis) {
        case SDL_CONTROLLER_AXIS_LEFTX:
            return (Sint16)state.Gamepad.sThumbLX;
        case SDL_CONTROLLER_AXIS_LEFTY:
            return negate_axis(state.Gamepad.sThumbLY);
        case SDL_CONTROLLER_AXIS_RIGHTX:
            return (Sint16)state.Gamepad.sThumbRX;
        case SDL_CONTROLLER_AXIS_RIGHTY:
            return negate_axis(state.Gamepad.sThumbRY);
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            return (Sint16)(((int)state.Gamepad.bLeftTrigger * 32767) / 255);
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            return (Sint16)(((int)state.Gamepad.bRightTrigger * 32767) / 255);
        default:
            return 0;
    }
}

static WORD button_mask(SDL_GameControllerButton button) {
    switch(button) {
        case SDL_CONTROLLER_BUTTON_A:
            return XINPUT_GAMEPAD_A;
        case SDL_CONTROLLER_BUTTON_B:
            return XINPUT_GAMEPAD_B;
        case SDL_CONTROLLER_BUTTON_X:
            return XINPUT_GAMEPAD_X;
        case SDL_CONTROLLER_BUTTON_Y:
            return XINPUT_GAMEPAD_Y;
        case SDL_CONTROLLER_BUTTON_BACK:
            return XINPUT_GAMEPAD_BACK;
        case SDL_CONTROLLER_BUTTON_START:
            return XINPUT_GAMEPAD_START;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            return XINPUT_GAMEPAD_LEFT_THUMB;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            return XINPUT_GAMEPAD_RIGHT_THUMB;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            return XINPUT_GAMEPAD_LEFT_SHOULDER;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            return XINPUT_GAMEPAD_RIGHT_SHOULDER;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            return XINPUT_GAMEPAD_DPAD_UP;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            return XINPUT_GAMEPAD_DPAD_DOWN;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            return XINPUT_GAMEPAD_DPAD_LEFT;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            return XINPUT_GAMEPAD_DPAD_RIGHT;
        default:
            return 0;
    }
}

Uint8 SDL_GameControllerGetButton(SDL_GameController *gamecontroller, SDL_GameControllerButton button) {
    XINPUT_STATE state;
    WORD mask;
    if(gamecontroller == NULL || XInputGetState((DWORD)gamecontroller->joy.user_index, &state) != ERROR_SUCCESS) {
        return 0;
    }
    mask = button_mask(button);
    if(mask == 0) {
        return 0;
    }
    return (state.Gamepad.wButtons & mask) != 0 ? 1 : 0;
}

int SDL_GameControllerAddMappingsFromFile(const char *file) {
    (void)file;
    return -1;
}

int SDL_GameControllerNumMappings(void) {
    return 0;
}

SDL_Haptic *SDL_HapticOpenFromJoystick(SDL_Joystick *joystick) {
    SDL_Haptic *haptic;
    if(joystick == NULL) {
        return NULL;
    }
    haptic = (SDL_Haptic *)calloc(1, sizeof(SDL_Haptic));
    if(haptic == NULL) {
        return NULL;
    }
    haptic->user_index = joystick->user_index;
    return haptic;
}

void SDL_HapticClose(SDL_Haptic *haptic) {
    if(haptic != NULL && haptic->user_index >= 0 && haptic->user_index < 4) {
        XINPUT_VIBRATION vib;
        vib.wLeftMotorSpeed = 0;
        vib.wRightMotorSpeed = 0;
        XInputSetState((DWORD)haptic->user_index, &vib);
        rumble_until[haptic->user_index] = 0;
    }
    free(haptic);
}

int SDL_HapticRumbleSupported(SDL_Haptic *haptic) {
    return haptic != NULL ? 1 : 0;
}

int SDL_HapticRumbleInit(SDL_Haptic *haptic) {
    if(haptic == NULL) {
        return -1;
    }
    return 0;
}

int SDL_HapticRumblePlay(SDL_Haptic *haptic, float strength, Uint32 length) {
    XINPUT_VIBRATION vib;
    int mag;
    if(haptic == NULL || haptic->user_index < 0 || haptic->user_index > 3) {
        return -1;
    }
    if(strength < 0.0f) {
        strength = 0.0f;
    }
    if(strength > 1.0f) {
        strength = 1.0f;
    }
    mag = (int)(strength * 65535.0f);
    if(length == 0) {
        mag = 0;
        rumble_until[haptic->user_index] = 0;
    } else {
        rumble_until[haptic->user_index] = SDL_GetTicks64() + (Uint64)length;
    }
    vib.wLeftMotorSpeed = (WORD)mag;
    vib.wRightMotorSpeed = (WORD)mag;
    if(XInputSetState((DWORD)haptic->user_index, &vib) != ERROR_SUCCESS) {
        sdl_set_error("XInputSetState failed");
        return -1;
    }
    return 0;
}

SDL_RWops *SDL_RWFromFile(const char *file, const char *mode) {
    SDL_RWops *rw;
    FILE *fp;
    if(file == NULL || mode == NULL) {
        sdl_set_error("SDL_RWFromFile missing path");
        return NULL;
    }
    fp = fopen(file, mode);
    if(fp == NULL) {
        sdl_set_error("fopen failed");
        return NULL;
    }
    rw = (SDL_RWops *)calloc(1, sizeof(SDL_RWops));
    if(rw == NULL) {
        fclose(fp);
        return NULL;
    }
    rw->fp = fp;
    return rw;
}

size_t SDL_RWwrite(SDL_RWops *context, const void *ptr, size_t size, size_t num) {
    if(context == NULL || context->fp == NULL || ptr == NULL) {
        return 0;
    }
    return fwrite(ptr, size, num, context->fp);
}

int SDL_RWclose(SDL_RWops *context) {
    if(context == NULL) {
        return -1;
    }
    if(context->fp != NULL) {
        fclose(context->fp);
    }
    free(context);
    return 0;
}
