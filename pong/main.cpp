#include <windows.h>
#include <iostream>
#include <vector>
#include <string>


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
    int currentTextIndex = 0;
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



void ShowText(int x, int y , std::string text, int count)
{
    SetTextColor(window.context, RGB(255, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
  
    HFONT hFont = CreateFont(60, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "Arial");
    HFONT hOldFont = (HFONT)SelectObject(window.context, hFont);
  
    SelectObject(window.context, hOldFont);
  
    TextOutA(window.context, x, y, text.c_str(), count);
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


struct Answer {
    std::string text;
    int nextDialog;

    Answer() : text(""), nextDialog(-1) {}
    Answer(const std::string& t, int next)
        : text(t), nextDialog(next) {
    }
};

struct Dialog {
    std::string speaker;
    std::string text;
    std::vector<Answer> variants;

    Dialog(const std::string& spk, const std::string& t)
        : speaker(spk), text(t) {
    }
};

struct NarrativeText {
    std::string text;

    NarrativeText() : text("") {}
    NarrativeText(const std::string& t) : text(t) {}
};

std::vector<NarrativeText> narratives;
std::vector<Dialog> dialogs;
std::vector<Dialog> currentDialogs;

bool initDialogs = false;

void initContentData() {

   
    if (initDialogs) return;
    initDialogs = true;

    narratives.clear();
    dialogs.clear();
    currentDialogs.clear();

    //Нарратив

    narratives.push_back({ "До работы больше времени, чем я рассчитывал." });

    narratives.push_back({ "Легкий запах кофе приблежают меня к кофейне в европейском стиле." });

    narratives.push_back({ "Никаких сомнений, чашка кофе перед первым рабочим днем, то без чего я не согласен работать." });

    narratives.push_back({ "Я захожу и разносится звон колокольчика." });

    narratives.push_back({ "Теплая атмосфера кофейни будто останавливает время, я единственный посетитель, хочется остаться в этом раю из темного дерева и мягких кресел, в конце зала виднеется небольшая сцена, уже представляю, как здесь вечерами играют джаз." });

    narratives.push_back({ "За массивной длинной барной стойкой девушка в классическом костюме с жакетом, приближается ко мне, нужно сделать музыку тише, она хочет поприветствовать меня." });

    narratives.push_back({ "Она показывает на меню напечатанное, и проиллюстрированное от руки." });

    narratives.push_back({ "Из того, что мне известно американо, капучино, латте, целая витрина бисквитных сладостей манит, но на десерт нет времени." });

    narratives.push_back({ "*Если музыку выключил слышит легкий джаз, в кофейни играет пластинка." });

    narratives.push_back({ "Герой сидит за барной стойкой, Рэра увлеченно готовит кофе, этот процесс поглотил ее, ни один мускул на лице не шелохнулся, все внимание на ритуале." });

    narratives.push_back({ "Она подает кофе в белой кружке на блюдце, небольшое печенье в форме искры." });

    narratives.push_back({ "Пробую первым делом кофе, Рэра терпеливо ждет и наблюдает за моей реакцией." });

   
    
    //Диалоги 

    Dialog d0("Рэра",
        "Ты всё ещё слушаешь музыку, когда не хочешь говорить, сделай хотя бы потише?");

    d0.variants.push_back(Answer("Капучино", 1));
    d0.variants.push_back(Answer("Американо", 2));
    d0.variants.push_back(Answer("Латте. И пирожное — в подарок тебе.", 3));

    dialogs.push_back(d0);


    //Выбор "Капучино"

    Dialog d1("ГГ", "Пена мягко касается губ. Вкус ровный.");
    d1.variants.push_back(Answer("Продолжить", 4));
    dialogs.push_back(d1);

    Dialog d4("ГГ", "- Хороший баланс.");
    d4.variants.push_back(Answer("Далее", 5));
    dialogs.push_back(d4);

    Dialog d5("Рэра", "Она чуть улыбается. В её глазах мелькает удовлетворение. - Рада, что оценил, не буду тебя беспокоить.");
    d5.variants.push_back(Answer("Далее", 6));
    dialogs.push_back(d5);

    Dialog d6("ГГ", "Наслаждаюсь печеньем, оно немного подгоревшее, но это его не портит.");
    d6.variants.push_back(Answer("Далее", 7));
    dialogs.push_back(d6);

    Dialog d7("Рэра", "- Вот ваш счёт, господин ГГ. - Удачного тебе дня! Возвращайся!");
    d7.variants.push_back(Answer("Спасибо, рад был повидаться!", -1));
    dialogs.push_back(d7);


    //Выбор "Американо"

    Dialog d2("ГГ", "Рэра приносит и уходит заниматься своими делами. Горечь ударяет сразу. Слишком прямо. Зато бодрит.");
    d2.variants.push_back(Answer("Продолжить", 8));
    dialogs.push_back(d2);

    Dialog d8("ГГ", "Бариста заговорчески улыбается и поглядывает с конца барной стойки. Я знаю, что она специально сделала его крепче.");
    d8.variants.push_back(Answer("Далее", 9));
    dialogs.push_back(d8);

    Dialog d9("ГГ", "Заедаю горечь печеньем, оно немного подгоревшее. Слишком много сегодня издевательств со стороны женщин, поскорей бы на завод.");
    d9.variants.push_back(Answer("Далее", 10));
    dialogs.push_back(d9);

    Dialog d10("Рэра", "- Вот ваш счёт. - Хорошего дня!");
    d10.variants.push_back(Answer("Спасибо, и тебе!", -1));
    dialogs.push_back(d10);


    //Выбор "Латте"

    Dialog d3("ГГ", "Молоко смягчает вкус, но не прячет кофе. Кофе теплый, будто обнимает.");
    d3.variants.push_back(Answer("Далее", 11));
    dialogs.push_back(d3);

    Dialog d11("ГГ", "Я делаю еще глоток и медлю, прежде чем ответить. - Похоже что ты стала настоящим профессионалом.");
    d11.variants.push_back(Answer("Далее", 12));
    dialogs.push_back(d11);

    Dialog d12("Рэра", "Пауза. Её пальцы касаются края блюдца. Взгляд становится мягче. - Тебе было скучно без меня, да?");
    d12.variants.push_back(Answer("Далее", 13));
    dialogs.push_back(d12);

    Dialog d13("Рэра", "Она улыбается мне, но смотрит куда-то в сторону сцены. - Можем как нибудь встретиться, нужно отпраздновать твой выпуск из университета.");
    d13.variants.push_back(Answer("Далее", 14));
    dialogs.push_back(d13);

    Dialog d14("ГГ", "Уже допиваю кофе, смотрю на часы. С Рэрой чувство, что никуда и не уезжал, она так беззаботно со мной общается, почему мне так неловко?");
    d14.variants.push_back(Answer("Далее", 15));
    dialogs.push_back(d14);

    Dialog d15("ГГ", "Можем, думаю да, но я спешу на первый рабочий день. Ты наверное знаешь, здесь ниже по дороге NECO.");
    d15.variants.push_back(Answer("Далее", 16));
    dialogs.push_back(d15);

    Dialog d16("Рэра", "Немного в сметении. - Да, знаю, твои будущие коллеги часто заходят. Думаю, ты как нибудь посетишь наш Джаз концерт вместе с ними. - Сейчас принесу счёт, подожди немного.");
    d16.variants.push_back(Answer("Далее", 17));
    dialogs.push_back(d16);

    Dialog d17("ГГ", "Она будто немного расстроилась после слов о NECO или ее тронуло то, что мне уже пора идти. Почему я снова то и думаю о ней? Не все ли равно?");
    d17.variants.push_back(Answer("Далее", 18));
    dialogs.push_back(d17);

    Dialog d18("Рэра", "- Вот ваш счёт, господин ГГ. - Удачного тебе рабочего дня! Возвращайся поскорее!");
    d18.variants.push_back(Answer("Спасибо, рад был встречи!", 19));
    dialogs.push_back(d18);

    Dialog d19("ГГ", "Выхожу с кофейни и направляюсь в сторону завода, мне идти еще минут 15, думаю прибавить громкости в наушниках, что бы не слышать шумы города.");
    dialogs.push_back(d19);
  
    /*ShowText(window.width/2, window.height/2, narratives[0].text , 44);
    ShowText(window.width / 2, 200, narratives[1].text, 64);
    ShowText(100, 150, narratives[2].text, narratives[2].text.length());
    ShowText(100, 200, narratives[3].text, narratives[3].text.length());
    ShowText(100, 250, narratives[4].text, narratives[4].text.length());
    ShowText(100, 300, narratives[5].text, narratives[5].text.length());
    ShowText(100, 350, narratives[6].text, narratives[6].text.length());
    ShowText(100, 400, narratives[7].text, narratives[7].text.length());
    ShowText(100, 450, narratives[8].text, narratives[8].text.length());
    ShowText(100, 500, narratives[9].text, narratives[9].text.length());
    ShowText(100, 550, narratives[10].text, narratives[10].text.length());
    ShowText(100, 600, narratives[11].text, narratives[11].text.length());*/
    currentDialogs.push_back(dialogs[0]);
   
}


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
void ShowCurrentNarrativeText() {
    if (game.currentTextIndex < narratives.size()) {
        // Показываем текущий текст
        ShowText(100, 200, narratives[game.currentTextIndex].text,
            narratives[game.currentTextIndex].text.length());

        // Показываем счетчик (необязательно)
        std::string counter = "Текст " + std::to_string(game.currentTextIndex + 1) +
            " из " + std::to_string(narratives.size());
        ShowText(100, 100, counter, counter.length());
    }
}

void HandleMouseClicks() {
    static bool wasPressed = false;

    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        if (!wasPressed) {
            // 1. Увеличиваем индекс
            game.currentTextIndex++;
            if (game.currentTextIndex >= narratives.size()) {
                game.currentTextIndex = narratives.size() - 1;
            }

            // 2. *** ПЕРЕРИСОВЫВАЕМ ФОН ПРЯМО СЕЙЧАС ***
            if (GetAsyncKeyState(VK_LBUTTON)) {
                ShowBitmap(window.context, 0, 0, window.width, window.height, hBack1);
            }
            else {
                ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);
            }

            // 3. Перерисовываем все спрайты
            ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);
            ShowBitmap(window.context, platform.x, platform.y, platform.width, platform.height, platform.hBitmap);
            ShowBitmap(window.context, enemy.x, enemy.y, enemy.width, enemy.height, enemy.hBitmap);
            ShowBitmap(window.context, 100, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);

            // 4. Рисуем НОВЫЙ текст
            ShowText(100, 200, narratives[game.currentTextIndex].text,
                narratives[game.currentTextIndex].text.length());

            std::string counter = "Текст " + std::to_string(game.currentTextIndex + 1) +
                " из " + std::to_string(narratives.size());
            ShowText(100, 100, counter, counter.length());

            wasPressed = true;
        }
    }
    else {
        wasPressed = false;
    }
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
        setlocale(LC_ALL, "");
        ShowRacketAndBall();//рисуем фон, ракетку и шарик
        initContentData();

        ShowCurrentNarrativeText(); // <-- Добавьте эту строку

        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);
        Sleep(16);
        Collise();
        input();
        HandleMouseClicks(); // <-- Добавьте эту строку
        ProcessInput();
        LimitRacket();
    }


    //while (!GetAsyncKeyState(VK_ESCAPE))
    //{
    //    
    //   
    //    setlocale(LC_ALL, "");
    //    ShowRacketAndBall();//рисуем фон, ракетку и шарик
    //    initContentData();
    //    HandleMouseClicks();
    //    ShowCurrentNarrativeText();
    //    
    //    //ShowScore();//рисуем очик и жизни
    //    BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
    //    Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)
    //    Collise();
    //    input();
    //    ProcessInput();//опрос клавиатуры
    //    LimitRacket();//проверяем, чтобы ракетка не убежала за экран

    //}

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