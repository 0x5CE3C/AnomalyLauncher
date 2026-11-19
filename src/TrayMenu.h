#pragma once
#include <windows.h>
#include <shellapi.h>

#define WM_TRAYICON (WM_APP + 1)

enum TrayEvent : UINT {
    TRAY_EVENT_NONE = 0,
    TRAY_EVENT_LCLICK = 1 << 0,
    TRAY_EVENT_LDBLCLK = 1 << 1,
    TRAY_EVENT_RCLICK = 1 << 2,
};

class TrayMenu {
public:
    bool Init(HWND hwnd, UINT iconId = 1, UINT callbackMsg = WM_TRAYICON);
    UINT HandleTrayMessage(WPARAM wParam, LPARAM lParam);
    void RenderPopup();   // опрашивает события + рисует ImGui popup
    void SetTooltip(const wchar_t* text);
    void SetIcon(HICON hIcon);
    void Remove();

    bool ShouldQuit()       const { return m_shouldQuit; }
    bool IsWindowVisible()  const { return m_windowVisible; }
    void RequestQuit() { m_shouldQuit = true; }
    void SetWindowVisible(bool v) { m_windowVisible = v; }
    void ToggleWindow() { m_windowVisible = !m_windowVisible; }

    // Запомнить HWND viewport главного окна (вызывается из gui::Render)
    void SetMainViewportHwnd(HWND hwnd) { m_mainViewportHwnd = hwnd; }

private:
    NOTIFYICONDATAW m_nid = {};
    HWND            m_hwnd = nullptr;
    UINT            m_pendingEvents = TRAY_EVENT_NONE;
    bool            m_shouldQuit = false;
    bool            m_windowVisible = true;
    bool            m_popupWasOpen = false;
    HWND            m_popupHwnd = nullptr; // HWND окна popup (запоминаем)
    HWND            m_mainViewportHwnd = nullptr;  // +++ НОВОЕ +++
    int  m_popupFrames = 0;  // +++ ДОБАВИТЬ: счётчик кадров popup +++
};

extern TrayMenu g_tray;