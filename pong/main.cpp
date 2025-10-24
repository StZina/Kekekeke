//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include <cmath>  

typedef struct {
    float x, y, width, height, rad, dx, Grav, speed;
    HBITMAP hBitmap;   //хэндл к спрайту шарика 
    bool isJumping;    // флаг прыжка
    float jumpSpeed;   // скорость прыжка
} sprite;

#define PLATFORM_COUNT 6  // количество платформ

sprite racket;//ракетка игрока
sprite enemy;//ракетка противника
sprite ball;//шарик
sprite platforms[PLATFORM_COUNT];// массив платформ


bool isBallActive = false;// Флаг активности шара

struct {
    int score, balls;//количество набранных очков и оставшихся "жизней"
    //bool action = false;//состояние - ожидание (игрок должен нажать пробела) или игра
} game;

struct {
    HWND hWnd;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

HBITMAP hBack;// хэндл для фонового изображения


void InitGame()
{
    //в этой секции загружаем спрайты с помощью функций gdi
    //пути относительные - файлы должны лежать рядом с .exe 
    //результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
    ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    enemy.hBitmap = (HBITMAP)LoadImageA(NULL, "racket_enemy.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    // ЗАГРУЗКА ПЛАТФОРМ
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        platforms[i].hBitmap = (HBITMAP)LoadImageA(NULL, "racket2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    }
   

    racket.width = 100;
    racket.height = 50;
    racket.speed = 30;
    racket.x = (window.width - racket.width) / 2.0f;
    racket.y = window.height - racket.height;
    racket.isJumping = false;
    racket.jumpSpeed = 15.0f;

    // НАСТРОЙКА ВСЕХ ПЛАТФОРМ
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        platforms[i].width = 200;
        platforms[i].height = 100;
    }

    // РАССТАНОВКА ПЛАТФОРМ 
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        platforms[i].x = 100.0f + i * (window.width - 100.0f) / (PLATFORM_COUNT - 1);
        platforms[i].y = 100.0f + i * (window.height - 200.0f) / (PLATFORM_COUNT - 1);
    }

    //enemy.x = racket.x;//х координату оппонета ставим в ту же точку что и игрока

    // НАСТРОЙКА ШАРА
    ball.rad = 20.0f;                          // радиус шара
    ball.width = 2 * ball.rad;              // ширина = диаметр
    ball.height = 2 * ball.rad;             // высота = диаметр
    ball.speed = 3.0f;                      // скорость движения

    // СЛУЧАЙНОЕ НАЧАЛЬНОЕ ПОЛОЖЕНИЕ
    ball.x = ball.rad + rand() % (window.width - (int)ball.width);
    ball.y = ball.rad + rand() % (window.height / 2);

    // Случайный угол от 0 до 360 градусов
    float angle = (rand() % 360) * 3.14159f / 180.0f;  // преобразуем в радианы

    // Вычисляем направление через синус и косинус
    ball.dx = cos(angle) * ball.speed;
    ball.Grav = sin(angle) * ball.speed;

    isBallActive = true;  // шар активен

    game.score = 0;
    game.balls = 9;
}


void MoveBall() {// ДВИЖЕНИЯ ШАРА
    if (!isBallActive) return;

    ball.x += ball.dx;
    ball.y += ball.Grav;

    // ОТСКОК ОТ ГРАНИЦ
    if (ball.x < ball.rad) { ball.x = ball.rad; ball.dx = -ball.dx; }
    if (ball.x > window.width - ball.rad) { ball.x = window.width - ball.rad; ball.dx = -ball.dx; }
    if (ball.y < ball.rad) { ball.y = ball.rad; ball.Grav = -ball.Grav; }
    if (ball.y > window.height - ball.rad) { ball.y = window.height - ball.rad; ball.Grav = -ball.Grav; }
}

// ФУНКЦИЯ ПРОВЕРКИ СТОЛКНОВЕНИЯ ШАРА С РАКЕТКОЙ
void CheckBallRacketCollision() {
    if (!isBallActive) return;  // если шар не активен, выходим

    float racketLeft = racket.x;
    float racketRight = racket.x + racket.width;
    float racketTop = racket.y;
    float racketBottom = racket.y + racket.height;

    float ballLeft = ball.x - ball.rad;
    float ballRight = ball.x + ball.rad;
    float ballTop = ball.y - ball.rad;
    float ballBottom = ball.y + ball.rad;


    if (racketLeft <= ballRight &&
        racketRight >= ballLeft &&
        racketTop <= ballBottom &&
        racketBottom >= ballTop) {

      
        isBallActive = false;

        //// ДОБОВЛЯЕМ ОЧКИ
        //game.score += 100;

   
    }
}

void ShowScore()
{
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];
    _itoa_s(game.score, txt, 10);
    TextOutA(window.context, 10, 10, "Score", 5);
    TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));

    _itoa_s(game.balls, txt, 10);
    TextOutA(window.context, 10, 100, "Balls", 5);
    TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));

    SelectObject(window.context, hTmp);  // восстанавливаем старый шрифт
    DeleteObject(hFont);                 // удаляем созданный шрифт
}

