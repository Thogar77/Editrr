#pragma once
#include "editrr/document/row.hpp"
#include <string>
#include <vector>
#include <optional>

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

        void insert_row(int at, const std::string& s);
        void delete_row(int at);

        void row_insert_char(int row_idx, int at, char c);
        void row_delete_char(int row_idx, int at);
        void row_append_string(int row_idx, const std::string& s);

        std::string to_string() const;

        static void rebuild_render(Row& r);

    private:
        std::vector<Row> rows_;
        bool dirty_{ false };
        std::string filename_;
    };

} // namespace editrr