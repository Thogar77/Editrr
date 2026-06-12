#include "editrr/app/editor_app.hpp"
int main(int argc, char** argv) {


  editrr::EditorApp app;
  if (argc >= 2) app.run(argv[1]);
  else app.run();
  return 0;
}