void ProcessInput()
{
    if (GetAsyncKeyState(VK_LEFT)) racket.x -= racket.speed;
    if (GetAsyncKeyState(VK_RIGHT)) racket.x += racket.speed;

    racket.y += racket.Grav;


    // Прыжок при нажатии пробела
    if (GetAsyncKeyState(VK_SPACE) && !racket.isJumping) {
        racket.isJumping = true;
        racket.Grav = -racket.jumpSpeed;
    }
   /* if (racket.y > window.height - racket.height) {
        racket.y = window.height - racket.height;
        racket.isJumping = false;
        racket.Grav = 0;
    }*/

}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(hDC);
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);

    if (hOldbm)
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm);

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
        }

        SelectObject(hMemDC, hOldbm);
    }

    DeleteDC(hMemDC);
}

void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//задний фон
    ShowBitmap(window.context, (int)racket.x, (int)racket.y, (int)racket.width, (int)racket.height, racket.hBitmap);// 

    // ОТРИСОВКА ВСЕХ ПЛАТФОРМ
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        ShowBitmap(window.context, (int)platforms[i].x, (int)platforms[i].y,
            (int)platforms[i].width, (int)platforms[i].height, platforms[i].hBitmap);
    }

    // ОТРИСОВКА ШАРА (только если активен)
    if (isBallActive) {
        ShowBitmap(window.context, (int)(ball.x - ball.rad), (int)(ball.y - ball.rad),
            (int)ball.width, (int)ball.height, ball.hBitmap, true);
    }

    //ShowBitmap(window.context, enemy.x - racket.width / 2, 0, racket.width, racket.height, enemy.hBitmap);//
}

void LimitRacket()
{
    racket.x = max(racket.x, 0.0f);
    racket.x = min(racket.x, window.width - racket.width);
}

//bool Collision(sprite a, sprite b)
//{
//    // Учитываем центрирование ракетки
//    float racketLeft = racket.x - a.width / 2;
//    float racketRight = racket.x + a.width / 2;
//    float racketTop = a.y;
//    float racketBottom = a.y + a.height;
//
//    float platformLeft = b.x;
//    float platformRight = b.x + b.width;
//    float platformTop = b.y;
//    float platformBottom = b.y + b.height;
//
//    return (racketLeft < platformRight &&
//        racketRight > platformLeft &&
//        racketTop < platformBottom &&
//        racketBottom > platformTop);
//}

bool HelpColl(sprite a, sprite b)
{
     if (a.x + a.width >= b.x  &&
         a.x <= b.x + b.width &&
         a.y + a.height >= b.y &&
         a.y <= b.y + b.height)

         return true;

        else

            return false;
}

