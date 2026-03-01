#include <windows.h>
#include <iostream>
#include <vector>
#include <String>


// секция данных игры  
typedef struct {
    float x, y, width, height, rad, dx, dy, speed;
    HBITMAP hBitmap;//хэндл к спрайту шарика 
} sprite;

sprite racket;//ракетка игрока
sprite enemy;//ракетка противника
sprite ball;//шарик
sprite platform;

struct {
    int score, balls;//количество набранных очков и оставшихся "жизней"
    bool action = false;//состояние - ожидание (игрок должен нажать пробел) или игра
} game;

struct {
    HWND hWnd;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;


HBITMAP hBack;// хэндл для фонового изображения
HBITMAP hBack1;
HBITMAP hBack2;
//cекция кода

void InitGame()
{
    //в этой секции загружаем спрайты с помощью функций gdi
    //пути относительные - файлы должны лежать рядом с .exe 
    //результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
    ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    enemy.hBitmap = (HBITMAP)LoadImageA(NULL, "defka.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "vagon.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack1 = (HBITMAP)LoadImageA(NULL, "stanciya.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack2 = (HBITMAP)LoadImageA(NULL, "f.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    platform.hBitmap = (HBITMAP)LoadImageA(NULL, "racket2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //------------------------------------------------------

    racket.width = 500;
    racket.height = 800;
    racket.speed = 30;//скорость перемещения ракетки
    racket.x = window.width / 2.;//ракетка посередине окна
    racket.y = window.height - racket.height;//чуть выше низа экрана - на высоту ракетки


    platform.y = racket.x;
    platform.height = 100;
    platform.width = 300;
    platform.x = window.width - platform.width;

    enemy.x = 300;//х координату оппонета ставим в ту же точку что и игрока
    enemy.y = 290;
    enemy.width = 500;
    enemy.height = 800;


    ball.dy = (rand() % 65 + 35) / 100.;//формируем вектор полета шарика
    ball.dx = -(1 - ball.dy);//формируем вектор полета шарика
    ball.speed = 11;
    ball.rad = 20;

    ball.x = racket.x;//x координата шарика - на середие ракетки
    ball.y = racket.y - ball.rad;//шарик лежит сверху ракетки

    game.score = 0;
    game.balls = 9;


}



void ShowScore()
{
    SetTextColor(window.context, RGB(255, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
  
    HFONT hFont = CreateFont(60, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "Arial");
    HFONT hOldFont = (HFONT)SelectObject(window.context, hFont);
  
    TextOutA(window.context, window.width / 2 - 155, window.height / 2, "GAME OVER", 9);
  
    SelectObject(window.context, hOldFont);
    DeleteObject(hFont);
}

void ProcessInput()
{
    if (GetAsyncKeyState(VK_LEFT)) racket.x -= racket.speed;
    if (GetAsyncKeyState(VK_RIGHT)) racket.x += racket.speed;


}void input() {
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        static bool wasPressed = false;
        if (!wasPressed) {
            //game.current = (game.current + 1) % 3;

            wasPressed = true;
        }
    }
    else {
        static bool wasPressed = false;
        wasPressed = false;
    }
}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(hDC); // Создаем контекст памяти, совместимый с контекстом отображения
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// Выбираем изображение bitmap в контекст памяти

    if (hOldbm) // Если не было ошибок, продолжаем работу
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // Определяем размеры изображения

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//все пиксели черного цвета будут интепретированы как прозрачные
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // Рисуем изображение bitmap
        }

        SelectObject(hMemDC, hOldbm);// Восстанавливаем контекст памяти
    }

    DeleteDC(hMemDC); // Удаляем контекст памяти
}

void ShowRacketAndBall()
{
    if (GetAsyncKeyState(VK_LBUTTON)) {
        ShowBitmap(window.context, 0, 0, window.width, window.height, hBack1);//задний фон
    }
    else {
        ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//задний фон
    }

    ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);// ракетка игрока

    ShowBitmap(window.context, platform.x, platform.y, platform.width, platform.height, platform.hBitmap);
    ShowBitmap(window.context, enemy.x, enemy.y, enemy.width, enemy.height, enemy.hBitmap);//ракетка оппонента
    ShowBitmap(window.context, 100, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);// шарик
}

void LimitRacket()
{
    racket.x = max(racket.x, racket.width / 2.);//если коодината левого угла ракетки меньше нуля, присвоим ей ноль
    racket.x = min(racket.x, window.width - racket.width / 2.);//аналогично для правого угла
}



bool tail = false;






void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//из хэндла окна достаем хэндл контекста устройства для рисования
    window.width = r.right - r.left;//определяем размеры и сохраняем
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//второй буфер
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//привязываем окно к контексту
    GetClientRect(window.hWnd, &r);

}


