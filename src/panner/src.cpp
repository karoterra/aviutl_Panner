#include <Windows.h>

#include <array>
#include <string>
#include <iostream>
#include <format>
#include <algorithm>
#include <cmath>
#include <numbers>

#include <exedit.hpp>

#include "../common/version.hpp"
#include "../common/panlaw.hpp"

#define DLLEXPORT extern "C" __declspec(dllexport)

// 定数

const char* FILTER_NAME = "パン";
const char* FILTER_INFO = "パン v" VERSION " by karoterra";

// トラックバー
constexpr int TRACK_NUM = 1;
const char* TRACK_NAME[TRACK_NUM] = { "パン" };

constexpr int TRACK_ID_PAN = 0;

int TRACK_DEFAULT[TRACK_NUM] = { 0 };
int TRACK_START[TRACK_NUM] = { -100 };
int TRACK_END[TRACK_NUM] = { 100 };
int TRACK_SCALE[TRACK_NUM] = { 1 };
int TRACK_DRAG_MIN[TRACK_NUM] = { -100 };
int TRACK_DRAG_MAX[TRACK_NUM] = { 100 };

// Pan Law
constexpr int PAN_LAW_NUM = 4;
constexpr std::array<PanLaw, PAN_LAW_NUM> PAN_LAWS = {
    PanLaw::Default,
    PanLaw::Minus3dB,
    PanLaw::Minus4_5dB,
    PanLaw::Minus6dB,
};

// Pan Lawドロップダウンリスト用の文字列長の計算
constexpr size_t calcTotalSize() {
    size_t size = 0;
    for (const auto& law : PAN_LAWS) {
        size += std::char_traits<char>::length(panLawToString(law)) + 1;
    }
    return size + 1;
}

// Pan Lawドロップダウンリスト用の文字列を生成
constexpr auto generateDropDownString() {
    std::array<char, calcTotalSize()> result{};
    size_t pos = 0;
    for (const auto& law : PAN_LAWS) {
        const char* str = panLawToString(law);
        while (*str) {
            result[pos++] = *str++;
        }
        result[pos++] = '\0';
    }
    result[pos++] = '\0';
    return result;
}

// Pan Lawドロップダウンリスト用の文字列
constexpr auto dropDownString = generateDropDownString();

// チェックボックス
// 初期値を-2にするとドロップダウンリストになる
constexpr int CHECK_NUM = 1;
const char* CHECK_NAME[CHECK_NUM] = { dropDownString.data() };
int CHECK_DEFAULT[CHECK_NUM] = { -2 };

// Exdata
struct Exdata {
    PanLaw panlaw;
};

constexpr int EXDATA_SIZE = sizeof(Exdata);
Exdata EXDATA_DEF{
    .panlaw = PanLaw::Default,
};
ExEdit::ExdataUse EXDATA_USE[] = {
    {
        .type = ExEdit::ExdataUse::Type::Number,
        .size = sizeof(PanLaw),
        .name = "panlaw",
    },
};

// 参考: https://github.com/nazonoSAUNA/Echo.eef/blob/main/src.cpp
static void(__cdecl* update_any_exdata)(ExEdit::ObjectFilterIndex, const char*) = nullptr;

// PannerConfig.auf
// プロジェクト設定のPanLawを取得する
int32_t(*getProjectPanLaw)() = nullptr;

/**
 * @brief デバッグ用のprintln
 *
 * @tparam Args
 * @param fmt
 * @param args
 */
template <typename... Args>
void debugPrintln(const std::string& fmt, Args&&... args) {
#ifdef _DEBUG
    std::string str = std::vformat(fmt, std::make_format_args(args...));
    std::cout << "[Panner.eef] " << str << std::endl;
#endif
}

/**
 * @brief panlawのインデックスを取得
 *
 * @param panlaw
 * @return int
 */
int getPanLawIndex(PanLaw panlaw) {
    for (int i = 0; i < PAN_LAW_NUM; i++) {
        if (panlaw == PAN_LAWS[i]) {
            return i;
        }
    }
    return 0;
}

/**
 * @brief フラグが立っているか確認する
 *
 * @tparam T
 * @param x
 * @param flag 調べるフラグ
 * @return フラグが立っていればtrue
 */
template<typename T>
bool isFlagSet(T x, T flag) {
    return (static_cast<uint32_t>(x) & static_cast<uint32_t>(flag)) != 0;
}

