#include <graphics.h>
#include <conio.h>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <cstdio>
#include <windows.h>
#include "manager.h"
#include "manageBook.h"
#include "manageBorrowrecord.h"
#include "manageTime.h"
#include "reader.h"
#include "book.h"

using namespace std;

// 使用 main.cpp 中定义的全局对象
extern Manager manager;
extern ManageBook bookmanager;
extern ManageBorrowRecord recordMgr;

// ==================== 常量定义 ====================
const int WIN_WIDTH = 1000;
const int WIN_HEIGHT = 700;
const int NAV_WIDTH = 180;
const int HEADER_HEIGHT = 60;
const COLORREF COLOR_BG = RGB(240, 242, 245);
const COLORREF COLOR_NAV = RGB(45, 55, 72);
const COLORREF COLOR_HEADER = RGB(66, 153, 225);
const COLORREF COLOR_BTN = RGB(56, 161, 105);
const COLORREF COLOR_BTN_HOVER = RGB(72, 187, 120);
const COLORREF COLOR_BTN_DANGER = RGB(229, 62, 62);
const COLORREF COLOR_WHITE = RGB(255, 255, 255);
const COLORREF COLOR_TEXT = RGB(45, 55, 72);
const COLORREF COLOR_BORDER = RGB(203, 213, 224);

// 当前页面枚举
enum Page { PAGE_HOME, PAGE_BOOK, PAGE_READER, PAGE_BORROW, PAGE_FILE };
Page currentPage = PAGE_HOME;

// 滚动偏移
int scrollOffset = 0;

// ==================== 工具函数 ====================
// GBK string 转宽字符（EasyX 默认使用 GBK）
static wstring s2ws(const string& s) {
    if (s.empty()) return wstring();
    int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, nullptr, 0);
    wstring ws(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, &ws[0], len);
    if (!ws.empty() && ws.back() == L'\0') ws.pop_back();
    return ws;
}

// 判断点是否在矩形内
static bool ptInRect(int px, int py, int x, int y, int w, int h) {
    return px >= x && px <= x + w && py >= y && py <= y + h;
}

// 绘制圆角矩形按钮
static void drawRoundBtn(int x, int y, int w, int h, const wchar_t* text, COLORREF bgColor, COLORREF textColor = COLOR_WHITE) {
    setfillcolor(bgColor);
    setlinecolor(bgColor);
    solidroundrect(x, y, x + w, y + h, 8, 8);
    settextcolor(textColor);
    setbkmode(TRANSPARENT);
    int tw = textwidth(text);
    int th = textheight(text);
    outtextxy(x + (w - tw) / 2, y + (h - th) / 2, text);
}

// 绘制输入框
static void drawInputBox(int x, int y, int w, int h, const wchar_t* label, const wstring& value, bool focused = false) {
    settextcolor(COLOR_TEXT);
    setbkmode(TRANSPARENT);
    outtextxy(x, y - 22, label);
    setfillcolor(COLOR_WHITE);
    setlinecolor(focused ? COLOR_HEADER : COLOR_BORDER);
    solidrectangle(x, y, x + w, y + h);
    rectangle(x, y, x + w, y + h);
    settextcolor(COLOR_TEXT);
    outtextxy(x + 8, y + (h - textheight(L"A")) / 2, value.c_str());
}

// 消息提示框
static string g_message = "";
static COLORREF g_msgColor = COLOR_BTN;
static void showMessage(const string& msg, COLORREF color = COLOR_BTN) {
    g_message = msg;
    g_msgColor = color;
}

// 从 manager 获取所有读者
static vector<Reader>& getAllReaders() {
    return manager.rd;
}

// ==================== 绘制导航栏 ====================
static void drawNavigation() {
    // 导航背景
    setfillcolor(COLOR_NAV);
    solidrectangle(0, 0, NAV_WIDTH, WIN_HEIGHT);

    // 系统标题
    settextstyle(26, 0, _T("微软雅黑"));
    settextcolor(COLOR_WHITE);
    setbkmode(TRANSPARENT);
    outtextxy(20, 20, _T("图书管理系统"));

    // 导航按钮
    settextstyle(20, 0, _T("微软雅黑"));
    const wchar_t* navItems[] = { L"首  页", L"图书管理", L"读者管理", L"借还书", L"文件管理" };
    Page pages[] = { PAGE_HOME, PAGE_BOOK, PAGE_READER, PAGE_BORROW, PAGE_FILE };

    for (int i = 0; i < 5; i++) {
        int y = 80 + i * 50;
        if (currentPage == pages[i]) {
            setfillcolor(RGB(74, 85, 104));
            solidrectangle(0, y, NAV_WIDTH, y + 40);
            setfillcolor(COLOR_HEADER);
            solidrectangle(0, y, 4, y + 40);
        }
        settextcolor(COLOR_WHITE);
        outtextxy(30, y + 10, navItems[i]);
    }
}

// ==================== 绘制顶部栏 ====================
static void drawHeader(const wchar_t* title) {
    setfillcolor(COLOR_HEADER);
    solidrectangle(NAV_WIDTH, 0, WIN_WIDTH, HEADER_HEIGHT);
    settextstyle(28, 0, _T("微软雅黑"));
    settextcolor(COLOR_WHITE);
    setbkmode(TRANSPARENT);
    outtextxy(NAV_WIDTH + 30, 18, title);
}

