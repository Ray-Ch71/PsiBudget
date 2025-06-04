#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_CATS 100

typedef struct {
    char name[100];
    double amount;
    COLORREF color;
} Category;

HWND hMainWindow, hIncomeWindow, hSpendingWindow, hTransactionsWindow, hPieChartWindow;
char currentType[20] = {0};
HWND hBalanceLabel;
HFONT hFont;
HBRUSH hBgBrush;
COLORREF bgColor = RGB(245, 245, 255);
COLORREF textColor = RGB(30, 30, 60);

Category cats[MAX_CATS];
int catCount = 0;
double totalSpending = 0.0;

// HSV to RGB conversion (h in degrees, s and v in [0,1])
COLORREF generateColor(int i) {
    double h = fmod(i * 137.508, 360.0); // use golden angle to maximize separation
    double s = 0.8, v = 0.95;

    double c = v * s;
    double x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    double m = v - c;

    double r = 0, g = 0, b = 0;

    if (h < 60)      { r = c; g = x; b = 0; }
    else if (h < 120){ r = x; g = c; b = 0; }
    else if (h < 180){ r = 0; g = c; b = x; }
    else if (h < 240){ r = 0; g = x; b = c; }
    else if (h < 300){ r = x; g = 0; b = c; }
    else             { r = c; g = 0; b = x; }

    return RGB((int)((r + m) * 255), (int)((g + m) * 255), (int)((b + m) * 255));
}


void ApplyFont(HWND hwnd) {
    HWND hChild = GetWindow(hwnd, GW_CHILD);
    while (hChild) {
        SendMessage(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);
        hChild = GetWindow(hChild, GW_HWNDNEXT);
    }
}

void updateBalance() {
    FILE* file = fopen("transactions.txt", "r");
    double balance = 0.0;
    char line[256];

    if (file) {
        while (fgets(line, sizeof(line), file)) {
            char type[20], name[100], amountStr[100];
            double amount;

            if (sscanf(line, " %[^|]| %[^|]| %[^|]|", type, name, amountStr) == 3) {
                amount = atof(amountStr);
                if (strncmp(type, "Income", 6) == 0)
                    balance += amount;
                else if (strncmp(type, "Spending", 8) == 0)
                    balance -= amount;
            }
        }
        fclose(file);
    }

    char text[100];
    sprintf(text, "Current Balance: €%.2f", balance);
    SetWindowText(hBalanceLabel, text);
}

