#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

typedef struct {
    float x, y, width, height, rad, dx, dy, speed;
    HBITMAP hBitmap;
} sprite;

sprite Moriko0;
sprite Moriko1;
sprite Moriko2;
sprite Moriko3;

// Структура для кнопки
struct Button {
    int x, y, width, height;
    int choiceIndex;
    bool hover;

    Button() : x(0), y(0), width(0), height(0), choiceIndex(0), hover(false) {}
    Button(int x_, int y_, int w, int h, int idx)
        : x(x_), y(y_), width(w), height(h), choiceIndex(idx), hover(false) {
    }
};

struct {
    int score, balls;
    bool action = false;
    int currentTextIndex = 0;
    bool inDialog = false;
    int currentDialogIndex = 0;
    std::vector<Button> currentButtons;
    int currentBackground = 0;
} game;

struct {
    HWND hWnd;
    HDC device_context, context;
    int width, height;
} window;

HBITMAP hBack;
HBITMAP hBack1;
HBITMAP hBack2;

void InitGame()
{
    Moriko0.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    Moriko1.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    Moriko2.hBitmap = (HBITMAP)LoadImageA(NULL, "defka.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    Moriko3.hBitmap = (HBITMAP)LoadImageA(NULL, "racket2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    hBack = (HBITMAP)LoadImageA(NULL, "vagon.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack1 = (HBITMAP)LoadImageA(NULL, "stanciya.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack2 = (HBITMAP)LoadImageA(NULL, "stanciy2a.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    Moriko0.width = 500;
    Moriko0.height = 800;
    Moriko0.speed = 30;
    Moriko0.x = window.width / 2.;
    Moriko0.y = window.height - Moriko0.height;

    Moriko1.y = Moriko1.x;
    Moriko1.height = 100;
    Moriko1.width = 300;
    Moriko1.x = window.width - Moriko1.width;

    Moriko2.x = 500;
    Moriko2.y = 890;
    Moriko2.width = 200;
    Moriko2.height = 400;

    Moriko3.y = Moriko1.x;
    Moriko3.height = 100;
    Moriko3.width = 300;
    Moriko3.x = window.width - Moriko1.width;

    game.score = 0;
    game.balls = 9;
    game.inDialog = false;
    game.currentDialogIndex = 0;
    game.currentBackground = 0;
}

void ShowText(int x, int y, std::string text, int count)
{
    SetTextColor(window.context, RGB(255, 0, 0));
    SetBkMode(window.context, TRANSPARENT);

    HFONT hFont = CreateFont(60, 0, 0, 0, FW_BOLD, 0, 0, 0,
        RUSSIAN_CHARSET, 0, 0, 2, 0, "Arial");
    HFONT hOldFont = (HFONT)SelectObject(window.context, hFont);

    TextOutA(window.context, x, y, text.c_str(), count);

    SelectObject(window.context, hOldFont);
    DeleteObject(hFont);
}

void ProcessInput()
{
    if (GetAsyncKeyState(VK_LEFT)) Moriko0.x -= Moriko0.speed;
    if (GetAsyncKeyState(VK_RIGHT)) Moriko0.x += Moriko0.speed;
}

void input() {
    static bool wasPressed = false;

    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        if (!wasPressed) {
            wasPressed = true;
        }
    }
    else {
        wasPressed = false;
    }
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

void ShowBackground() {
    switch (game.currentBackground) {
    case 0:
        ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);
        break;
    case 1:
        ShowBitmap(window.context, 0, 0, window.width, window.height, hBack1);
        break;
    case 2:
        ShowBitmap(window.context, 0, 0, window.width, window.height, hBack2);
        break;
    default:
        ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);
        break;
    }
}

void ShowRacketAndBall()
{
    ShowBackground();

    ShowBitmap(window.context, Moriko0.x - Moriko0.width / 2., Moriko0.y, Moriko0.width, Moriko0.height, Moriko0.hBitmap);
    ShowBitmap(window.context, Moriko1.x, Moriko1.y, Moriko1.width, Moriko1.height, Moriko1.hBitmap);
    ShowBitmap(window.context, 100, Moriko3.y - Moriko3.rad, 2 * Moriko3.rad, 2 * Moriko3.rad, Moriko3.hBitmap, true);
}

struct Answer {
    std::string text;
    int nextDialog;

    Answer() : text(""), nextDialog(-1) {}
    Answer(const std::string& t, int next) : text(t), nextDialog(next) {}
};

struct Dialog {
    std::string speaker;
    std::string text;
    std::vector<Answer> variants;
    int background;

    Dialog(const std::string& spk, const std::string& t) : speaker(spk), text(t), background(-1) {}
    Dialog(const std::string& spk, const std::string& t, int bg) : speaker(spk), text(t), background(bg) {}
};

struct NarrativeText {
    std::string text;
    int background;

    NarrativeText() : text(""), background(-1) {}
    NarrativeText(const std::string& t) : text(t), background(-1) {}
    NarrativeText(const std::string& t, int bg) : text(t), background(bg) {}
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

    narratives.push_back({ "До работы больше времени, чем я рассчитывал.", 1 });
    narratives.push_back({ "Легкий запах кофе приближает меня к кофейне в европейском стиле.", 2 });
    narratives.push_back({ "Никаких сомнений, чашка кофе перед первым рабочим днем, то без чего я не согласен работать.", 2 });
    narratives.push_back({ "Я захожу и разносится звон колокольчика.", 2 });
    narratives.push_back({ "Теплая атмосфера кофейни будто останавливает время, я единственный посетитель, хочется остаться в этом раю из темного дерева и мягких кресел, в конце зала виднеется небольшая сцена, уже представляю, как здесь вечерами играют джаз.", 2 });
    narratives.push_back({ "За массивной длинной барной стойкой девушка в классическом костюме с жакетом, приближается ко мне, нужно сделать музыку тише, она хочет поприветствовать меня.", 2 });
    narratives.push_back({ "Она показывает на меню напечатанное, и проиллюстрированное от руки.", 2 });
    narratives.push_back({ "Из того, что мне известно американо, капучино, латте, целая витрина бисквитных сладостей манит, но на десерт нет времени.", 2 });
    narratives.push_back({ "*Если музыку выключил слышит легкий джаз, в кофейни играет пластинка.", 2 });
    narratives.push_back({ "Герой сидит за барной стойкой, Рэра увлеченно готовит кофе, этот процесс поглотил ее, ни один мускул на лице не шелохнулся, все внимание на ритуале.", 2 });
    narratives.push_back({ "Она подает кофе в белой кружке на блюдце, небольшое печенье в форме искры.", 2 });
    narratives.push_back({ "Пробую первым делом кофе, Рэра терпеливо ждет и наблюдает за моей реакцией.", 2 });

    Dialog d0("Рэра", "Ты всё ещё слушаешь музыку, когда не хочешь говорить, сделай хотя бы потише?", 2);
    d0.variants.push_back(Answer("Капучино", 1));
    d0.variants.push_back(Answer("Американо", 2));
    d0.variants.push_back(Answer("Латте. И пирожное — в подарок тебе.", 3));
    dialogs.push_back(d0);

    Dialog d1("ГГ", "Пена мягко касается губ. Вкус ровный.", 2);
    d1.variants.push_back(Answer("Продолжить", 4));
    dialogs.push_back(d1);

    Dialog d4("ГГ", "- Хороший баланс.", 1);
    d4.variants.push_back(Answer("Далее", 5));
    dialogs.push_back(d4);

    Dialog d5("Рэра", "Она чуть улыбается. В её глазах мелькает удовлетворение. - Рада, что оценил, не буду тебя беспокоить.", 2);
    d5.variants.push_back(Answer("Далее", 6));
    dialogs.push_back(d5);

    Dialog d6("ГГ", "Наслаждаюсь печеньем, оно немного подгоревшее, но это его не портит.", 2);
    d6.variants.push_back(Answer("Далее", 7));
    dialogs.push_back(d6);

    Dialog d7("Рэра", "- Вот ваш счёт, господин ГГ. - Удачного тебе дня! Возвращайся!", 2);
    d7.variants.push_back(Answer("Спасибо, рад был повидаться!", -1));
    dialogs.push_back(d7);

    Dialog d2("ГГ", "Рэра приносит и уходит заниматься своими делами. Горечь ударяет сразу. Слишком прямо. Зато бодрит.", 2);
    d2.variants.push_back(Answer("Продолжить", 8));
    dialogs.push_back(d2);

    Dialog d8("ГГ", "Бариста заговорчески улыбается и поглядывает с конца барной стойки. Я знаю, что она специально сделала его крепче.", 2);
    d8.variants.push_back(Answer("Далее", 9));
    dialogs.push_back(d8);

    Dialog d9("ГГ", "Заедаю горечь печеньем, оно немного подгоревшее. Слишком много сегодня издевательств со стороны женщин, поскорей бы на завод.", 2);
    d9.variants.push_back(Answer("Далее", 10));
    dialogs.push_back(d9);

    Dialog d10("Рэра", "- Вот ваш счёт. - Хорошего дня!", 2);
    d10.variants.push_back(Answer("Спасибо, и тебе!", -1));
    dialogs.push_back(d10);

    Dialog d3("ГГ", "Молоко смягчает вкус, но не прячет кофе. Кофе теплый, будто обнимает.", 2);
    d3.variants.push_back(Answer("Далее", 11));
    dialogs.push_back(d3);

    Dialog d11("ГГ", "Я делаю еще глоток и медлю, прежде чем ответить. - Похоже что ты стала настоящим профессионалом.", 2);
    d11.variants.push_back(Answer("Далее", 12));
    dialogs.push_back(d11);

    Dialog d12("Рэра", "Пауза. Её пальцы касаются края блюдца. Взгляд становится мягче. - Тебе было скучно без меня, да?", 2);
    d12.variants.push_back(Answer("Далее", 13));
    dialogs.push_back(d12);

    Dialog d13("Рэра", "Она улыбается мне, но смотрит куда-то в сторону сцены. - Можем как нибудь встретиться, нужно отпраздновать твой выпуск из университета.", 2);
    d13.variants.push_back(Answer("Далее", 14));
    dialogs.push_back(d13);

    Dialog d14("ГГ", "Уже допиваю кофе, смотрю на часы. С Рэрой чувство, что никуда и не уезжал, она так беззаботно со мной общается, почему мне так неловко?", 2);
    d14.variants.push_back(Answer("Далее", 15));
    dialogs.push_back(d14);

    Dialog d15("ГГ", "Можем, думаю да, но я спешу на первый рабочий день. Ты наверное знаешь, здесь ниже по дороге NECO.", 2);
    d15.variants.push_back(Answer("Далее", 16));
    dialogs.push_back(d15);

    Dialog d16("Рэра", "Немного в сметении. - Да, знаю, твои будущие коллеги часто заходят. Думаю, ты как нибудь посетишь наш Джаз концерт вместе с ними. - Сейчас принесу счёт, подожди немного.", 2);
    d16.variants.push_back(Answer("Далее", 17));
    dialogs.push_back(d16);

    Dialog d17("ГГ", "Она будто немного расстроилась после слов о NECO или ее тронуло то, что мне уже пора идти. Почему я снова то и думаю о ней? Не все ли равно?", 2);
    d17.variants.push_back(Answer("Далее", 18));
    dialogs.push_back(d17);

    Dialog d18("Рэра", "- Вот ваш счёт, господин ГГ. - Удачного тебе рабочего дня! Возвращайся поскорее!", 2);
    d18.variants.push_back(Answer("Спасибо, рад был встречи!", 19));
    dialogs.push_back(d18);

    Dialog d19("ГГ", "Выхожу с кофейни и направляюсь в сторону завода, мне идти еще минут 15, думаю прибавить громкости в наушниках, что бы не слышать шумы города.", 2);
    dialogs.push_back(d19);

    currentDialogs.push_back(dialogs[0]);
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
}

void ShowCurrentNarrativeText() {
    if (!game.inDialog && game.currentTextIndex < narratives.size()) {
        if (narratives[game.currentTextIndex].background != -1) {
            game.currentBackground = narratives[game.currentTextIndex].background;
        }

        ShowText(100, 200, narratives[game.currentTextIndex].text,
            narratives[game.currentTextIndex].text.length());

        std::string counter = "Текст " + std::to_string(game.currentTextIndex + 1) +
            " из " + std::to_string(narratives.size());
        ShowText(100, 100, counter, counter.length());
    }
}

void UpdateButtonHover() {
    POINT mousePos;
    GetCursorPos(&mousePos);
    ScreenToClient(window.hWnd, &mousePos);

    for (auto& btn : game.currentButtons) {
        btn.hover = (mousePos.x >= btn.x && mousePos.x <= btn.x + btn.width &&
            mousePos.y >= btn.y && mousePos.y <= btn.y + btn.height);
    }
}

void HandleDialogMouseClick() {
    if (!game.inDialog) return;

    static bool wasPressed = false;

    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        if (!wasPressed) {
            POINT mousePos;
            GetCursorPos(&mousePos);
            ScreenToClient(window.hWnd, &mousePos);

            for (auto& btn : game.currentButtons) {
                if (mousePos.x >= btn.x && mousePos.x <= btn.x + btn.width &&
                    mousePos.y >= btn.y && mousePos.y <= btn.y + btn.height) {

                    Dialog& d = dialogs[game.currentDialogIndex];
                    if (btn.choiceIndex < (int)d.variants.size()) {
                        int nextDialog = d.variants[btn.choiceIndex].nextDialog;
                        if (nextDialog == -1) {
                            game.inDialog = false;
                            game.currentTextIndex++;
                        }
                        else if (nextDialog < (int)dialogs.size()) {
                            game.currentDialogIndex = nextDialog;
                            game.currentButtons.clear();
                        }
                    }
                    break;
                }
            }
            wasPressed = true;
        }
    }
    else {
        wasPressed = false;
    }
}