// ==================== 首页 ====================
static void drawHomePage() {
    drawHeader(L"首页 - 系统概览");
    int cx = NAV_WIDTH + 50;
    int cy = HEADER_HEIGHT + 40;

    settextstyle(22, 0, _T("微软雅黑"));
    settextcolor(COLOR_TEXT);

    // 统计卡片
    int cardW = 200, cardH = 100;
    vector<Book>& books = bookmanager.allBooks();
    vector<Reader>& readers = getAllReaders();

    // 图书总数卡片
    setfillcolor(RGB(66, 153, 225));
    solidroundrect(cx, cy, cx + cardW, cy + cardH, 10, 10);
    settextcolor(COLOR_WHITE);
    settextstyle(20, 0, _T("微软雅黑"));
    outtextxy(cx + 20, cy + 15, L"图书总数");
    settextstyle(36, 0, _T("微软雅黑"));
    wchar_t buf[32];
    swprintf(buf, 32, L"%d", (int)books.size());
    outtextxy(cx + 20, cy + 45, buf);

    // 读者总数卡片
    setfillcolor(RGB(72, 187, 120));
    solidroundrect(cx + cardW + 30, cy, cx + cardW * 2 + 30, cy + cardH, 10, 10);
    settextcolor(COLOR_WHITE);
    settextstyle(20, 0, _T("微软雅黑"));
    outtextxy(cx + cardW + 50, cy + 15, L"读者总数");
    settextstyle(36, 0, _T("微软雅黑"));
    swprintf(buf, 32, L"%d", (int)readers.size());
    outtextxy(cx + cardW + 50, cy + 45, buf);

    // 借阅记录卡片
    setfillcolor(RGB(237, 137, 54));
    solidroundrect(cx + (cardW + 30) * 2, cy, cx + cardW * 3 + 60, cy + cardH, 10, 10);
    settextcolor(COLOR_WHITE);
    settextstyle(20, 0, _T("微软雅黑"));
    outtextxy(cx + (cardW + 30) * 2 + 20, cy + 15, L"借阅记录");
    settextstyle(36, 0, _T("微软雅黑"));
    swprintf(buf, 32, L"%d", (int)recordMgr.allRecords().size());
    outtextxy(cx + (cardW + 30) * 2 + 20, cy + 45, buf);

    // 欢迎信息
    cy += cardH + 50;
    settextstyle(24, 0, _T("微软雅黑"));
    settextcolor(COLOR_TEXT);
    outtextxy(cx, cy, L"欢迎使用图书管理系统！");

    settextstyle(18, 0, _T("微软雅黑"));
    settextcolor(RGB(113, 128, 150));
    outtextxy(cx, cy + 35, L"· 点击左侧菜单进行相应操作");
    outtextxy(cx, cy + 55, L"· 首次使用请先在「文件管理」中加载数据");
    outtextxy(cx, cy + 75, L"· 操作完成后请保存数据以防丢失");
}

// ==================== 图书管理页面 ====================
static int bookListScroll = 0;
static int selectedBookIdx = -1;
static wstring bookSearchKey = L"";

