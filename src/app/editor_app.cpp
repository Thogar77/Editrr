#include "editrr/app/editor_app.hpp"

#include <sys/ioctl.h>
#include <unistd.h>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>

namespace editrr {

    static int get_window_size(int& rows, int& cols) {
        winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) return -1;
        cols = ws.ws_col;
        rows = ws.ws_row - 2;  // status + message bar
        if (rows < 1) rows = 1;
        return 0;
    }

    void EditorContext::set_status(const char* fmt, ...) {
        char buf[256];
        va_list ap;
        va_start(ap, fmt);
        std::vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);

        status.text = buf;
        status.time = std::time(nullptr);
    }

    void EditorContext::clamp_cursor() { vp.clamp_cursor(doc, cur); }

    void EditorContext::on_row_changed(int row_idx) {
        if (!syntax) return;

        // highlight single row
        const bool changed = syntax->highlight_row(doc, row_idx);

        if (changed) {
            syntax->highlight_from(doc, row_idx + 1);
        }
    }

    void EditorContext::rehighlight_all() {
        if (!syntax) return;
        syntax->set_filename(doc.filename());
        syntax->highlight_from(doc, 0);
    }

    void EditorContext::move_cursor(KeyCode code) {
        switch (code) {
        case KeyCode::ArrowLeft:
            if (cur.x != 0) {
                cur.x--;
            }
            else if (cur.y > 0) {
                cur.y--;
                cur.x = (int)doc.row(cur.y).chars.size();
            }
            break;

        case KeyCode::ArrowRight:
            if (cur.y == doc.num_rows()) break;
            if (cur.x < (int)doc.row(cur.y).chars.size()) {
                cur.x++;
            }
            else if (cur.x == (int)doc.row(cur.y).chars.size() && cur.y < doc.num_rows() - 1) {
                cur.y++;
                cur.x = 0;
            }
            break;

        case KeyCode::ArrowUp:
            if (cur.y > 0) cur.y--;
            break;

        case KeyCode::ArrowDown:
            if (cur.y < doc.num_rows()) cur.y++;
            break;

        case KeyCode::PageUp:
            cur.y = (cur.y > vp.height) ? (cur.y - vp.height) : 0;
            break;

        case KeyCode::PageDown:
            cur.y = std::min(cur.y + vp.height, doc.num_rows());
            break;

        case KeyCode::Home:
            cur.x = 0;
            break;

        case KeyCode::End:
            cur.x = (cur.y < doc.num_rows()) ? (int)doc.row(cur.y).chars.size() : 0;
            break;

        default:
            break;
        }

        clamp_cursor();
    }

    void EditorContext::insert_char(char c) {
        if (cur.y == doc.num_rows()) {
            doc.insert_row(doc.num_rows(), "");
            on_row_changed(doc.num_rows() - 1);
        }

        doc.row_insert_char(cur.y, cur.x, c);
        cur.x++;

        on_row_changed(cur.y);
    }

    void EditorContext::insert_newline() {
        if (cur.y == doc.num_rows()) {
            doc.insert_row(doc.num_rows(), "");
            on_row_changed(doc.num_rows() - 1);
            cur.y++;
            cur.x = 0;
            return;
        }

        if (cur.x == 0) {
            doc.insert_row(cur.y, "");
            on_row_changed(cur.y);
            if (syntax) syntax->highlight_from(doc, cur.y);
        }
        else {
            Row& row = doc.row(cur.y);

            std::string right = row.chars.substr(cur.x);
            row.chars.erase(cur.x);
            Document::rebuild_render(row);
            doc.set_dirty(true);

            doc.insert_row(cur.y + 1, right);

            on_row_changed(cur.y);
            on_row_changed(cur.y + 1);
            if (syntax) syntax->highlight_from(doc, cur.y + 1);
        }

        cur.y++;
        cur.x = 0;
    }

    void EditorContext::delete_char(bool delete_key) {
        if (cur.y == doc.num_rows()) return;
        if (doc.num_rows() == 0) return;

        if (!delete_key) {
            // BACKSPACE
            if (cur.x == 0) {
                if (cur.y == 0) return;

                int prev_len = (int)doc.row(cur.y - 1).chars.size();

                // join current into previous
                doc.row_append_string(cur.y - 1, doc.row(cur.y).chars);
                doc.delete_row(cur.y);

                cur.y--;
                cur.x = prev_len;

                on_row_changed(cur.y);
                if (syntax) syntax->highlight_from(doc, cur.y + 1);
                return;
            }

            // delete char before cursor
            doc.row_delete_char(cur.y, cur.x - 1);
            cur.x--;

            on_row_changed(cur.y);
            return;
        }

        // DELETE key (delete at cursor)
        Row& row = doc.row(cur.y);
        if (cur.x < (int)row.chars.size()) {
            doc.row_delete_char(cur.y, cur.x);
            on_row_changed(cur.y);
        }
        else {
            // at end -> join with next row
            if (cur.y < doc.num_rows() - 1) {
                doc.row_append_string(cur.y, doc.row(cur.y + 1).chars);
                doc.delete_row(cur.y + 1);

                on_row_changed(cur.y);
                if (syntax) syntax->highlight_from(doc, cur.y + 1);
            }
        }
    }

    void EditorApp::init_terminal(EditorContext& ctx) {
        int rows, cols;
        if (get_window_size(rows, cols) == -1) std::exit(1);
        ctx.vp.height = rows;
        ctx.vp.width = cols;
    }

    void EditorApp::process_key(EditorContext& ctx, const Key& k) {
        // Ctrl-Q quit protection
        if (k.ctrl && k.ch == 'q') {
            if (ctx.doc.dirty() && ctx.quit_times > 0) {
                ctx.set_status("WARNING!!! Unsaved changes. Press Ctrl-Q %d more times to quit.", ctx.quit_times);
                ctx.quit_times--;
                return;
            }
            ctx.renderer->clear_screen();
            ctx.running = false;
            return;
        }

        // Ctrl-S save (with Save as prompt)
        if (k.ctrl && k.ch == 's') {
            std::string path = ctx.doc.filename();
            if (path.empty()) {
                auto name = ctx.prompt.prompt(
                    *ctx.input, *ctx.renderer, ctx.doc, ctx.cur, ctx.vp, ctx.status,
                    ctx.doc.filename(), ctx.doc.dirty(), "Save as: %s (ESC to cancel)");
                if (!name) {
                    ctx.set_status("Save aborted");
                    return;
                }
                path = *name;
            }

            std::string err;
            if (ctx.file.save(ctx.doc, path, err)) {
                ctx.rehighlight_all();
                ctx.set_status("%zu bytes written", ctx.doc.to_string().size());
            }
            else {
                ctx.set_status("Can't save! I/O error: %s", err.c_str());
            }
            return;
        }

        // Ctrl-F find (interactive prompt + arrows navigate)
        if (k.ctrl && k.ch == 'f') {
            const int saved_cx = ctx.cur.x;
            const int saved_cy = ctx.cur.y;
            const int saved_coloff = ctx.vp.coloff;
            const int saved_rowoff = ctx.vp.rowoff;

            ctx.find_last_match = -1;
            ctx.find_direction = 1;

            auto query = ctx.prompt.prompt(
                *ctx.input, *ctx.renderer, ctx.doc, ctx.cur, ctx.vp, ctx.status,
                ctx.doc.filename(), ctx.doc.dirty(), "Search: %s (ESC cancel, arrows navigate)",
                [&](const std::string& q, const Key& kk) {
                    if (kk.code == KeyCode::Enter || kk.code == KeyCode::Esc) return;

                    if (kk.code == KeyCode::ArrowRight || kk.code == KeyCode::ArrowDown) ctx.find_direction = 1;
                    else if (kk.code == KeyCode::ArrowLeft || kk.code == KeyCode::ArrowUp) ctx.find_direction = -1;
                    else {
                        ctx.find_last_match = -1;
                        ctx.find_direction = 1;
                    }

                    ctx.search.find_next(ctx.doc, ctx.cur, ctx.vp, q, ctx.find_last_match, ctx.find_direction);
                });

            if (!query) {
                ctx.cur.x = saved_cx;
                ctx.cur.y = saved_cy;
                ctx.vp.coloff = saved_coloff;
                ctx.vp.rowoff = saved_rowoff;
            }
            return;
        }

        if (k.code == KeyCode::Enter) {
            ctx.insert_newline();
            ctx.quit_times = 3;
            return;
        }

        if (auto cmd = dispatcher_.map_key_to_command(k)) {
            cmd->execute(ctx);
        }

        ctx.quit_times = 3;
    }

    void EditorApp::run(const std::string& path) {
        EditorContext ctx;
        ctx.input = &input_;
        ctx.renderer = &renderer_;

        // create syntax strategy
        ctx.syntax = make_default_highlighter();

        ctx.set_status("HELP: Ctrl-Q quit | Ctrl-S save | Ctrl-F find");
        init_terminal(ctx);
        input_.enable_raw_mode();

        if (!path.empty()) {
            std::string err;
            if (!ctx.file.open(ctx.doc, path, err)) {
                ctx.set_status("Can't open: %s", err.c_str());
            }
            else {
                ctx.rehighlight_all();
            }
        }
        else {
            ctx.rehighlight_all();
        }

        while (ctx.running) {
            int rows, cols;
            if (get_window_size(rows, cols) == -1) std::exit(1);
            ctx.vp.height = rows;
            ctx.vp.width = cols;
            renderer_.refresh(ctx.doc, ctx.cur, ctx.vp, ctx.status, ctx.doc.filename(), ctx.doc.dirty());
            Key k = input_.read_key();
            process_key(ctx, k);
        }

        input_.restore();
    }

} // namespace editrr