void ShowCurrentDialog() {
    if (game.inDialog && game.currentDialogIndex < dialogs.size()) {
        Dialog& d = dialogs[game.currentDialogIndex];

        if (d.background != -1) {
            game.currentBackground = d.background;
        }

        std::string speakerText = d.speaker + ":";
        ShowText(100, 150, speakerText, speakerText.length());

        ShowText(100, 220, d.text, d.text.length());

        if (!d.variants.empty()) {
            ShowText(100, 400, "Выберите вариант:", 18);

            game.currentButtons.clear();

            for (size_t i = 0; i < d.variants.size(); i++) {
                int yPos = 450 + (int)(i * 50);
                std::string choiceText = d.variants[i].text;

                Button btn(100, yPos, 600, 40, (int)i);
                game.currentButtons.push_back(btn);

                RECT rect = { 100, yPos, 700, yPos + 40 };
                HPEN pen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
                HPEN oldPen = (HPEN)SelectObject(window.context, pen);
                HBRUSH oldBrush = (HBRUSH)SelectObject(window.context, GetStockObject(NULL_BRUSH));
                Rectangle(window.context, rect.left, rect.top, rect.right, rect.bottom);
                SelectObject(window.context, oldPen);
                SelectObject(window.context, oldBrush);
                DeleteObject(pen);

                if (game.currentButtons.back().hover) {
                    HBRUSH brush = CreateSolidBrush(RGB(80, 80, 80));
                    FillRect(window.context, &rect, brush);
                    DeleteObject(brush);
                }

                ShowText(120, yPos, choiceText, choiceText.length());
            }
        }
        else {
            ShowText(100, 500, "[Пробел] Продолжить", 20);
        }
    }
}

