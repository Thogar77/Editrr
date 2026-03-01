#include "editrr/commands/dispatcher.hpp"
#include "editrr/app/editor_app.hpp"
#include <memory>

namespace editrr {

    struct MoveCursorCmd : ICommand {
        KeyCode dir;
        explicit MoveCursorCmd(KeyCode d) : dir(d) {}
        void execute(EditorContext& ctx) override;
    };

    struct InsertCharCmd : ICommand {
        char c;
        explicit InsertCharCmd(char ch) : c(ch) {}
        void execute(EditorContext& ctx) override;
    };

    struct DeleteCharCmd : ICommand {
        bool del_key;
        explicit DeleteCharCmd(bool dk) : del_key(dk) {}
        void execute(EditorContext& ctx) override;
    };

    std::unique_ptr<ICommand> Dispatcher::map_key_to_command(const Key& k) {
        if (k.code == KeyCode::ArrowUp || k.code == KeyCode::ArrowDown ||
            k.code == KeyCode::ArrowLeft || k.code == KeyCode::ArrowRight ||
            k.code == KeyCode::Home || k.code == KeyCode::End ||
            k.code == KeyCode::PageUp || k.code == KeyCode::PageDown) {
            return std::make_unique<MoveCursorCmd>(k.code);
        }

        if (k.code == KeyCode::Backspace) return std::make_unique<DeleteCharCmd>(false);
        if (k.code == KeyCode::DeleteKey) return std::make_unique<DeleteCharCmd>(true);

        if (k.ch && k.ch >= 32 && k.ch <= 126) return std::make_unique<InsertCharCmd>(k.ch);

        return {};
    }

    void MoveCursorCmd::execute(EditorContext& ctx) { ctx.move_cursor(dir); }
    void InsertCharCmd::execute(EditorContext& ctx) { ctx.insert_char(c); }
    void DeleteCharCmd::execute(EditorContext& ctx) { ctx.delete_char(del_key); }

} // namespace editrr