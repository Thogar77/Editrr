#pragma once
#include "editrr/document/document.hpp"
#include <string>

namespace editrr {

    class FileService {
    public:
        bool open(Document& doc, const std::string& path, std::string& err);
        bool save(Document& doc, const std::string& path, std::string& err);
    };

} // namespace editrr