static void drawBookPage() {
    drawHeader(L"图书管理");
    int cx = NAV_WIDTH + 20;
    int cy = HEADER_HEIGHT + 20;

    // 操作按钮
    settextstyle(18, 0, _T("微软雅黑"));
    drawRoundBtn(cx, cy, 80, 35, L"添加", COLOR_BTN);
    drawRoundBtn(cx + 90, cy, 80, 35, L"删除", COLOR_BTN_DANGER);
    drawRoundBtn(cx + 180, cy, 80, 35, L"修改", RGB(237, 137, 54));
    drawRoundBtn(cx + 270, cy, 80, 35, L"查询", COLOR_HEADER);

    // 图书列表区域
    cy += 50;
    setfillcolor(COLOR_WHITE);
    solidrectangle(cx, cy, WIN_WIDTH - 20, WIN_HEIGHT - 20);
    setlinecolor(COLOR_BORDER);
    rectangle(cx, cy, WIN_WIDTH - 20, WIN_HEIGHT - 20);

    // 表头
    setfillcolor(RGB(237, 242, 247));
    solidrectangle(cx + 1, cy + 1, WIN_WIDTH - 21, cy + 35);
    settextcolor(COLOR_TEXT);
    settextstyle(18, 0, _T("微软雅黑"));
    outtextxy(cx + 15, cy + 10, L"ID");
    outtextxy(cx + 60, cy + 10, L"书名");
    outtextxy(cx + 250, cy + 10, L"作者");
    outtextxy(cx + 400, cy + 10, L"分类");
    outtextxy(cx + 520, cy + 10, L"总数");
    outtextxy(cx + 580, cy + 10, L"剩余");
    outtextxy(cx + 640, cy + 10, L"状态");

    // 图书列表（过滤已删除的）
    vector<Book>& allBooks = bookmanager.allBooks();
    vector<int> validIdx;
    for (int i = 0; i < (int)allBooks.size(); i++) {
        if (allBooks[i].getIsexisting()) validIdx.push_back(i);
    }
    int listY = cy + 40;
    int rowHeight = 30;
    int maxRows = (WIN_HEIGHT - 20 - listY) / rowHeight;

    settextstyle(16, 0, _T("微软雅黑"));
    for (int vi = bookListScroll; vi < (int)validIdx.size() && vi < bookListScroll + maxRows; vi++) {
        const Book& b = allBooks[validIdx[vi]];
        int rowY = listY + (vi - bookListScroll) * rowHeight;

        // 选中高亮
        if (validIdx[vi] == selectedBookIdx) {
            setfillcolor(RGB(235, 248, 255));
            solidrectangle(cx + 1, rowY, WIN_WIDTH - 21, rowY + rowHeight - 1);
        }

        // 斑马纹
        if ((vi - bookListScroll) % 2 == 1 && validIdx[vi] != selectedBookIdx) {
            setfillcolor(RGB(250, 250, 250));
            solidrectangle(cx + 1, rowY, WIN_WIDTH - 21, rowY + rowHeight - 1);
        }

        settextcolor(COLOR_TEXT);
        wchar_t buf[256];
        swprintf(buf, 256, L"%d", b.getID());
        outtextxy(cx + 15, rowY + 7, buf);
        outtextxy(cx + 60, rowY + 7, s2ws(b.getBookname()).c_str());
        outtextxy(cx + 250, rowY + 7, s2ws(b.getAuthor()).c_str());
        outtextxy(cx + 400, rowY + 7, s2ws(b.getCategory()).c_str());
        swprintf(buf, 256, L"%d", b.getTotal());
        outtextxy(cx + 520, rowY + 7, buf);
        swprintf(buf, 256, L"%d", b.getLeftnum());
        outtextxy(cx + 580, rowY + 7, buf);

        if (b.getIsborrowed()) {
            settextcolor(COLOR_BTN_DANGER);
            outtextxy(cx + 640, rowY + 7, L"已借出");
        }
        else {
            settextcolor(COLOR_BTN);
            outtextxy(cx + 640, rowY + 7, L"可借");
        }
    }

    // 提示信息（显示在查询按钮后面）
    if (!g_message.empty()) {
        settextcolor(g_msgColor);
        settextstyle(16, 0, _T("微软雅黑"));
        outtextxy(cx + 370, HEADER_HEIGHT + 28, s2ws(g_message).c_str());
    }
}

// ==================== 读者管理页面 ====================
static int readerListScroll = 0;
static int selectedReaderIdx = -1;

static void drawReaderPage() {
    drawHeader(L"读者管理");
    int cx = NAV_WIDTH + 20;
    int cy = HEADER_HEIGHT + 20;

    // 操作按钮
    settextstyle(18, 0, _T("微软雅黑"));
    drawRoundBtn(cx, cy, 80, 35, L"添加", COLOR_BTN);
    drawRoundBtn(cx + 90, cy, 80, 35, L"删除", COLOR_BTN_DANGER);
    drawRoundBtn(cx + 180, cy, 80, 35, L"修改", RGB(237, 137, 54));
    drawRoundBtn(cx + 270, cy, 80, 35, L"查询", COLOR_HEADER);

    // 读者列表区域
    cy += 50;
    setfillcolor(COLOR_WHITE);
    solidrectangle(cx, cy, WIN_WIDTH - 20, WIN_HEIGHT - 20);
    setlinecolor(COLOR_BORDER);
    rectangle(cx, cy, WIN_WIDTH - 20, WIN_HEIGHT - 20);

    // 表头
    setfillcolor(RGB(237, 242, 247));
    solidrectangle(cx + 1, cy + 1, WIN_WIDTH - 21, cy + 35);
    settextcolor(COLOR_TEXT);
    settextstyle(18, 0, _T("微软雅黑"));
    outtextxy(cx + 15, cy + 10, L"ID");
    outtextxy(cx + 70, cy + 10, L"用户名");
    outtextxy(cx + 200, cy + 10, L"真实姓名");
    outtextxy(cx + 350, cy + 10, L"联系方式");
    outtextxy(cx + 500, cy + 10, L"最大借书量");
    outtextxy(cx + 620, cy + 10, L"借书状态");

    // 读者列表（过滤已删除的）
    vector<Reader>& allReaders = getAllReaders();
    vector<int> validIdx;
    for (int i = 0; i < (int)allReaders.size(); i++) {
        if (allReaders[i].getIdExisting()) validIdx.push_back(i);
    }
    int listY = cy + 40;
    int rowHeight = 30;
    int maxRows = (WIN_HEIGHT - 20 - listY) / rowHeight;

    settextstyle(16, 0, _T("微软雅黑"));
    for (int vi = readerListScroll; vi < (int)validIdx.size() && vi < readerListScroll + maxRows; vi++) {
        const Reader& r = allReaders[validIdx[vi]];
        int rowY = listY + (vi - readerListScroll) * rowHeight;

        if (validIdx[vi] == selectedReaderIdx) {
            setfillcolor(RGB(235, 248, 255));
            solidrectangle(cx + 1, rowY, WIN_WIDTH - 21, rowY + rowHeight - 1);
        }

        if ((vi - readerListScroll) % 2 == 1 && validIdx[vi] != selectedReaderIdx) {
            setfillcolor(RGB(250, 250, 250));
            solidrectangle(cx + 1, rowY, WIN_WIDTH - 21, rowY + rowHeight - 1);
        }

        settextcolor(COLOR_TEXT);
        wchar_t buf[256];
        swprintf(buf, 256, L"%d", r.getID());
        outtextxy(cx + 15, rowY + 7, buf);
        outtextxy(cx + 70, rowY + 7, s2ws(r.getUsername()).c_str());
        outtextxy(cx + 200, rowY + 7, s2ws(r.getRealName()).c_str());
        outtextxy(cx + 350, rowY + 7, s2ws(r.getContact()).c_str());
        swprintf(buf, 256, L"%d", r.getMaxBorrowNum());
        outtextxy(cx + 530, rowY + 7, buf);

        if (r.getIsBorrowing()) {
            settextcolor(RGB(237, 137, 54));
            outtextxy(cx + 620, rowY + 7, L"借阅中");
        }
        else {
            settextcolor(COLOR_BTN);
            outtextxy(cx + 620, rowY + 7, L"无借阅");
        }
    }

    // 提示信息（显示在查询按钮后面）
    if (!g_message.empty()) {
        settextcolor(g_msgColor);
        settextstyle(16, 0, _T("微软雅黑"));
        outtextxy(cx + 370, HEADER_HEIGHT + 28, s2ws(g_message).c_str());
    }
}

