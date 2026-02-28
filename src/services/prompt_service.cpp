#include "editrr/services/prompt_service.hpp"
#include <cstdio>
#include <cctype>
#include <ctime>

namespace editrr {

    std::optional<std::string> PromptService::prompt(InputReader& in, Renderer& r,
        Document& doc, Cursor& cur, Viewport& vp,
        StatusMessage& status,
        const std::string& filename, bool dirty,
        const char* fmt,
        std::function<void(const std::string&, const Key&)> cb) {
        std::string buf;
        buf.reserve(128);

        while (true) {
            char tmp[256];
            std::snprintf(tmp, sizeof(tmp), fmt, buf.c_str());
            status.text = tmp;
            status.time = std::time(nullptr);

            r.refresh(doc, cur, vp, status, filename, dirty);

            Key k = in.read_key();

            if (k.code == KeyCode::Backspace || (k.ctrl && k.ch == 'h')) {
                if (!buf.empty()) buf.pop_back();
            }
            else if (k.code == KeyCode::Esc) {
                status.text.clear();
                status.time = std::time(nullptr);
                if (cb) cb(buf, k);
                return std::nullopt;
            }
            else if (k.code == KeyCode::Enter) {
                status.text.clear();
                status.time = std::time(nullptr);
                if (cb) cb(buf, k);
                return buf;
            }
            else if (k.ch && std::isprint((unsigned char)k.ch)) {
                buf.push_back(k.ch);
            }

            if (cb) cb(buf, k);
        }
    }

} // namespace editrr