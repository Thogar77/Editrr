#include <string>
#include <vector>
namespace editrr {
class GapBuffer {
 public:
  explicit GapBuffer(size_t initial_capacity = 4096);

  void insert(size_t pos, char c);
  void insert(size_t pos, std::string_view text);
  void erase(size_t pos, size_t count = 1);
  void clear();

  char at(size_t pos) const;
  size_t size() const { return buf_.size() - (gap_end_ - gap_start_); };
  size_t gap_size() const { return gap_end_ - gap_start_; };
  bool empty() const { return size() == 0; };

  std::string to_string() const;
  std::string substr(size_t pos, size_t len) const;

 private:
  std::vector<char> buf_;
  size_t gap_start_ = 0;
  size_t gap_end_ = 0;

  void move_gap_to(size_t pos);
  void grow_gap(size_t needed);
};
}  // namespace editrr