#pragma once
#include <optional>
#include <string>
#include <vector>

#include "editrr/document/gap_buffer.hpp"
#include "editrr/document/row.hpp"

namespace editrr {

class Document {
 public:
  int num_rows() const { return (int)rows_.size(); }
  bool empty() const { return rows_.empty(); }

  Row& row(int i) { return rows_.at(i); }
  const Row& row(int i) const { return rows_.at(i); }

  const std::vector<Row>& rows() const { return rows_; }
  std::vector<Row>& rows() { return rows_; }

  bool dirty() const { return dirty_; }
  void set_dirty(bool v) { dirty_ = v; }

  const std::string& filename() const { return filename_; }
  void set_filename(std::string name) { filename_ = std::move(name); }

  void load_from_string(const std::string& text);

  void insert_row(int at, const std::string& s);
  void delete_row(int at);

  void row_insert_char(int row_idx, int at, char c);
  void row_delete_char(int row_idx, int at);
  void row_append_string(int row_idx, const std::string& s);
  std::string row_text(int row_idx) const;

  void split_row(int row_idx, int at);

  std::string to_string() const { return buf_.to_string(); };

  static void rebuild_render(Row& r, std::string_view text);
  void rebuild_rows_from(size_t buf_pos);
  void update_offsets_from(int row_idx);

 private:
  std::vector<Row> rows_;
  bool dirty_{false};
  std::string filename_;
  GapBuffer buf_;
};

}  // namespace editrr