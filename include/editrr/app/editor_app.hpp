#pragma once

#include "editrr/core/types.hpp"
#include "editrr/core/viewport.hpp"
#include "editrr/document/document.hpp"
#include "editrr/input/input_reader.hpp"
#include "editrr/ui/renderer.hpp"
#include "editrr/commands/dispatcher.hpp"
#include "editrr/services/file_service.hpp"
#include "editrr/services/search_service.hpp"
#include "editrr/services/prompt_service.hpp"

#include "editrr/services/syntax/syntax.hpp"

#include <memory>
#include <string>

namespace editrr {

    struct EditorContext {
        Document doc;
        Cursor cur;
        Viewport vp;
        StatusMessage status;

        bool running{ true };

        int find_last_match{ -1 };
        int find_direction{ 1 };

        int quit_times{ 3 };

        FileService file;
        SearchService search;
        PromptService prompt;
        std::unique_ptr<ISyntaxHighlighter> syntax;

        InputReader* input{ nullptr };
        Renderer* renderer{ nullptr };

        void set_status(const char* fmt, ...);

        void clamp_cursor();

        void move_cursor(KeyCode code);

        void insert_char(char c);
        void insert_newline();

        void delete_char(bool delete_key);

        void on_row_changed(int row_idx);

        void rehighlight_all();
    };

    class EditorApp {
    public:
        void run(const std::string& path = {});

    private:
        void process_key(EditorContext& ctx, const Key& k);
        void init_terminal(EditorContext& ctx);

        Dispatcher dispatcher_;
        InputReader input_;
        Renderer renderer_;
    };

} // namespace editrr