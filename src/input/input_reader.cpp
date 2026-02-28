#include "editrr/input/input_reader.hpp"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

namespace editrr {

    namespace {

        // Czytanie 1 bajtu z retry: kluczowe przy ESC sekwencjach (strzałki),
        // bo przy VMIN=0/VTIME>0 kolejne bajty mogą przyjść “później”.
        static int read_byte_retry(char& out, int attempts = 8) {
            for (int i = 0; i < attempts; ++i) {
                const ssize_t n = ::read(STDIN_FILENO, &out, 1);
                if (n == 1) return 1;
                if (n == 0) continue;                // timeout -> próbuj dalej
                if (n == -1 && errno == EAGAIN) continue;
                return -1;                           // real error
            }
            return 0;                              // nie udało się doczytać
        }

        // Neutralny KeyCode dla “zwykłych znaków”.
        // Nie zgadujemy czy masz KeyCode::None/Unknown, bierzemy 0.
        static constexpr KeyCode kTextCode = static_cast<KeyCode>(0);

    } // namespace

    InputReader::~InputReader() { restore(); }

    void InputReader::enable_raw_mode() {
        if (raw_enabled_) return;

        if (tcgetattr(STDIN_FILENO, &orig_) == -1) {
            perror("tcgetattr");
            std::exit(1);
        }

        termios raw = orig_;

        raw.c_iflag &= static_cast<tcflag_t>(~(BRKINT | ICRNL | INPCK | ISTRIP | IXON));
        raw.c_oflag &= static_cast<tcflag_t>(~(OPOST));
        raw.c_cflag |= static_cast<tcflag_t>(CS8);
        raw.c_lflag &= static_cast<tcflag_t>(~(ECHO | ICANON | IEXTEN | ISIG));

        // non-blocking read with timeout
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1; // 0.1s

        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
            perror("tcsetattr");
            std::exit(1);
        }

        raw_enabled_ = true;
    }

    void InputReader::restore() {
        if (!raw_enabled_) return;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_);
        raw_enabled_ = false;
    }

    Key InputReader::read_key() {
        char c = 0;

        // czekamy aż coś przyjdzie
        while (true) {
            const ssize_t n = ::read(STDIN_FILENO, &c, 1);
            if (n == 1) break;
            if (n == -1 && errno != EAGAIN) {
                perror("read");
                std::exit(1);
            }
        }

        // ===== ESC sequences (arrows, home/end, etc.) =====
        if (c == '\x1b') {
            char seq[3]{ 0, 0, 0 };

            if (read_byte_retry(seq[0]) != 1) return Key{ KeyCode::Esc, 0, false };
            if (read_byte_retry(seq[1]) != 1) return Key{ KeyCode::Esc, 0, false };

            if (seq[0] == '[') {
                if (seq[1] >= '0' && seq[1] <= '9') {
                    if (read_byte_retry(seq[2]) != 1) return Key{ KeyCode::Esc, 0, false };

                    if (seq[2] == '~') {
                        switch (seq[1]) {
                        case '1': return Key{ KeyCode::Home, 0, false };
                        case '3': return Key{ KeyCode::DeleteKey, 0, false };
                        case '4': return Key{ KeyCode::End, 0, false };
                        case '5': return Key{ KeyCode::PageUp, 0, false };
                        case '6': return Key{ KeyCode::PageDown, 0, false };
                        case '7': return Key{ KeyCode::Home, 0, false };
                        case '8': return Key{ KeyCode::End, 0, false };
                        }
                    }
                }
                else {
                    switch (seq[1]) {
                    case 'A': return Key{ KeyCode::ArrowUp, 0, false };
                    case 'B': return Key{ KeyCode::ArrowDown, 0, false };
                    case 'C': return Key{ KeyCode::ArrowRight, 0, false };
                    case 'D': return Key{ KeyCode::ArrowLeft, 0, false };
                    case 'H': return Key{ KeyCode::Home, 0, false };
                    case 'F': return Key{ KeyCode::End, 0, false };
                    }
                }
            }
            else if (seq[0] == 'O') {
                // niektóre terminale wysyłają ESC O A/B/C/D
                switch (seq[1]) {
                case 'A': return Key{ KeyCode::ArrowUp, 0, false };
                case 'B': return Key{ KeyCode::ArrowDown, 0, false };
                case 'C': return Key{ KeyCode::ArrowRight, 0, false };
                case 'D': return Key{ KeyCode::ArrowLeft, 0, false };
                case 'H': return Key{ KeyCode::Home, 0, false };
                case 'F': return Key{ KeyCode::End, 0, false };
                }
            }

            return Key{ KeyCode::Esc, 0, false };
        }

        // ===== ENTER =====
        if (c == '\r') return Key{ KeyCode::Enter, 0, false };

        // ===== BACKSPACE (DEL) =====
        if (static_cast<unsigned char>(c) == 127) return Key{ KeyCode::Backspace, 0, false };

        // ===== CTRL (Ctrl-A..Ctrl-Z) =====
        if (static_cast<unsigned char>(c) <= 26) {
            const char ch = static_cast<char>(c + 'a' - 1);
            return Key{ kTextCode, ch, true };
        }

        // ===== zwykły znak =====
        return Key{ kTextCode, c, false };
    }

} // namespace editrr