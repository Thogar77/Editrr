#pragma once
#include "editrr/input/key.hpp"
#include <termios.h>

namespace editrr {

    class InputReader {
    public:
        InputReader() = default;
        ~InputReader();

        void enable_raw_mode();
        void restore();

        Key read_key();

    private:
        bool raw_enabled_{ false };
        termios orig_{};
    };

} // namespace editrr