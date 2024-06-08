#include <Windows.h>
#include <CommCtrl.h>

#include <aviutl.hpp>

#include "../common/version.hpp"
#include "../common/panlaw.hpp"

#define DLLEXPORT extern "C" __declspec(dllexport)

using AviUtl::FilterPlugin;
using AviUtl::EditHandle;
using AviUtl::detail::FilterPluginFlag;
using AviUtl::detail::FilterPluginWindowMessage;

// 定数

const char* FILTER_NAME = "パン";
const char* FILTER_INFO = "パン v" VERSION " by karoterra";

constexpr int IDC_COMBO_PANLAW = 1001;

const PanLaw PAN_LAWS[] = {
    PanLaw::Zero_dB,
    PanLaw::Minus3dB,
    PanLaw::Minus4_5dB,
    PanLaw::Minus6dB,
};
constexpr int DEFAULT_PAN_LAW_ID = 1;

// グローバル変数

HFONT g_font = NULL;
HWND g_comboPanLaw = NULL;
PanLaw g_panLaw = PAN_LAWS[DEFAULT_PAN_LAW_ID];

/**
 * @brief プロジェクト設定のPanLawを取得する
 *
 * @return PanLaw
 */
DLLEXPORT int32_t getProjectPanLaw() {
    return static_cast<int32_t>(g_panLaw);
}

/**
 * @brief 指定したウィンドウにフォントを設定する
 *
 * @param hwnd ウィンドウハンドル
 */
void setFont(HWND hwnd) {
    SendMessage(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), MAKELPARAM(TRUE, 0));
}

/**
 * @brief フィルタプラグインのウィンドウを作成する
 *
 * @param fp フィルタ構造体へのポインタ
 * @return 成功ならtrue
 */
bool createFilterWindow(FilterPlugin* fp) {
    HWND label = CreateWindow(
        WC_STATIC, "Pan Law",
        WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
        8, 8, 52, 12,
        fp->hwnd, nullptr, fp->dll_hinst, nullptr
    );
    setFont(label);

    g_comboPanLaw = CreateWindow(
        WC_COMBOBOX, nullptr,
        CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
        60, 4, 72, 139,
        fp->hwnd, reinterpret_cast<HMENU>(IDC_COMBO_PANLAW), fp->dll_hinst, nullptr
    );
    setFont(g_comboPanLaw);

    for (size_t i = 0; i < _countof(PAN_LAWS); i++) {
        auto str = panLawToString(PAN_LAWS[i]);
        SendMessage(g_comboPanLaw, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(str));
    }

    SendMessage(g_comboPanLaw, CB_SETCURSEL, DEFAULT_PAN_LAW_ID, 0);

    return true;
}

/**
 * @brief プラグインの初期化処理
 *
 * @param fp フィルタ構造体へのポインタ
 * @return BOOL TRUEで初期化成功
 */
BOOL func_init(FilterPlugin* fp) {
    NONCLIENTMETRICS ncm{ .cbSize = sizeof(NONCLIENTMETRICS) };
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
    g_font = CreateFontIndirect(&ncm.lfCaptionFont);

    return TRUE;
}

/**
 * @brief プラグインのウィンドウハンドル
 *
 * @param hwnd 設定画面のウィンドウハンドル
 * @param message ウィンドウメッセージ
 * @param wparam 追加メッセージ情報
 * @param lparam 追加メッセージ情報
 * @param editp エディットハンドル
 * @param fp フィルタ構造体へのポインタ
 * @return BOOL TRUEなら再描画
 */
BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, EditHandle* editp, FilterPlugin* fp) {
    switch (message) {
    case FilterPluginWindowMessage::Init:
        // ウィンドウハンドルなどの準備が完了した後の初期化処理
        createFilterWindow(fp);
        break;

    case WM_COMMAND:
        switch (LOWORD(wparam)) {
        case IDC_COMBO_PANLAW:
            // Pan Law コンボボックスの選択が変更されたとき
            if (HIWORD(wparam) == CBN_SELCHANGE) {
                size_t panLawId = SendMessage(g_comboPanLaw, CB_GETCURSEL, 0, 0);
                g_panLaw = PAN_LAWS[panLawId];
                return TRUE;
            }
            break;
        }
        break;
    }

    return FALSE;
}

/**
 * @brief プロジェクトファイル読み込み時の処理
 *
 * @param fp フィルタ構造体へのポインタ
 * @param editp エディットハンドル
 * @param data プロジェクトから読み込むデータ
 * @param size プロジェクトから読み込むデータのバイト数
 * @return BOOL 成功ならTRUE
 */
BOOL func_project_load(FilterPlugin* fp, EditHandle* editp, void* data, int32_t size) {
    if (size < sizeof(PanLaw)) {
        return FALSE;
    }

    auto panLaw = *reinterpret_cast<PanLaw*>(data);
    for (size_t i = 0; i < _countof(PAN_LAWS); i++) {
        if (panLaw == PAN_LAWS[i]) {
            g_panLaw = panLaw;
            SendMessage(g_comboPanLaw, CB_SETCURSEL, static_cast<WPARAM>(i), 0);
            break;
        }
    }
    return TRUE;
}

/**
 * @brief プロジェクトファイル保存時の処理
 *
 * @param fp フィルタ構造体へのポインタ
 * @param editp エディットハンドル
 * @param data プロジェクトに書き込むデータ
 * @param size プロジェクトに書き込むデータのバイト数
 * @return BOOL 成功ならTRUE
 */
BOOL func_project_save(FilterPlugin* fp, EditHandle* editp, void* data, int32_t* size) {
    *size = sizeof(PanLaw);
    if (!data) {
        return TRUE;
    }

    *reinterpret_cast<PanLaw*>(data) = g_panLaw;
    return TRUE;
}

AviUtl::FilterPluginDLL filterDll{
    .flag = FilterPluginFlag::AlwaysActive
        | FilterPluginFlag::PriorityLowest
        | FilterPluginFlag::ExInformation
        | FilterPluginFlag::DispFilter
        | FilterPluginFlag::WindowSize
        | FilterPluginFlag::WindowThickFrame,
    .x = 200,
    .y = 100,
    .name = FILTER_NAME,
    .func_init = func_init,
    .func_WndProc = func_WndProc,
    .information = FILTER_INFO,
    .func_project_load = func_project_load,
    .func_project_save = func_project_save,
};

DLLEXPORT AviUtl::FilterPluginDLL* GetFilterTable(void) {
    return &filterDll;
}
