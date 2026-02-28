#include "editrr/services/file_service.hpp"
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

namespace editrr {

    bool FileService::open(Document& doc, const std::string& path, std::string& err) {
        std::ifstream in(path);
        if (!in) { err = "open_file: " + std::string(std::strerror(errno)); return false; }

        doc.rows().clear();
        doc.set_filename(path);

        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            doc.insert_row(doc.num_rows(), line);
        }

        doc.set_dirty(false);
        return true;
    }

    bool FileService::save(Document& doc, const std::string& path, std::string& err) {
        const std::string data = doc.to_string();

        int fd = ::open(path.c_str(), O_RDWR | O_CREAT, 0644);
        if (fd == -1) { err = std::strerror(errno); return false; }

        if (::ftruncate(fd, (off_t)data.size()) == -1) { ::close(fd); err = std::strerror(errno); return false; }

        ssize_t written = ::write(fd, data.data(), data.size());
        ::close(fd);

        if (written != (ssize_t)data.size()) { err = std::strerror(errno); return false; }

        doc.set_filename(path);
        doc.set_dirty(false);
        return true;
    }

} // namespace editrr