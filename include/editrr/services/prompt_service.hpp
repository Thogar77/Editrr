#pragma once
#include "editrr/input/input_reader.hpp"
#include "editrr/ui/renderer.hpp"
#include "editrr/core/types.hpp"
#include "editrr/core/viewport.hpp"
#include "editrr/document/document.hpp"
#include <optional>
#include <functional>
#include <string>

namespace editrr {

    class PromptService {
    public:
        std::optional<std::string> prompt(InputReader& in, Renderer& r,
            Document& doc, Cursor& cur, Viewport& vp,
            StatusMessage& status,
            const std::string& filename, bool dirty,
            const char* fmt,
            std::function<void(const std::string&, const Key&)> cb = {});
    };

} // namespace editrr