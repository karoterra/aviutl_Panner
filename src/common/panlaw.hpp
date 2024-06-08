#pragma once

enum class PanLaw {
    Default = 0,
    Minus3dB = 1,
    Minus4_5dB = 2,
    Minus6dB = 3,
};

const char* panLawToString(PanLaw panLaw) {
    switch (panLaw) {
    case PanLaw::Default: return "プロジェクトの設定";
    case PanLaw::Minus3dB: return "-3dB";
    case PanLaw::Minus4_5dB: return "-4.5dB";
    case PanLaw::Minus6dB: return "-6dB";
    default: return "";
    }
}