void Collise() {

    if (platform.x == racket.x + racket.width) {
        racket.x = 100;
    }




}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{

    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    InitGame();//здесь инициализируем переменные игры

    ShowCursor(NULL);

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//рисуем фон, ракетку и шарик
        ShowScore();//рисуем очик и жизни
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)
        Collise();
        input();
        ProcessInput();//опрос клавиатуры
        LimitRacket();//проверяем, чтобы ракетка не убежала за экран

    }

}



//#include "Dialogsistem.h"

//struct {
//    HBITMAP back[3];
//    int current;
//} game;
//
//struct {
//    HWND hwnd;
//    HDC dc;
//    HDC buf;
//    int w, h;
//} win;
//void text() {
//
// }
//
//void load() {
//    game.back[0] = (HBITMAP)LoadImageA(0, "Assets/back1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
//    game.back[1] = (HBITMAP)LoadImageA(0, "Assets/back2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
//    game.back[2] = (HBITMAP)LoadImageA(0, "Assets/back3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
//    game.current = 0;
//}
//
//void draw() {
//    RECT r = { 0, 0, win.w, win.h };
//    FillRect(win.buf, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
//
//    if (game.back[game.current]) {
//        HDC mem = CreateCompatibleDC(win.buf);
//        HBITMAP old = (HBITMAP)SelectObject(mem, game.back[game.current]);
//        BITMAP bm;
//        GetObject(game.back[game.current], sizeof(bm), &bm);
//        StretchBlt(win.buf, 0, 0, win.w, win.h, mem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
//        SelectObject(mem, old);
//        DeleteDC(mem);
//       
//    }
//
//    BitBlt(win.dc, 0, 0, win.w, win.h, win.buf, 0, 0, SRCCOPY);
//}
//
//void Text() {
//
//    SetTextColor(window.context, RGB(255, 0, 0));
//    SetBkMode(window.context, TRANSPARENT);
//
//    HFONT hFont = CreateFont(60, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, L"Arial");
//    HFONT hOldFont = (HFONT)SelectObject(window.context, hFont);
//
//    TextOutA(window.context, window.w / 2 - 155, window.h / 2, "GAME OVER", 23);
//
//    SelectObject(window.context, hOldFont);
//    DeleteObject(hFont);
//
//}
//void input() {
//    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
//        static bool wasPressed = false;
//        if (!wasPressed) {
//            game.current = (game.current + 1) % 3;
//            draw();
//            wasPressed = true;
//        }
//    }
//    else {
//        static bool wasPressed = false;
//        wasPressed = false;
//    }
//}
//
//LRESULT CALLBACK wnd_proc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
//    if (msg == WM_LBUTTONDOWN) {
//        game.current = (game.current + 1) % 3;
//        draw();
//    }
//    if (msg == WM_DESTROY) {
//        PostQuitMessage(0);
//    }
//    return DefWindowProc(h, msg, wp, lp);
//}
//
//void create_window() {
//    WNDCLASS wc = {};
//    wc.lpfnWndProc = wnd_proc;
//    wc.hInstance = GetModuleHandle(0);
//    wc.lpszClassName = "win";
//    RegisterClass(&wc);
//
//    win.hwnd = CreateWindow("win", "фон", WS_OVERLAPPEDWINDOW,
//        0, 0, 1920, 1080, 0, 0, wc.hInstance, 0);
//
//    win.dc = GetDC(win.hwnd);
//    RECT rc;
//    GetClientRect(win.hwnd, &rc);
//    win.w = rc.right - rc.left;
//    win.h = rc.bottom - rc.top;
//
//    win.buf = CreateCompatibleDC(win.dc);
//    HBITMAP bmp = CreateCompatibleBitmap(win.dc, win.w, win.h);
//    SelectObject(win.buf, bmp);
//
//    ShowWindow(win.hwnd, SW_SHOW);
//}
//
//int WINAPI WinMain(HINSTANCE h, HINSTANCE hp, LPSTR cmd, int show) {
//    create_window();
//    load();
//    draw();
//
//    MSG msg;
//    while (GetMessage(&msg, 0, 0, 0)) {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//
//        input();
//    }
//    return msg.wParam;
//}