/**
 * @brief 拡張編集のインスタンスハンドルを取得する
 *
 * https://github.com/nazonoSAUNA/Echo.eef/blob/main/src.cpp
 *
 * @param efp フィルタ構造体へのポインタ
 * @return int
 */
int getExEditDllHinst(const ExEdit::Filter* efp) {
    return reinterpret_cast<int>(efp->exfunc) - 0xa41e0;
}

/**
 * @brief フィルタ画面を更新
 *
 * @param efp フィルタ構造体へのポインタ
 */
void updateFilterWindow(ExEdit::Filter* efp) {
    debugPrintln("updateFilterWindow");

    Exdata* exdata = reinterpret_cast<Exdata*>(efp->exdata_ptr);
    auto get_hwnd = efp->exfunc->get_hwnd;

    // ドロップダウンリストの選択状態を更新
    SendMessage(get_hwnd(efp->processing, 6, 0), CB_SETCURSEL, getPanLawIndex(exdata->panlaw), 0);
    // ドロップダウンリストの隣にテキストを表示
    SetWindowText(get_hwnd(efp->processing, 7, 0), "Pan Law");
}

// フィルタのコールバック

/**
 * @brief フィルタ処理
 *
 * 参考
 * - https://www.cs.cmu.edu/~music/icm-online/readings/panlaws/
 * - https://ryukau.github.io/filter_notes/stereo_panning/stereo_panning.html
 *
 * @param efp
 * @param efpip
 * @return BOOL
 */
BOOL func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
    debugPrintln("func_proc");

    // 音声が2chではない場合は何もせず終了
    if (efpip->audio_ch != 2) {
        return TRUE;
    }

    Exdata* exdata = reinterpret_cast<Exdata*>(efp->exdata_ptr);

    // Pan Law
    PanLaw panlaw = exdata->panlaw;
    if (panlaw == PanLaw::Default) {
        panlaw = static_cast<PanLaw>(getProjectPanLaw());
    }

    // パン位置 [0, 1]
    float pan = static_cast<float>(efp->track[TRACK_ID_PAN] + 100) / 200.f;

    // ゲインの計算
    float l = 1.f;
    float r = 1.f;
    switch (panlaw) {
    case PanLaw::Default:
    case PanLaw::Minus3dB: {
        float theta = pan * std::numbers::pi / 2;
        l = std::cos(theta);
        r = std::sin(theta);
        break;
    }
    case PanLaw::Minus4_5dB: {
        float theta = pan * std::numbers::pi / 2;
        l = std::sqrt((1 - pan) * std::cos(theta));
        r = std::sqrt(pan * std::sin(theta));
        break;
    }
    case PanLaw::Minus6dB: {
        l = 1 - pan;
        r = pan;
        break;
    }
    }

    float ll = (pan >= 0.5f) ? 1.f : (pan + 0.5f);
    float rr = (pan >= 0.5F) ? 1.f : (1.5f - pan);
    float rl = 1 - ll;
    float lr = 1 - rr;

    ll *= l;
    rl *= l;
    lr *= r;
    rr *= r;

    debugPrintln("Pan Law: {}, Pan pos: {:.2f}", static_cast<int>(panlaw), pan);

    // フレームの音声データ
    int16_t* audiop;
    if (isFlagSet(efp->flag, ExEdit::Filter::Flag::Effect)) {
        audiop = efpip->audio_data;
    }
    else {
        audiop = efpip->audio_p;
    }

    // パンニングの適用
    for (int i = 0; i < efpip->audio_n; i++) {
        auto srcL = audiop[0];
        auto srcR = audiop[1];
        auto dstL = ll * srcL + rl * srcR;
        auto dstR = lr * srcL + rr * srcR;
        audiop[0] = dstL;
        audiop[1] = dstR;
        audiop += 2;
    }

    return TRUE;
}

/**
 * @brief フィルタがロードされてから最初に呼ばれる処理
 *
 * @param efp
 * @return BOOL
 */
