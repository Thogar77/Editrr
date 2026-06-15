#pragma once
namespace editrr {

struct EditorState;
struct EditorServices;

enum class Command { MoveCursor, InsertChar, DeleteChar, Quit, Save, Find};


struct ICommand {
  virtual ~ICommand() = default;
  virtual void execute(EditorState& state, EditorServices& srv) = 0;
};

}  // namespace editrr