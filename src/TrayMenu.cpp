#include "main.h"
#include "TrayMenu.h"
#include <imgui.h>

TrayMenu g_tray;

// ─── Init ─────────────────────────────────────────────

bool TrayMenu::Init(HWND hwnd, UINT iconId, UINT callbackMsg) {
    m_hwnd = hwnd;

    ZeroMemory(&m_nid, sizeof(m_nid));
    m_nid.cbSize = sizeof(m_nid);          // Важно: для Windows Vista+ это должно быть NOTIFYICONDATA_V4_SIZE, если используешь флаги V4
    m_nid.hWnd = m_hwnd;
    m_nid.uID = iconId;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = callbackMsg;
    HMODULE hModule = GetModuleHandle(nullptr);
    HICON hIcon = LoadIcon(hModule, MAKEINTRESOURCE(IDI_ICON1));
    if (!hIcon) {
        hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    }
    m_nid.hIcon = hIcon;

    wcscpy_s(m_nid.szTip, ARRAYSIZE(m_nid.szTip), L"AnomalyLauncher Extended");

    // ГЛАВНОЕ: только Shell_NotifyIcon (без A/W)
    if (!Shell_NotifyIconW(NIM_ADD, &m_nid)) {
        return false;
    }

    return true;
}

// ─── HandleTrayMessage ────────────────────────────────
// Только накапливает флаги. ImGui трогать нельзя —
// рендер идёт в gui::Render().

UINT TrayMenu::HandleTrayMessage(WPARAM wParam, LPARAM lParam) {
    switch (static_cast<UINT>(lParam)) {
    case WM_LBUTTONUP:      m_pendingEvents |= TRAY_EVENT_LCLICK;   break;
    case WM_LBUTTONDBLCLK:  m_pendingEvents |= TRAY_EVENT_LDBLCLK;  break;
    case WM_RBUTTONUP:      m_pendingEvents |= TRAY_EVENT_RCLICK;   break;
    }
    return 0;
}

// ─── RenderPopup ──────────────────────────────────────
// Вызывается внутри ImGui кадра (в gui::Render).
// Опрашивает события, открывает popup, рисует пункты меню.


void TrayMenu::RenderPopup() {
    UINT events = m_pendingEvents;
    m_pendingEvents = TRAY_EVENT_NONE;

    if (events & TRAY_EVENT_LCLICK || events & TRAY_EVENT_LDBLCLK) {
        ToggleWindow();
    }

    if (events & TRAY_EVENT_RCLICK) {
        SetForegroundWindow(m_hwnd);

        POINT pt;
        GetCursorPos(&pt);
        ImGui::SetNextWindowPos(ImVec2((float)pt.x, (float)pt.y));
        ImGui::SetNextWindowFocus();
        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::SetNextWindowSize(ImVec2(gui::WINDOW_WIDTH / 4, -footer_height_to_reserve));
        ImGui::OpenPopup("TrayPopup");
    }

    bool popupOpen = ImGui::BeginPopup("TrayPopup");

    if (popupOpen) {
        ImGuiViewport* vp = ImGui::GetWindowViewport();
        if (vp && vp->PlatformHandle) {
            m_popupHwnd = (HWND)vp->PlatformHandle;
            SetWindowPos(m_popupHwnd, HWND_TOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }

        // +++ Закрываем popup, если фокус ушёл в другое окно +++
        // Пропускаем первый кадр — даём popup развернуться
        m_popupFrames++;
        if (m_popupFrames > 1) {
            HWND fg = GetForegroundWindow();
            // Фокус не на popup, не на скрытом окне, не на главном viewport
            if (fg != m_popupHwnd && fg != m_hwnd && fg != m_mainViewportHwnd) {
                ImGui::CloseCurrentPopup();
            }
        }

        // --- Содержимое меню ---

        if (pMenu->AddStaticButton(m_windowVisible ? "Скрыть окно" : "Показать окно"))
        {
            ToggleWindow();
        }

        ImGui::Separator();

        if (pMenu->AddStaticButton("Выход"))
        {
            RequestQuit();
        }

        ImGui::EndPopup();
    }
    else {
        m_popupFrames = 0;  // +++ Сбрасываем счётчик +++
    }

    // Popup закрылся — снимаем TOPMOST и возвращаем фокус
    if (m_popupWasOpen && !popupOpen) {
        if (m_popupHwnd) {
            SetWindowPos(m_popupHwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            m_popupHwnd = nullptr;
        }

        HWND target = m_mainViewportHwnd ? m_mainViewportHwnd : m_hwnd;
        if (m_windowVisible && target) {
            SetForegroundWindow(target);
            SetFocus(target);
        }
    }

    m_popupWasOpen = popupOpen;
}

// ─── Вспомогательные ──────────────────────────────────

void TrayMenu::SetTooltip(const wchar_t* text) {
    wcscpy_s(m_nid.szTip, text);
    m_nid.uFlags = NIF_TIP;
    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
}

void TrayMenu::SetIcon(HICON hIcon) {
    m_nid.hIcon = hIcon;
    m_nid.uFlags = NIF_ICON;
    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
}

void TrayMenu::Remove() {
    Shell_NotifyIconW(NIM_DELETE, &m_nid);
}