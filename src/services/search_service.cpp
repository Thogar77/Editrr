#include "editrr/services/search_service.hpp"

namespace editrr {

bool SearchService::find_next(Document& doc, Cursor& cur, Viewport& vp, const std::string& query,
                              int& last_match, int& direction) {
  if (query.empty() || doc.empty()) return false;

  int current = last_match;
  for (int i = 0; i < doc.num_rows(); i++) {
    current += direction;
    if (current == -1) current = doc.num_rows() - 1;
    if (current == doc.num_rows()) current = 0;

    const Row& row = doc.row(current);
    size_t pos = row.render.find(query);
    if (pos != std::string::npos) {
      last_match = current;
      cur.y = current;
      cur.x = vp.rx_to_cx(row.render, (int)pos);
      vp.rowoff = doc.num_rows();
      return true;
    }
  }
  return false;
}

}  // namespace editrr
