#include "editrr/commands/dispatcher.hpp"

#include <memory>

#include "editrr/app/editor_app.hpp"
#include "editrr/config/app_config.hpp"

namespace editrr {

struct MoveCursorCmd : ICommand {
  KeyCode dir;
  explicit MoveCursorCmd(KeyCode d) : dir(d) {}
  void execute(EditorState& state, EditorServices& srv) override;
};

struct InsertCharCmd : ICommand {
  char c;
  explicit InsertCharCmd(char ch) : c(ch) {}
  void execute(EditorState& state, EditorServices& srv) override;
};

struct DeleteCharCmd : ICommand {
  bool del_key;
  explicit DeleteCharCmd(bool dk) : del_key(dk) {}
  void execute(EditorState& state, EditorServices& srv) override;
};

struct QuitCmd : ICommand {
  void execute(EditorState& state, EditorServices& srv) override;
};

struct SaveCmd : ICommand {
  void execute(EditorState& state, EditorServices& svc) override {
    std::string path = state.doc.filename();

    if (path.empty()) {
      auto name = svc.prompt->prompt(*svc.input, *svc.renderer, state.doc, state.cur, state.vp,
                                     state.status, state.doc.filename(), state.doc.dirty(),
                                     "Save as: %s (ESC to cancel)");

      if (!name) {
        state.set_status("Save aborted");
        return;
      }
      path = *name;
    }

    std::string err;
    if (svc.file->save(state.doc, path, err)) {
      state.rehighlight_all();
      state.set_status("%zu bytes written", state.doc.to_string().size());
    } else {
      state.set_status("Can't save! I/O error: %s", err.c_str());
    }
  }
};

struct FindCmd : ICommand {
  void execute(EditorState& state, EditorServices& svc) override {
    // Zapamiętaj pozycję przed szukaniem
    const int saved_cx = state.cur.x;
    const int saved_cy = state.cur.y;
    const int saved_coloff = state.vp.coloff;
    const int saved_rowoff = state.vp.rowoff;

    state.find_last_match = -1;
    state.find_direction = 1;

    auto query = svc.prompt->prompt(
        *svc.input, *svc.renderer, state.doc, state.cur, state.vp, state.status,
        state.doc.filename(), state.doc.dirty(), "Search: %s (ESC cancel, arrows navigate)",
        [&](const std::string& q, const Key& kk) {
          if (kk.code == KeyCode::Enter || kk.code == KeyCode::Esc) return;

          if (kk.code == KeyCode::ArrowRight || kk.code == KeyCode::ArrowDown)
            state.find_direction = 1;
          else if (kk.code == KeyCode::ArrowLeft || kk.code == KeyCode::ArrowUp)
            state.find_direction = -1;
          else {
            state.find_last_match = -1;
            state.find_direction = 1;
          }

          svc.search->find_next(state.doc, state.cur, state.vp, q, state.find_last_match,
                                state.find_direction);
        });

    // ESC — przywróć pozycję
    if (!query) {
      state.cur.x = saved_cx;
      state.cur.y = saved_cy;
      state.vp.coloff = saved_coloff;
      state.vp.rowoff = saved_rowoff;
    }
  }
};

struct EnterCmd : ICommand {
  void execute(EditorState& state, EditorServices& svc) override {
    state.insert_newline();
    state.quit_times = 3;
  }
};

std::unique_ptr<ICommand> Dispatcher::map_key_to_command(const Key& k) {
  // ===== Ctrl-klawisze =====
  if (k.ctrl && k.ch == char(config::Keybinding::QUIT)) return std::make_unique<QuitCmd>();
  if (k.ctrl && k.ch == char(config::Keybinding::SAVE)) return std::make_unique<SaveCmd>();
  if (k.ctrl && k.ch == char(config::Keybinding::FIND)) return std::make_unique<FindCmd>();

  // ===== Specjalne klawisze =====
  if (k.code == KeyCode::Enter) return std::make_unique<EnterCmd>();
  if (k.code == KeyCode::Backspace) return std::make_unique<DeleteCharCmd>(false);
  if (k.code == KeyCode::DeleteKey) return std::make_unique<DeleteCharCmd>(true);

  // ===== Ruch kursora =====
  if (k.code == KeyCode::ArrowUp || k.code == KeyCode::ArrowDown || k.code == KeyCode::ArrowLeft ||
      k.code == KeyCode::ArrowRight || k.code == KeyCode::Home || k.code == KeyCode::End ||
      k.code == KeyCode::PageUp || k.code == KeyCode::PageDown) {
    return std::make_unique<MoveCursorCmd>(k.code);
  }

  // ===== Zwykłe znaki =====
  if (k.ch && k.ch >= 32 && k.ch <= 126) return std::make_unique<InsertCharCmd>(k.ch);

  return {};
}

void MoveCursorCmd::execute(EditorState& state, EditorServices& srv) { state.move_cursor(dir); }
void InsertCharCmd::execute(EditorState& state, EditorServices& srv) { state.insert_char(c); }
void DeleteCharCmd::execute(EditorState& state, EditorServices& srv) { state.delete_char(del_key); }

void QuitCmd::execute(EditorState& state, EditorServices& srv) {
  if (state.doc.dirty() && state.quit_times > 0) {
    state.set_status("WARNING!!! Unsaved changes. Press Ctrl-Q %d more times to quit.",
                     state.quit_times);
    state.quit_times--;
    return;
  }
  srv.renderer->clear_screen();
  state.running = false;
}

}  // namespace editrr