void HandleMouseClicks() {
    static bool wasPressed = false;

    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        if (!wasPressed) {
            if (!game.inDialog && game.currentTextIndex < narratives.size()) {
                game.currentTextIndex++;
                if (game.currentTextIndex >= narratives.size()) {
                    game.currentTextIndex = narratives.size() - 1;
                }

                // ДОБАВЛЯЕМ: обновляем фон из текущего нарратива
                if (narratives[game.currentTextIndex].background != -1) {
                    game.currentBackground = narratives[game.currentTextIndex].background;
                }

                // ДОБАВЛЯЕМ: перерисовываем фон
                ShowBackground();

                // Рисуем спрайты поверх фона
                ShowBitmap(window.context, Moriko0.x - Moriko0.width / 2., Moriko0.y, Moriko0.width, Moriko0.height, Moriko0.hBitmap);
                ShowBitmap(window.context, Moriko2.x, Moriko2.y, Moriko2.width, Moriko2.height, Moriko2.hBitmap);
                ShowBitmap(window.context, Moriko1.x, Moriko1.y, Moriko1.width, Moriko1.height, Moriko1.hBitmap);
                ShowBitmap(window.context, 100, Moriko3.y - Moriko3.rad, 2 * Moriko3.rad, 2 * Moriko3.rad, Moriko3.hBitmap, true);

                ShowText(100, 200, narratives[game.currentTextIndex].text,
                    narratives[game.currentTextIndex].text.length());

                std::string counter = "Текст " + std::to_string(game.currentTextIndex + 1) +
                    " из " + std::to_string(narratives.size());
                ShowText(100, 100, counter, counter.length());
            }
            wasPressed = true;
        }
    }
    else {
        wasPressed = false;
    }
}

void Collise() {
    if (Moriko2.x == Moriko0.x + Moriko0.width) {
        Moriko0.x = 100;
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    InitWindow();
    InitGame();
    initContentData();
    ShowCursor(TRUE);

    game.inDialog = true;
    game.currentDialogIndex = 0;

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        setlocale(LC_ALL, "");
        ShowRacketAndBall();

        if (game.inDialog) {
            UpdateButtonHover();
            ShowCurrentDialog();
            HandleDialogMouseClick();
            // Убрана функция HandleDialogInput()
        }
        else {
            ShowCurrentNarrativeText();
        }

        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);
        Sleep(16);
        Collise();
        input();
        HandleMouseClicks();
        ProcessInput();
    }
}