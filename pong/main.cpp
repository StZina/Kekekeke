#include <windows.h>

struct {
    HBITMAP back[3];
    int current;
} game;

struct {
    HWND hwnd;
    HDC dc;
    HDC buf;
    int w, h;
} win;

void load() {
    game.back[0] = (HBITMAP)LoadImageA(0, "Assets/back1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    game.back[1] = (HBITMAP)LoadImageA(0, "Assets/back2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    game.back[2] = (HBITMAP)LoadImageA(0, "Assets/back3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    game.current = 0;
}

void draw() {
    RECT r = { 0, 0, win.w, win.h };
    FillRect(win.buf, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));

    if (game.back[game.current]) {
        HDC mem = CreateCompatibleDC(win.buf);
        HBITMAP old = (HBITMAP)SelectObject(mem, game.back[game.current]);
        BITMAP bm;
        GetObject(game.back[game.current], sizeof(bm), &bm);
        StretchBlt(win.buf, 0, 0, win.w, win.h, mem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
        SelectObject(mem, old);
        DeleteDC(mem);
    }

    BitBlt(win.dc, 0, 0, win.w, win.h, win.buf, 0, 0, SRCCOPY);
}

void input() {
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        static bool wasPressed = false;
        if (!wasPressed) {
            game.current = (game.current + 1) % 3;
            draw();
            wasPressed = true;
        }
    }
    else {
        static bool wasPressed = false;
        wasPressed = false;
    }
}

LRESULT CALLBACK wnd_proc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_LBUTTONDOWN) {
        game.current = (game.current + 1) % 3;
        draw();
    }
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
    }
    return DefWindowProc(h, msg, wp, lp);
}

void create_window() {
    WNDCLASS wc = {};
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = GetModuleHandle(0);
    wc.lpszClassName = "win";
    RegisterClass(&wc);

    win.hwnd = CreateWindow("win", "фон", WS_OVERLAPPEDWINDOW,
        100, 100, 800, 600, 0, 0, wc.hInstance, 0);

    win.dc = GetDC(win.hwnd);
    RECT rc;
    GetClientRect(win.hwnd, &rc);
    win.w = rc.right - rc.left;
    win.h = rc.bottom - rc.top;

    win.buf = CreateCompatibleDC(win.dc);
    HBITMAP bmp = CreateCompatibleBitmap(win.dc, win.w, win.h);
    SelectObject(win.buf, bmp);

    ShowWindow(win.hwnd, SW_SHOW);
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE hp, LPSTR cmd, int show) {
    create_window();
    load();
    draw();

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        input();
    }
    return msg.wParam;
}