void saveTransaction() {
    char name[100] = {0}, amount[100] = {0};
    HWND hActive = (currentType[0] == 'I') ? hIncomeWindow : hSpendingWindow;

    GetWindowText(GetDlgItem(hActive, 101), name, 100);
    GetWindowText(GetDlgItem(hActive, 102), amount, 100);

    if (name[0] == '\0' || amount[0] == '\0') {
        MessageBox(NULL, "Please fill all fields!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    double val = atof(amount);
    if (val <= 0) {
        MessageBox(NULL, "Amount must be a positive number!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    FILE* file = fopen("transactions.txt", "a");
    if (file) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        fprintf(file, "%s | %s | %.2f | %04d-%02d-%02d\n",
            currentType, name, val, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
        fclose(file);
    } else {
        MessageBox(NULL, "Unable to save transaction!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    SetWindowText(GetDlgItem(hActive, 101), "");
    SetWindowText(GetDlgItem(hActive, 102), "");
    MessageBox(NULL, "Transaction saved!", "Success", MB_OK);
    updateBalance();
}

LRESULT CALLBACK StyledProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CTLCOLORSTATIC || msg == WM_CTLCOLORBTN || msg == WM_CTLCOLOREDIT) {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, bgColor);
        SetTextColor(hdc, textColor);
        return (INT_PTR)hBgBrush;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK TransactionWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateWindow("STATIC", "Name:", WS_VISIBLE | WS_CHILD, 20, 20, 100, 20, hwnd, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 20, 150, 20, hwnd, (HMENU)101, NULL, NULL);
        CreateWindow("STATIC", "Amount:", WS_VISIBLE | WS_CHILD, 20, 50, 100, 20, hwnd, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 50, 150, 20, hwnd, (HMENU)102, NULL, NULL);
        CreateWindow("BUTTON", "Save", WS_VISIBLE | WS_CHILD, 100, 100, 120, 30, hwnd, (HMENU)103, NULL, NULL);
        ApplyFont(hwnd);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 103) saveTransaction();
        break;

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        ShowWindow(hMainWindow, SW_SHOW);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK TransactionsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HWND hEdit = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
            10, 10, 480, 360, hwnd, (HMENU)201, NULL, NULL);
        HFONT hMono = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, "Courier New");
        SendMessage(hEdit, WM_SETFONT, (WPARAM)hMono, TRUE);
        break;
    }

    case WM_SHOWWINDOW:
        if (wParam) {
            FILE* file = fopen("transactions.txt", "r");
            HWND hEdit = GetDlgItem(hwnd, 201);
            if (file) {
                char line[256], output[5000] = "", formatted[300];
                strcat(output, "Type      | Name                | Amount   | Date\r\n");
                strcat(output, "----------|---------------------|----------|----------\r\n");

                while (fgets(line, sizeof(line), file)) {
                    char type[20], name[100], amount[100], date[20];
                    if (sscanf(line, " %[^|]| %[^|]| %[^|]| %s", type, name, amount, date) == 4) {
                        sprintf(formatted, "%-10s| %-20s| %8s | %-14s\r\n", type, name, amount, date);
                        strcat(output, formatted);
                    }
                }
                fclose(file);
                SetWindowText(hEdit, output);
            }
            else {
                SetWindowText(hEdit, "No transactions yet.");
            }
        }
        break;

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        ShowWindow(hMainWindow, SW_SHOW);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void loadCategories() {
    catCount = 0;
    totalSpending = 0;

    FILE* file = fopen("transactions.txt", "r");
    if (!file) return;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char type[20], name[100], date[20];
        double amount;

        if (sscanf(line, " %[^|]| %[^|]| %lf | %s", type, name, &amount, date) == 4) {
            if (strncmp(type, "Spending", 8) == 0) {
                totalSpending += amount;

                // Check if category already exists
                int found = -1;
                for (int i = 0; i < catCount; i++) {
                    if (strcmp(cats[i].name, name) == 0) {
                        found = i;
                        break;
                    }
                }
                if (found == -1 && catCount < MAX_CATS) {
                    strcpy(cats[catCount].name, name);
                    cats[catCount].amount = amount;
                    cats[catCount].color = generateColor(catCount);
                    catCount++;
                }
                else if (found != -1) {
                    cats[found].amount += amount;
                }
            }
        }
    }
    fclose(file);
}

