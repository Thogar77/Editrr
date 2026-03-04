#pragma once

#include <memory>
#include <string>

#include "editrr/commands/dispatcher.hpp"
#include "editrr/core/types.hpp"
#include "editrr/core/viewport.hpp"
#include "editrr/document/document.hpp"
#include "editrr/input/input_reader.hpp"
#include "editrr/services/file_service.hpp"
#include "editrr/services/prompt_service.hpp"
#include "editrr/services/search_service.hpp"
#include "editrr/services/syntax/syntax.hpp"
#include "editrr/ui/renderer.hpp"

namespace editrr {

struct EditorState {
  Document doc;
  Cursor cur;
  Viewport vp;
  StatusMessage status;
  std::unique_ptr<ISyntaxHighlighter> syntax;
  Mode current_mode;

  bool running{true};
  int find_last_match{-1};
  int find_direction{1};
  int quit_times{3};

  void set_status(const char* fmt, ...);
  void clamp_cursor();
  void move_cursor(KeyCode code);
  void insert_char(char c);
  void insert_newline();
  void delete_char(bool delete_key);
  void on_row_changed(int row_idx);
  void rehighlight_all();
};

struct EditorServices {
  InputReader* input = nullptr;
  Renderer* renderer = nullptr;
  FileService* file = nullptr;
  SearchService* search = nullptr;
  PromptService* prompt = nullptr;
};

class EditorApp {
 public:
  void run(const std::string& path = {});

 private:
  void process_key(EditorState& state, EditorServices& srv, const Key& k);
  void init_terminal(EditorState& ctx);

  InputReader input_;
  Renderer renderer_;
  FileService file_;
  SearchService search_;
  PromptService prompt_;
  Dispatcher dispatcher_;
};

}  // namespace editrr