void Coll() {
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        if (HelpColl(racket, platforms[i])) {

            float UP = abs((racket.y + racket.height) - platforms[i].y);
            float DOWN = abs(racket.y - (platforms[i].y + platforms[i].height));
            float LEFT = abs((racket.x + racket.width) - platforms[i].x);
            float RIGHT = abs(racket.x - (platforms[i].x + platforms[i].width));

            float overY = min(UP, DOWN);
            float overX = min(LEFT, RIGHT);

            if (overY < overX) {

                if (UP < DOWN) {
                    racket.y = platforms[i].y - racket.height;
                    racket.isJumping = false;
                    racket.Grav = 0;
                }
                else {
                    racket.y = platforms[i].y + platforms[i].height;
                    racket.Grav = 0.5f;
                }
            }
            else {
               
                if (LEFT < RIGHT) {
                    racket.x = platforms[i].x - racket.width;    
                }
                else {
                    racket.x = platforms[i].x + platforms[i].width;  
                }
            }
        }
    }
}

 
void BallColl() {
 
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        if (HelpColl(ball, platforms[i])) {

            float UP = abs((ball.y + ball.height) - platforms[i].y);
            float DOWN = abs(ball.y - (platforms[i].y + platforms[i].height));
            float LEFT = abs((ball.x + ball.width) - platforms[i].x);
            float RIGHT = abs(ball.x - (platforms[i].x + platforms[i].width));

            float overY = min(UP, DOWN);
            float overX = min(LEFT, RIGHT);

            if (overY < overX) {

                if (UP < DOWN) {
                    ball.y = platforms[i].y - ball.height;
                    ball.Grav = -ball.Grav;
                }
                else {
                    ball.y = platforms[i].y + platforms[i].height;
                    ball.Grav = -ball.Grav;  
                }
            }
            else {

                if (LEFT < RIGHT) {
                    ball.x = platforms[i].x - ball.width;
                    ball.dx = -ball.dx;
                }
                else {
                    ball.x = platforms[i].x + platforms[i].width;
                    ball.dx = -ball.dx;
                }
            }
        }
    }

}
    
    
    
    
    



//bool CheckCollision(float objectLeft, float objectTop, float objectRight, float objectBottom,
//    float platformLeft, float platformTop, float platformRight, float platformBottom) {
//    return (objectLeft < platformRight && objectRight > platformLeft &&
//        objectTop < platformBottom && objectBottom > platformTop);
//}
//
//// Функция для определения стороны столкновения
//int GetCollisionSide(float objectLeft, float objectTop, float objectRight, float objectBottom,
//    float platformLeft, float platformTop, float platformRight, float platformBottom) {
//
//    // Вычисляем перекрытия по осям
//    float overlapHorizontal = min(objectRight, platformRight) - max(objectLeft, platformLeft);
//    float overlapVertical = min(objectBottom, platformBottom) - max(objectTop, platformTop);
//
//    // Определяем сторону по меньшему перекрытию
//    if (overlapHorizontal < overlapVertical) {
//        // Боковое столкновение
//        return (objectLeft < platformLeft) ? 1 : 2; // 1 = слева, 2 = справа
//    }
//    else {
//        // Вертикальное столкновение  
//        return (objectTop < platformTop) ? 3 : 4; // 3 = сверху, 4 = снизу
//    }
//}
//
//void Collision2() {
//    float nextRacketPositionY = racket.y + racket.Grav;
//
//    for (int platformIndex = 0; platformIndex < PLATFORM_COUNT; platformIndex++) {
//        // Границы ракетки
//        float racketLeft = racket.x - racket.width / 2;
//        float racketRight = racket.x + racket.width / 2;
//        float racketTop = racket.y;
//        float racketBottom = racket.y + racket.height;
//
//        // Границы платформы
//        float platformLeft = platforms[platformIndex].x;
//        float platformRight = platforms[platformIndex].x + platforms[platformIndex].width;
//        float platformTop = platforms[platformIndex].y;
//        float platformBottom = platforms[platformIndex].y + platforms[platformIndex].height;
//
//        // Проверяем столкновения
//        bool currentCollision = CheckCollision(racketLeft, racketTop, racketRight, racketBottom,
//            platformLeft, platformTop, platformRight, platformBottom);
//        bool futureCollision = CheckCollision(racketLeft, nextRacketPositionY, racketRight, nextRacketPositionY + racket.height,
//            platformLeft, platformTop, platformRight, platformBottom);
//
//        if (currentCollision || futureCollision) {
//            // ОПРЕДЕЛЯЕМ СТОРОНУ СТОЛКНОВЕНИЯ
//            int collisionSide = GetCollisionSide(racketLeft, racketTop, racketRight, racketBottom,
//                platformLeft, platformTop, platformRight, platformBottom);
//
//            switch (collisionSide) {
//            case 1: // СТОЛКНОВЕНИЕ СЛЕВА
//                racket.x = platformLeft - racket.width / 2; // отталкиваем влево
//                break;
//
//            case 2: // СТОЛКНОВЕНИЕ СПРАВА  
//                racket.x = platformRight + racket.width / 2; // отталкиваем вправо
//                break;
//
//            case 3: // СТОЛКНОВЕНИЕ СВЕРХУ (падаем на платформу)
//                racket.y = platformTop - racket.height;
//                racket.isJumping = false;
//                racket.Grav = 0;
//                break;
//
//            case 4: // СТОЛКНОВЕНИЕ СНИЗУ (ударяемся головой)
//                racket.y = platformBottom;
//                racket.Grav = 0.5f; // начинаем падать
//                break;
//            }
//            break; // выходим после обработки первого столкновения
//        }
//    }
//}