// ==================== 借还书页面 ====================
static void drawBorrowPage() {
    drawHeader(L"借还书管理");
    int cx = NAV_WIDTH + 40;
    int cy = HEADER_HEIGHT + 40;

    // 借书区域
    settextstyle(22, 0, _T("微软雅黑"));
    settextcolor(COLOR_TEXT);
    outtextxy(cx, cy, L"借书操作");

    settextstyle(18, 0, _T("微软雅黑"));
    drawRoundBtn(cx, cy + 40, 150, 40, L"借书 (输入)", COLOR_BTN);

    settextcolor(RGB(113, 128, 150));
    outtextxy(cx + 170, cy + 52, L"点击按钮后在弹出框中输入读者ID和图书ID");

    // 分割线
    cy += 120;
    setlinecolor(COLOR_BORDER);
    line(cx, cy, WIN_WIDTH - 40, cy);

    // 还书区域
    cy += 30;
    settextstyle(22, 0, _T("微软雅黑"));
    settextcolor(COLOR_TEXT);
    outtextxy(cx, cy, L"还书操作");

    settextstyle(18, 0, _T("微软雅黑"));
    drawRoundBtn(cx, cy + 40, 150, 40, L"还书 (输入)", RGB(237, 137, 54));

    settextcolor(RGB(113, 128, 150));
    outtextxy(cx + 170, cy + 52, L"点击按钮后在弹出框中输入读者ID");

    // 借阅记录区域
    cy += 120;
    setlinecolor(COLOR_BORDER);
    line(cx, cy, WIN_WIDTH - 40, cy);

    cy += 20;
    settextstyle(22, 0, _T("微软雅黑"));
    settextcolor(COLOR_TEXT);
    outtextxy(cx, cy, L"最近借阅记录");

    cy += 35;
    setfillcolor(COLOR_WHITE);
    solidrectangle(cx, cy, WIN_WIDTH - 40, WIN_HEIGHT - 30);
    setlinecolor(COLOR_BORDER);
    rectangle(cx, cy, WIN_WIDTH - 40, WIN_HEIGHT - 30);

    // 借阅记录表头
    setfillcolor(RGB(237, 242, 247));
    solidrectangle(cx + 1, cy + 1, WIN_WIDTH - 41, cy + 30);
    settextcolor(COLOR_TEXT);
    settextstyle(16, 0, _T("微软雅黑"));
    outtextxy(cx + 10, cy + 8, L"借阅ID");
    outtextxy(cx + 80, cy + 8, L"读者ID");
    outtextxy(cx + 160, cy + 8, L"图书ID");
    outtextxy(cx + 240, cy + 8, L"借阅时间");
    outtextxy(cx + 400, cy + 8, L"应还时间");
    outtextxy(cx + 560, cy + 8, L"状态");

    const vector<BorrowRecord>& records = recordMgr.allRecords();
    int listY = cy + 35;
    int rowHeight = 25;
    int maxShow = min(8, (int)records.size());

    for (int i = 0; i < maxShow; i++) {
        int idx = (int)records.size() - 1 - i;
        if (idx < 0) break;
        const BorrowRecord& rec = records[idx];
        int rowY = listY + i * rowHeight;

        if (i % 2 == 1) {
            setfillcolor(RGB(250, 250, 250));
            solidrectangle(cx + 1, rowY, WIN_WIDTH - 41, rowY + rowHeight - 1);
        }

        settextcolor(COLOR_TEXT);
        wchar_t buf[64];
        swprintf(buf, 64, L"%d", rec.getBorrowID());
        outtextxy(cx + 10, rowY + 5, buf);
        swprintf(buf, 64, L"%d", rec.getUserID());
        outtextxy(cx + 80, rowY + 5, buf);
        swprintf(buf, 64, L"%d", rec.getBookID());
        outtextxy(cx + 160, rowY + 5, buf);
        outtextxy(cx + 240, rowY + 5, s2ws(rec.getBorrowTime().substr(0, 8)).c_str());
        outtextxy(cx + 400, rowY + 5, s2ws(rec.getShouldReturnTime().substr(0, 8)).c_str());

        if (rec.getReturnTime().empty()) {
            settextcolor(RGB(237, 137, 54));
            outtextxy(cx + 560, rowY + 5, L"未归还");
        }
        else {
            settextcolor(COLOR_BTN);
            outtextxy(cx + 560, rowY + 5, L"已归还");
        }
    }

    if (!g_message.empty()) {
        settextcolor(g_msgColor);
        settextstyle(18, 0, _T("微软雅黑"));
        outtextxy(cx + 300, HEADER_HEIGHT + 52, s2ws(g_message).c_str());
    }
}