BOOL func_init(ExEdit::Filter* efp) {
    debugPrintln("func_init");

    auto exeditDllHinst = getExEditDllHinst(efp);
    if (!update_any_exdata) {
        update_any_exdata = reinterpret_cast<decltype(update_any_exdata)>(exeditDllHinst + 0x4a7e0);
    }

    HMODULE configMod = GetModuleHandle("PannerConfig.auf");
    if (!configMod) {
        debugPrintln("PannerConfig.aufが見つかりませんでした");
        return FALSE;
    }

    getProjectPanLaw = reinterpret_cast<decltype(getProjectPanLaw)>(GetProcAddress(configMod, "getProjectPanLaw"));
    if (!getProjectPanLaw) {
        debugPrintln("getProjectPanLaw()が見つかりませんでした");
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief フィルタのウィンドウプロシージャ
 *
 * @param hwnd
 * @param message
 * @param wparam
 * @param lparam
 * @param editp
 * @param efp
 * @return BOOL
 */
BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, AviUtl::EditHandle* editp, ExEdit::Filter* efp) {
    debugPrintln("func_WndProc, msg: {:X}, wp: {:X}, lp: {:X}", message, wparam, lparam);

    if (message == ExEdit::ExtendedFilter::Message::WM_EXTENDEDFILTER_COMMAND) {
        // ドロップダウンリストが操作されたとき
        if (LOWORD(wparam) == ExEdit::ExtendedFilter::CommandId::EXTENDEDFILTER_SELECT_DROPDOWN) {
            int panlawId = std::clamp(static_cast<int>(lparam), 0, PAN_LAW_NUM - 1);
            auto panlaw = PAN_LAWS[panlawId];
            Exdata* exdata = reinterpret_cast<Exdata*>(efp->exdata_ptr);
            // Pan Lawが変更されたとき
            if (panlaw != exdata->panlaw) {
                efp->exfunc->set_undo(efp->processing, 0);
                exdata->panlaw = panlaw;
                update_any_exdata(efp->processing, EXDATA_USE[0].name);
                updateFilterWindow(efp);
            }
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * @brief フィルタ画面が表示されるときの処理？
 *
 * @param hinstance
 * @param hwnd
 * @param y
 * @param base_id
 * @param sw_param
 * @param efp
 * @return int
 */
int func_window_init(HINSTANCE hinstance, HWND hwnd, int y, int base_id, int sw_param, ExEdit::Filter* efp) {
    debugPrintln("func_window_init");
    updateFilterWindow(efp);
    return 0;
}

// AviUtl本体側へのエクスポート

ExEdit::Filter filterEffectDll{
    .flag = ExEdit::Filter::Flag::Audio
        | ExEdit::Filter::Flag::Effect,
    .name = FILTER_NAME,
    .track_n = TRACK_NUM,
    .track_name = const_cast<char**>(TRACK_NAME),
    .track_default = TRACK_DEFAULT,
    .track_s = TRACK_START,
    .track_e = TRACK_END,
    .check_n = CHECK_NUM,
    .check_name = const_cast<char**>(CHECK_NAME),
    .check_default = CHECK_DEFAULT,
    .func_proc = func_proc,
    .func_init = func_init,
    .func_WndProc = func_WndProc,
    .exdata_size = EXDATA_SIZE,
    .information = const_cast<char*>(FILTER_INFO),
    .func_window_init = func_window_init,
    .exdata_def = &EXDATA_DEF,
    .exdata_use = EXDATA_USE,
    .track_scale = TRACK_SCALE,
    .track_drag_min = TRACK_DRAG_MIN,
    .track_drag_max = TRACK_DRAG_MAX,
};
ExEdit::Filter filterDll{
    .flag = ExEdit::Filter::Flag::Audio,
    .name = FILTER_NAME,
    .track_n = TRACK_NUM,
    .track_name = const_cast<char**>(TRACK_NAME),
    .track_default = TRACK_DEFAULT,
    .track_s = TRACK_START,
    .track_e = TRACK_END,
    .check_n = CHECK_NUM,
    .check_name = const_cast<char**>(CHECK_NAME),
    .check_default = CHECK_DEFAULT,
    .func_proc = func_proc,
    .func_init = func_init,
    .func_WndProc = func_WndProc,
    .exdata_size = EXDATA_SIZE,
    .information = const_cast<char*>(FILTER_INFO),
    .func_window_init = func_window_init,
    .exdata_def = &EXDATA_DEF,
    .exdata_use = EXDATA_USE,
    .track_scale = TRACK_SCALE,
    .track_drag_min = TRACK_DRAG_MIN,
    .track_drag_max = TRACK_DRAG_MAX,
};
ExEdit::Filter* filterList[] = {
    &filterEffectDll,
    &filterDll,
    nullptr,
};

DLLEXPORT ExEdit::Filter** GetFilterTableList() {
    debugPrintln("GetFilterTableList");
    debugPrintln(__DATE__ " " __TIME__);
    return filterList;
}