//void CheckBallPlatformCollision() {
//    if (!isBallActive) return;
//
//    for (int platformIndex = 0; platformIndex < PLATFORM_COUNT; platformIndex++) {
//        // Границы шара
//        float ballLeft = ball.x - ball.rad;
//        float ballRight = ball.x + ball.rad;
//        float ballTop = ball.y - ball.rad;
//        float ballBottom = ball.y + ball.rad;
//
//        // Границы платформы
//        float platformLeft = platforms[platformIndex].x;
//        float platformRight = platforms[platformIndex].x + platforms[platformIndex].width;
//        float platformTop = platforms[platformIndex].y;
//        float platformBottom = platforms[platformIndex].y + platforms[platformIndex].height;
//
//        // Проверяем столкновение
//        bool ballCollision = TestColl(ballLeft, ballTop, ballRight, ballBottom,
//            platformLeft, platformTop, platformRight, platformBottom);
//
//        if (ballCollision) {
//            // ОПРЕДЕЛЯЕМ СТОРОНУ СТОЛКНОВЕНИЯ
//            int collisionSide = TestColl(ballLeft, ballTop, ballRight, ballBottom,
//                platformLeft, platformTop, platformRight, platformBottom);
//
//            // ОБРАБАТЫВАЕМ ОТСКОК ШАРА
//            switch (collisionSide) {
//            case 1: // СТОЛКНОВЕНИЕ СЛЕВА
//                ball.x = platformLeft - ball.rad;
//                ball.dx = -abs(ball.dx); // двигаем влево
//                break;
//
//            case 2: // СТОЛКНОВЕНИЕ СПРАВА  
//                ball.x = platformRight + ball.rad;
//                ball.dx = abs(ball.dx); // двигаем вправо
//                break;
//
//            case 3: // СТОЛКНОВЕНИЕ СВЕРХУ
//                ball.y = platformTop - ball.rad;
//                ball.Grav = -abs(ball.Grav); // отскакиваем вверх
//                break;
//
//            case 4: // СТОЛКНОВЕНИЕ СНИЗУ
//                ball.y = platformBottom + ball.rad;
//                ball.Grav = abs(ball.Grav); // отскакиваем вниз
//                break;
//            }
//
//            break; // выходим после первого столкновения
//        }
//    }
//}

void ProcessJumping() {
    // Гравитация применяется ВСЕГДА
    racket.Grav += 0.4f;  // постоянное ускорение вниз

    // Ограничиваем максимальную скорость падения
    if (racket.Grav > 10.0f) {
        racket.Grav = 10.0f;
    }

    // Применяем скорость к позиции
    racket.y += racket.Grav;

    // ПРОВЕРКА ЗЕМЛИ (низ экрана)
    if (racket.y > window.height - racket.height) {
        racket.y = window.height - racket.height;  
        racket.isJumping = false;  
        racket.Grav = 0;           
    }
}

void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);
    window.width = r.right - r.left;
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));
    GetClientRect(window.hWnd, &r);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    InitWindow();
    InitGame();
    ShowCursor(NULL);

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ProcessInput();
        ProcessJumping();
       
      /*  Collision2();*/
        BallColl();
        Coll();
        LimitRacket();
        MoveBall();                    // Движение шара
        CheckBallRacketCollision();// Проверка столкновения шара с ракеткой
       /* CheckBallPlatformCollision();*/ // Проверка столкновения шара с платформой
        ShowRacketAndBall();
        ShowScore();
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);
        Sleep(16);
    }
   
}