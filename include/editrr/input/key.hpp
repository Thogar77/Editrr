#pragma once
#include <cstdint>

namespace editrr {

    enum class KeyCode : int32_t {
        Unknown = 0,

        Esc = 27,
        Enter = '\r',
        Backspace = 127,

        ArrowUp = 1000,
        ArrowDown,
        ArrowLeft,
        ArrowRight,
        Home,
        End,
        PageUp,
        PageDown,
        DeleteKey,
    };

    struct Key {
        KeyCode code{ KeyCode::Unknown };
        char ch{ 0 };
        bool ctrl{ false };
    };

    constexpr int ctrl_key(char c) { return (c & 0x1f); }

} // namespace editrr