// ==================== 文件管理页面 ====================
static void drawFilePage() {
    drawHeader(L"文件管理");
    int cx = NAV_WIDTH + 60;
    int cy = HEADER_HEIGHT + 60;

    settextstyle(20, 0, _T("微软雅黑"));
    settextcolor(COLOR_TEXT);
    outtextxy(cx, cy, L"数据文件管理");

    cy += 40;
    settextstyle(18, 0, _T("微软雅黑"));
    settextcolor(RGB(113, 128, 150));
    outtextxy(cx, cy, L"数据将保存到以下文件：");
    outtextxy(cx + 20, cy + 25, L"· readers.txt  - 读者信息（文本）");
    outtextxy(cx + 20, cy + 50, L"· books.txt    - 图书信息（文本）");
    outtextxy(cx + 20, cy + 75, L"· records.txt  - 借阅记录（文本）");

    cy += 130;
    drawRoundBtn(cx, cy, 180, 50, L"保存所有数据", COLOR_BTN);
    drawRoundBtn(cx + 220, cy, 180, 50, L"加载所有数据", COLOR_HEADER);

    cy += 80;
    setfillcolor(RGB(254, 243, 199));
    solidroundrect(cx, cy, cx + 500, cy + 80, 8, 8);
    settextcolor(RGB(146, 64, 14));
    settextstyle(18, 0, _T("微软雅黑"));
    outtextxy(cx + 20, cy + 15, L"注意事项：");
    outtextxy(cx + 20, cy + 38, L"· 首次使用请先点击「加载所有数据」");
    outtextxy(cx + 20, cy + 58, L"· 操作完成后请点击「保存所有数据」以防数据丢失");

    if (!g_message.empty()) {
        settextcolor(g_msgColor);
        settextstyle(20, 0, _T("微软雅黑"));
        outtextxy(cx, cy + 110, s2ws(g_message).c_str());
    }
}

// ==================== 输入对话框 ====================
static bool showInputDialog(const wchar_t* title, const wchar_t* labels[], wstring values[], int fieldCount) {
    // 使用 EasyX 的 InputBox 逐个输入
    for (int i = 0; i < fieldCount; i++) {
        wchar_t buf[256] = { 0 };
        wcscpy_s(buf, values[i].c_str());

        // 构建提示信息
        wstring prompt = wstring(title) + L"\n\n" + labels[i];

        // 显示输入框
        if (InputBox(buf, 256, prompt.c_str(), title, values[i].c_str(), 0, 0, false)) {
            values[i] = buf;
        }
        else {
            return false; // 用户取消
        }
    }
    return true;
}

