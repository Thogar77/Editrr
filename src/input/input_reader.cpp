#include "editrr/input/input_reader.hpp"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

namespace editrr {

    namespace {

        static int read_byte_retry(char& out, int attempts = 12) {
            for (int i = 0; i < attempts; ++i) {
                const ssize_t n = ::read(STDIN_FILENO, &out, 1);
                if (n == 1) return 1;
                if (n == 0) continue;
                if (n == -1 && errno == EAGAIN) continue;
                return -1;
            }
            return 0;
        }

        static constexpr KeyCode kTextCode = static_cast<KeyCode>(0);

        static bool is_final_csi(char c) {
            return (c >= '@' && c <= '~'); // standard CSI final range
        }

        struct CsiInfo {
            char final{ 0 };
            int p1{ 0 };
            int p2{ 0 };
            bool ctrl{ false };
            bool alt{ false };
            bool shift{ false };
        };

        static CsiInfo parse_csi(const std::string& s) {
            // s to np. "A" albo "1;5A" albo "3~" albo "1;5D"
            CsiInfo out{};
            if (s.empty()) return out;

            out.final = s.back();

            // odetnij final
            std::string body = s.substr(0, s.size() - 1);

            // w body mogą być parametry: "1;5" albo "3" albo ""
            // parser: p1 ; p2
            auto read_int = [](const std::string& str, size_t& i) -> int {
                int v = 0;
                bool any = false;
                while (i < str.size() && str[i] >= '0' && str[i] <= '9') {
                    any = true;
                    v = v * 10 + (str[i] - '0');
                    ++i;
                }
                return any ? v : 0;
                };

            size_t i = 0;
            out.p1 = read_int(body, i);
            if (i < body.size() && body[i] == ';') {
                ++i;
                out.p2 = read_int(body, i);
            }

            // Xterm mod: 2=Shift, 3=Alt, 5=Ctrl, 6=Shift+Ctrl, 7=Alt+Ctrl, 8=Shift+Alt+Ctrl
            const int mod = out.p2;
            if (mod == 2 || mod == 6 || mod == 8) out.shift = true;
            if (mod == 3 || mod == 7 || mod == 8) out.alt = true;
            if (mod == 5 || mod == 6 || mod == 7 || mod == 8) out.ctrl = true;

            return out;
        }

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

        while (true) {
            const ssize_t n = ::read(STDIN_FILENO, &c, 1);
            if (n == 1) break;
            if (n == -1 && errno != EAGAIN) {
                perror("read");
                std::exit(1);
            }
        }

        // ===== ESC sequences =====
        if (c == '\x1b') {
            char first = 0;
            if (read_byte_retry(first) != 1) return Key{ KeyCode::Esc, 0, false };

            if (first == '[') {
                std::string seq;
                seq.reserve(16);

                for (int k = 0; k < 16; ++k) {
                    char b = 0;
                    if (read_byte_retry(b) != 1) break;
                    seq.push_back(b);
                    if (is_final_csi(b)) break;
                }

                const CsiInfo info = parse_csi(seq);

                if (info.final == 'A') return Key{ KeyCode::ArrowUp, 0, info.ctrl };
                if (info.final == 'B') return Key{ KeyCode::ArrowDown, 0, info.ctrl };
                if (info.final == 'C') return Key{ KeyCode::ArrowRight, 0, info.ctrl };
                if (info.final == 'D') return Key{ KeyCode::ArrowLeft, 0, info.ctrl };
                if (info.final == 'H') return Key{ KeyCode::Home, 0, info.ctrl };
                if (info.final == 'F') return Key{ KeyCode::End, 0, info.ctrl };

                if (info.final == '~') {
                    switch (info.p1) {
                    case 1: return Key{ KeyCode::Home, 0, info.ctrl };
                    case 3: return Key{ KeyCode::DeleteKey, 0, info.ctrl };
                    case 4: return Key{ KeyCode::End, 0, info.ctrl };
                    case 5: return Key{ KeyCode::PageUp, 0, info.ctrl };
                    case 6: return Key{ KeyCode::PageDown, 0, info.ctrl };
                    case 7: return Key{ KeyCode::Home, 0, info.ctrl };
                    case 8: return Key{ KeyCode::End, 0, info.ctrl };
                    }
                }

                return Key{ KeyCode::Esc, 0, false };
            }

            if (first == 'O') {
                char b = 0;
                if (read_byte_retry(b) != 1) return Key{ KeyCode::Esc, 0, false };
                switch (b) {
                case 'A': return Key{ KeyCode::ArrowUp, 0, false };
                case 'B': return Key{ KeyCode::ArrowDown, 0, false };
                case 'C': return Key{ KeyCode::ArrowRight, 0, false };
                case 'D': return Key{ KeyCode::ArrowLeft, 0, false };
                case 'H': return Key{ KeyCode::Home, 0, false };
                case 'F': return Key{ KeyCode::End, 0, false };
                }
                return Key{ KeyCode::Esc, 0, false };
            }

            return Key{ kTextCode, first, false };
        }

        if (c == '\r') return Key{ KeyCode::Enter, 0, false };

        if (static_cast<unsigned char>(c) == 127) return Key{ KeyCode::Backspace, 0, false };

        if (static_cast<unsigned char>(c) == 8) return Key{ KeyCode::Backspace, 0, true };

        if (static_cast<unsigned char>(c) <= 26) {
            const char ch = static_cast<char>(c + 'a' - 1);
            return Key{ kTextCode, ch, true };
        }

        return Key{ kTextCode, c, false };
    }

} // namespace editrr