LRESULT CALLBACK PieChartWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);
        HBRUSH hWhite = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdc, &rect, hWhite);
        DeleteObject(hWhite);

        loadCategories();

        if (catCount == 0) {
            DrawText(hdc, "No spending data to display", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            EndPaint(hwnd, &ps);
            break;
        }

        int cx = rect.right / 2;
        int cy = rect.bottom / 2;
        int radius = (cx < cy ? cx : cy) - 50;

        double startAngle = 0;

        for (int i = 0; i < catCount; i++) {
            double sliceAngle = cats[i].amount / totalSpending * 360;

            // Calculate pie slice points
            POINT pts[3];
            pts[0].x = cx;
            pts[0].y = cy;
            double startRad = startAngle * 3.1415926535 / 180.0;
            double endRad = (startAngle + sliceAngle) * 3.1415926535 / 180.0;

            pts[1].x = cx + (int)(radius * cos(startRad));
            pts[1].y = cy - (int)(radius * sin(startRad));
            pts[2].x = cx + (int)(radius * cos(endRad));
            pts[2].y = cy - (int)(radius * sin(endRad));

            // Create a brush with the category color
            HBRUSH hBrush = CreateSolidBrush(cats[i].color);
            HGDIOBJ oldBrush = SelectObject(hdc, hBrush);

            // Create a pen for the outline
            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            HGDIOBJ oldPen = SelectObject(hdc, hPen);

            Pie(hdc,
                cx - radius, cy - radius, cx + radius, cy + radius,
                cx + (int)(radius * cos(startRad)), cy - (int)(radius * sin(startRad)),
                cx + (int)(radius * cos(endRad)), cy - (int)(radius * sin(endRad)));


            SelectObject(hdc, oldBrush);
            SelectObject(hdc, oldPen);

            DeleteObject(hBrush);
            DeleteObject(hPen);

            startAngle += sliceAngle;
        }

        // Draw legend
        int legendX = 20;
        int legendY = rect.bottom - catCount * 25 - 20;
        for (int i = 0; i < catCount; i++) {
            RECT box = { legendX, legendY + i * 25, legendX + 20, legendY + 20 + i * 25 };
            HBRUSH hBrush = CreateSolidBrush(cats[i].color);
            FillRect(hdc, &box, hBrush);
            DeleteObject(hBrush);

            SetTextColor(hdc, textColor);
            SetBkMode(hdc, TRANSPARENT);

            char buf[200];
            snprintf(buf, sizeof(buf), "%s: €%.2f", cats[i].name, cats[i].amount);
            TextOut(hdc, legendX + 30, legendY + 5 + i * 25, buf, (int)strlen(buf));
        }

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        ShowWindow(hMainWindow, SW_SHOW);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        hFont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            FF_SWISS, "Segoe UI");

        hBgBrush = CreateSolidBrush(bgColor);

        CreateWindow("BUTTON", "Add Income", WS_VISIBLE | WS_CHILD, 20, 20, 150, 40, hwnd, (HMENU)201, NULL, NULL);
        CreateWindow("BUTTON", "Add Spending", WS_VISIBLE | WS_CHILD, 200, 20, 150, 40, hwnd, (HMENU)202, NULL, NULL);
        CreateWindow("BUTTON", "Show Transactions", WS_VISIBLE | WS_CHILD, 20, 80, 330, 40, hwnd, (HMENU)203, NULL, NULL);
        CreateWindow("BUTTON", "Show Spending Pie Chart", WS_VISIBLE | WS_CHILD, 20, 140, 330, 40, hwnd, (HMENU)204, NULL, NULL);

        hBalanceLabel = CreateWindow("STATIC", "Current Balance: €0.00", WS_VISIBLE | WS_CHILD, 20, 220, 330, 30, hwnd, NULL, NULL, NULL);

        ApplyFont(hwnd);

        updateBalance();
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 201:
            strcpy(currentType, "Income");
            ShowWindow(hIncomeWindow, SW_SHOW);
            ShowWindow(hwnd, SW_HIDE);
            break;
        case 202:
            strcpy(currentType, "Spending");
            ShowWindow(hSpendingWindow, SW_SHOW);
            ShowWindow(hwnd, SW_HIDE);
            break;
        case 203:
            ShowWindow(hTransactionsWindow, SW_SHOW);
            ShowWindow(hwnd, SW_HIDE);
            break;
        case 204:
            ShowWindow(hPieChartWindow, SW_SHOW);
            ShowWindow(hwnd, SW_HIDE);
            InvalidateRect(hPieChartWindow, NULL, TRUE);
            break;
        }
        break;

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLOREDIT:
    {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, bgColor);
        SetTextColor(hdc, textColor);
        return (INT_PTR)hBgBrush;
    }

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        DeleteObject(hFont);
        DeleteObject(hBgBrush);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wcMain = { 0 }, wcTransaction = { 0 }, wcTransactions = { 0 }, wcPieChart = { 0 };

    wcMain.lpfnWndProc = MainWndProc;
    wcMain.hInstance = hInstance;
    wcMain.lpszClassName = "MainWindowClass";
    wcMain.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcMain.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    wcTransaction.lpfnWndProc = TransactionWndProc;
    wcTransaction.hInstance = hInstance;
    wcTransaction.lpszClassName = "TransactionWindowClass";
    wcTransaction.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    wcTransactions.lpfnWndProc = TransactionsWndProc;
    wcTransactions.hInstance = hInstance;
    wcTransactions.lpszClassName = "TransactionsWindowClass";
    wcTransactions.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    wcPieChart.lpfnWndProc = PieChartWndProc;
    wcPieChart.hInstance = hInstance;
    wcPieChart.lpszClassName = "PieChartWindowClass";
    wcPieChart.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wcMain);
    RegisterClass(&wcTransaction);
    RegisterClass(&wcTransactions);
    RegisterClass(&wcPieChart);

    hMainWindow = CreateWindow("MainWindowClass", "Psi-Budget Manager",
        WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 320,
        NULL, NULL, hInstance, NULL);

    hIncomeWindow = CreateWindow("TransactionWindowClass", "Add Income",
        WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
        CW_USEDEFAULT, CW_USEDEFAULT, 320, 180,
        NULL, NULL, hInstance, NULL);

    hSpendingWindow = CreateWindow("TransactionWindowClass", "Add Spending",
        WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
        CW_USEDEFAULT, CW_USEDEFAULT, 320, 180,
        NULL, NULL, hInstance, NULL);

    hTransactionsWindow = CreateWindow("TransactionsWindowClass", "Transactions",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        NULL, NULL, hInstance, NULL);

    hPieChartWindow = CreateWindow("PieChartWindowClass", "Spending Pie Chart",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hMainWindow, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