// ==================== 主函数 ====================
int main_gui() {
    initgraph(WIN_WIDTH, WIN_HEIGHT);
    setbkcolor(COLOR_BG);
    BeginBatchDraw();

    // 尝试加载数据（文件可能不存在）
    FILE* f1 = fopen("readers.txt", "r");
    if (f1) { fclose(f1); manager.load("readers.txt"); }
    FILE* f2 = fopen("books.txt", "r");
    if (f2) { fclose(f2); bookmanager.load("books.txt"); }
    FILE* f3 = fopen("records.txt", "r");
    if (f3) { fclose(f3); recordMgr.load("records.txt"); }

    while (true) {
        cleardevice();

        // 绘制导航
        drawNavigation();

        // 绘制当前页面
        switch (currentPage) {
        case PAGE_HOME: drawHomePage(); break;
        case PAGE_BOOK: drawBookPage(); break;
        case PAGE_READER: drawReaderPage(); break;
        case PAGE_BORROW: drawBorrowPage(); break;
        case PAGE_FILE: drawFilePage(); break;
        }

        FlushBatchDraw();

        // 处理事件
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {
                int mx = msg.x, my = msg.y;

                // 导航点击
                if (mx < NAV_WIDTH) {
                    for (int i = 0; i < 5; i++) {
                        if (my >= 80 + i * 50 && my < 120 + i * 50) {
                            Page pages[] = { PAGE_HOME, PAGE_BOOK, PAGE_READER, PAGE_BORROW, PAGE_FILE };
                            currentPage = pages[i];
                            g_message = "";
                            scrollOffset = 0;
                            break;
                        }
                    }
                }

                // 页面内点击处理
                int cx = NAV_WIDTH + 20;
                int cy = HEADER_HEIGHT + 20;

                if (currentPage == PAGE_BOOK) {
                    // 添加图书
                    if (ptInRect(mx, my, cx, cy, 80, 35)) {
                        const wchar_t* labels[] = { L"图书ID:", L"书名:", L"作者:", L"出版社:", L"出版年份:", L"分类:", L"总数:" };
                        wstring vals[7] = { L"", L"", L"", L"", L"2024", L"", L"1" };
                        if (showInputDialog(L"添加图书", labels, vals, 7)) {
                            Book b;
                            b.setID(_wtoi(vals[0].c_str()));
                            char buf[256];
                            WideCharToMultiByte(CP_ACP, 0, vals[1].c_str(), -1, buf, 256, NULL, NULL);
                            b.setBookname(buf);
                            WideCharToMultiByte(CP_ACP, 0, vals[2].c_str(), -1, buf, 256, NULL, NULL);
                            b.setAuthor(buf);
                            WideCharToMultiByte(CP_ACP, 0, vals[3].c_str(), -1, buf, 256, NULL, NULL);
                            b.setPress(buf);
                            b.setPublishYear(_wtoi(vals[4].c_str()));
                            WideCharToMultiByte(CP_ACP, 0, vals[5].c_str(), -1, buf, 256, NULL, NULL);
                            b.setCategory(buf);
                            int total = _wtoi(vals[6].c_str());
                            b.setTotal(total);
                            b.setLeftnum(total);
                            bookmanager.addBook(b);
                            showMessage("图书添加成功！", COLOR_BTN);
                        }
                    }
                    // 删除图书
                    if (ptInRect(mx, my, cx + 90, cy, 80, 35) && selectedBookIdx >= 0) {
                        bookmanager.delBook(selectedBookIdx);
                        selectedBookIdx = -1;
                        showMessage("图书已删除", COLOR_BTN_DANGER);
                    }
                    // 修改图书
                    if (ptInRect(mx, my, cx + 180, cy, 80, 35) && selectedBookIdx >= 0) {
                        Book& b = bookmanager.allBooks()[selectedBookIdx];
                        const wchar_t* labels[] = { L"书名:", L"作者:", L"分类:", L"总数:" };
                        wstring vals[4] = { s2ws(b.getBookname()), s2ws(b.getAuthor()), s2ws(b.getCategory()), to_wstring(b.getTotal()) };
                        if (showInputDialog(L"修改图书", labels, vals, 4)) {
                            char buf[256];
                            WideCharToMultiByte(CP_ACP, 0, vals[0].c_str(), -1, buf, 256, NULL, NULL);
                            bookmanager.chgBookname(selectedBookIdx, buf);
                            WideCharToMultiByte(CP_ACP, 0, vals[1].c_str(), -1, buf, 256, NULL, NULL);
                            bookmanager.chgBookAuthor(selectedBookIdx, buf);
                            WideCharToMultiByte(CP_ACP, 0, vals[2].c_str(), -1, buf, 256, NULL, NULL);
                            bookmanager.chgBookCategory(selectedBookIdx, buf);
                            bookmanager.chgBookTotal(selectedBookIdx, _wtoi(vals[3].c_str()));
                            showMessage("图书修改成功！", RGB(237, 137, 54));
                        }
                    }
                    // 查询图书
                    if (ptInRect(mx, my, cx + 270, cy, 80, 35)) {
                        const wchar_t* labels[] = { L"关键词(书名/作者/分类):" };
                        wstring vals[1] = { L"" };
                        if (showInputDialog(L"查询图书", labels, vals, 1)) {
                            char buf[256];
                            WideCharToMultiByte(CP_ACP, 0, vals[0].c_str(), -1, buf, 256, NULL, NULL);
                            int idx = bookmanager.findBook(string(buf));
                            if (idx >= 0) {
                                selectedBookIdx = idx;
                                bookListScroll = max(0, idx - 5);
                                showMessage("已找到图书！", COLOR_HEADER);
                            }
                            else {
                                showMessage("未找到匹配图书", COLOR_BTN_DANGER);
                            }
                        }
                    }
                    // 列表点击选择
                    int listY = cy + 90;
                    int rowH = 30;
                    if (mx > cx && my > listY) {
                        int clickRow = (my - listY) / rowH + bookListScroll;
                        if (clickRow < (int)bookmanager.allBooks().size()) {
                            selectedBookIdx = clickRow;
                        }
                    }
                }

                if (currentPage == PAGE_READER) {
                    // 添加读者
                    if (ptInRect(mx, my, cx, cy, 80, 35)) {
                        const wchar_t* labels[] = { L"读者ID:", L"用户名:", L"真实姓名:", L"联系方式:", L"密码:" };
                        wstring vals[5] = { L"", L"", L"", L"", L"" };
                        if (showInputDialog(L"添加读者", labels, vals, 5)) {
                            Reader r;
                            r.setID(_wtoi(vals[0].c_str()));
                            char buf[256];
                            WideCharToMultiByte(CP_ACP, 0, vals[1].c_str(), -1, buf, 256, NULL, NULL);
                            r.setUsername(buf);
                            WideCharToMultiByte(CP_ACP, 0, vals[2].c_str(), -1, buf, 256, NULL, NULL);
                            r.setRealName(buf);
                            WideCharToMultiByte(CP_ACP, 0, vals[3].c_str(), -1, buf, 256, NULL, NULL);
                            r.setContact(buf);
                            WideCharToMultiByte(CP_ACP, 0, vals[4].c_str(), -1, buf, 256, NULL, NULL);
                            r.setPassord(buf);
                            manager.addReader(r);
                            showMessage("读者添加成功！", COLOR_BTN);
                        }
                    }
                    // 删除读者
                    if (ptInRect(mx, my, cx + 90, cy, 80, 35) && selectedReaderIdx >= 0) {
                        manager.delReader(selectedReaderIdx);
                        selectedReaderIdx = -1;
                        showMessage("读者已删除", COLOR_BTN_DANGER);
                    }
                    // 修改读者
                    if (ptInRect(mx, my, cx + 180, cy, 80, 35) && selectedReaderIdx >= 0) {
                        Reader& r = manager.rd[selectedReaderIdx];
                        const wchar_t* labels[] = { L"用户名:", L"真实姓名:", L"联系方式:", L"最大借书量:" };
                        wstring vals[4] = { s2ws(r.getUsername()), s2ws(r.getRealName()), s2ws(r.getContact()), to_wstring(r.getMaxBorrowNum()) };
                        if (showInputDialog(L"修改读者", labels, vals, 4)) {
                            char buf[256];
                            WideCharToMultiByte(CP_ACP, 0, vals[0].c_str(), -1, buf, 256, NULL, NULL);
                            manager.chgReaderUsername(selectedReaderIdx, buf);
                            WideCharToMultiByte(CP_ACP, 0, vals[1].c_str(), -1, buf, 256, NULL, NULL);
                            manager.chgReaderRealname(selectedReaderIdx, buf);
                            WideCharToMultiByte(CP_ACP, 0, vals[2].c_str(), -1, buf, 256, NULL, NULL);
                            manager.chgReaderContact(selectedReaderIdx, buf);
                            manager.chgReaderMaxBorrownum(selectedReaderIdx, _wtoi(vals[3].c_str()));
                            showMessage("读者修改成功！", RGB(237, 137, 54));
                        }
                    }
                    // 查询读者
                    if (ptInRect(mx, my, cx + 270, cy, 80, 35)) {
                        const wchar_t* labels[] = { L"读者ID或姓名:" };
                        wstring vals[1] = { L"" };
                        if (showInputDialog(L"查询读者", labels, vals, 1)) {
                            char buf[256];
                            WideCharToMultiByte(CP_ACP, 0, vals[0].c_str(), -1, buf, 256, NULL, NULL);
                            int idx = manager.findReader(string(buf));
                            if (idx < 0) idx = manager.findReader(_wtoi(vals[0].c_str()));
                            if (idx >= 0) {
                                selectedReaderIdx = idx;
                                readerListScroll = max(0, idx - 5);
                                showMessage("已找到读者！", COLOR_HEADER);
                            }
                            else {
                                showMessage("未找到匹配读者", COLOR_BTN_DANGER);
                            }
                        }
                    }
                    // 列表点击
                    int listY = cy + 90;
                    int rowH = 30;
                    if (mx > cx && my > listY) {
                        int clickRow = (my - listY) / rowH + readerListScroll;
                        vector<Reader>& readers = getAllReaders();
                        if (clickRow < (int)readers.size()) {
                            selectedReaderIdx = clickRow;
                        }
                    }
                }

                if (currentPage == PAGE_BORROW) {
                    int bcx = NAV_WIDTH + 40;
                    int bcy = HEADER_HEIGHT + 80;
                    // 借书
                    if (ptInRect(mx, my, bcx, bcy, 150, 40)) {
                        const wchar_t* labels[] = { L"读者ID:", L"图书ID:" };
                        wstring vals[2] = { L"", L"" };
                        if (showInputDialog(L"借书操作", labels, vals, 2)) {
                            int readerID = _wtoi(vals[0].c_str());
                            int bookID = _wtoi(vals[1].c_str());
                            int rIdx = manager.findReader(readerID);
                            int bIdx = bookmanager.findBook(bookID);
                            if (rIdx == -1) {
                                showMessage("读者不存在！", COLOR_BTN_DANGER);
                            }
                            else if (bIdx == -1) {
                                showMessage("图书不存在！", COLOR_BTN_DANGER);
                            }
                            else {
                                Book b = bookmanager.getBook(bIdx);
                                if (b.getLeftnum() <= 0) {
                                    showMessage("该书库存不足！", COLOR_BTN_DANGER);
                                }
                                else {
                                    ManageTime tu;
                                    time_t now = time(nullptr);
                                    char buf[32];
                                    tu.TimeToString(now, buf);
                                    string borrowTime = buf;
                                    // 2. 弹出输入框询问借阅天数
                                    wchar_t inputDays[10] = L"7"; // 1. 使用 wchar_t 定义宽字符数组，L前缀表示宽字符串
                                    InputBox(inputDays, 10, L"请输入借阅天数："); // 2. 提示文字也要加 L 前缀
                                    int days = _wtoi(inputDays); // 3. 使用 _wtoi (宽字符转整数) 替代 atoi
                                    if (days <= 0) days = 3;    // 容错处理,最少借书3天
                                    time_t nextDate = now + (long long)days * 24 * 3600;
                                    tu.TimeToString(nextDate, buf);
                                    string shouldReturn = buf;

                                    // 4. 创建借阅记录
                                    int borrowID = (int)recordMgr.allRecords().size() + 1;
                                    BorrowRecord rec(readerID, b.getID(), borrowTime, shouldReturn, "", 0, false, false, 0.0, borrowID);
                                    recordMgr.addBorrowRecord(rec);
                                    // 减少库存并标记为借出
                                    bookmanager.chgBookLeftnum(bIdx, b.getLeftnum() - 1);
                                    bookmanager.chgBookIsborrowed(bIdx, true);
                                    manager.setReaderBook(rIdx, b);
                                    // 自动保存数据
                                    manager.save("readers.txt");
                                    bookmanager.save("books.txt");
                                    recordMgr.save("records.txt");
                                    showMessage("借书成功！", COLOR_BTN);
                                }
                            }
                        }
                    }
                    // 还书
                    bcy += 150;
                    if (ptInRect(mx, my, bcx, bcy, 150, 40)) {
                        const wchar_t* labels[] = { L"读者ID:" };
                        wstring vals[1] = { L"" };
                        if (showInputDialog(L"还书操作", labels, vals, 1)) {
                            int readerID = _wtoi(vals[0].c_str());
                            int rIdx = manager.findReader(readerID);
                            if (rIdx == -1) {
                                showMessage("读者不存在！", COLOR_BTN_DANGER);
                            }
                            else {
                                Reader r = manager.getreader(rIdx);
                                // 直接从借阅记录中查找该读者未归还的借阅记录
                                vector<int> recs = recordMgr.findRecordsByUser(r.getID());
                                int targetID = -1;
                                int targetBookID = 0;
                                for (int idx : recs) {
                                    if (idx >= 0 && idx < (int)recordMgr.allRecords().size()) {
                                        const BorrowRecord& br = recordMgr.allRecords()[idx];
                                        if (br.getReturnTime().empty()) {
                                            targetID = br.getBorrowID();
                                            targetBookID = br.getBookID();
                                            break;
                                        }
                                    }
                                }
                                if (targetID == -1) {
                                    showMessage("该读者无未归还的借阅记录！", COLOR_BTN_DANGER);
                                }
                                else {
                                    double fine = recordMgr.returnBook(targetID, bookmanager.allBooks());

                                    manager.clearReaderBook(rIdx);
                                    int bIdx = bookmanager.findBook(targetBookID);
                                    if (bIdx != -1) bookmanager.chgBookIsborrowed(bIdx, false);
                                    // 自动保存数据
                                    manager.save("readers.txt");
                                    bookmanager.save("books.txt");
                                    recordMgr.save("records.txt");
                                    char msg[64];
                                    sprintf(msg, "还书成功！罚款: %.0f 元", fine);
                                    showMessage(msg, COLOR_BTN);
                                }
                            }
                        }
                    }
                }

                if (currentPage == PAGE_FILE) {
                    int fcx = NAV_WIDTH + 60;
                    int fcy = HEADER_HEIGHT + 230;
                    // 保存
                    if (ptInRect(mx, my, fcx, fcy, 180, 50)) {
                        manager.save("readers.txt");
                        bookmanager.save("books.txt");
                        recordMgr.save("records.txt");
                        showMessage("所有数据已保存！", COLOR_BTN);
                    }
                    // 加载
                    if (ptInRect(mx, my, fcx + 220, fcy, 180, 50)) {
                        manager.load("readers.txt");
                        bookmanager.load("books.txt");
                        recordMgr.load("records.txt");
                        showMessage("所有数据已加载！", COLOR_HEADER);
                    }
                }
            }

            // 滚轮滚动
            if (msg.uMsg == WM_MOUSEWHEEL) {
                int delta = (short)HIWORD(msg.mkShift);
                if (currentPage == PAGE_BOOK) {
                    bookListScroll -= delta / 120;
                    if (bookListScroll < 0) bookListScroll = 0;
                    int maxScroll = max(0, (int)bookmanager.allBooks().size() - 15);
                    if (bookListScroll > maxScroll) bookListScroll = maxScroll;
                }
                if (currentPage == PAGE_READER) {
                    readerListScroll -= delta / 120;
                    if (readerListScroll < 0) readerListScroll = 0;
                    int maxScroll = max(0, (int)getAllReaders().size() - 15);
                    if (readerListScroll > maxScroll) readerListScroll = maxScroll;
                }
            }
        }

        // ESC 退出
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 27) break;
        }

        Sleep(16);
    }

    EndBatchDraw();
    closegraph();
    return 0;
}