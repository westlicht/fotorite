#include "catalog.h"

#include <fmt/std.h>
#include <spdlog/spdlog.h>

FR_NAMESPACE_BEGIN

Catalog::Catalog(const std::filesystem::path &root_path) : _root_path(root_path) { refresh(); }

Catalog::~Catalog() {}

CatalogDirPtr CatalogDir::load(const std::filesystem::path &path, CatalogDir *parent) {
    CatalogDirPtr dir = std::make_shared<CatalogDir>(path, parent);

    for (const auto &it : std::filesystem::directory_iterator(path)) {
        if (it.is_directory()) {
            spdlog::info("directory {}", it.path());
            dir->_dirs.push_back(load(it.path(), dir.get()));
        } else if (it.is_regular_file()) {
            spdlog::info("file {}", it.path().generic_string());
            CatalogFile file(it.path(), dir.get());
            dir->_files.push_back(file);
        }
    }

    return dir;
}

void Catalog::refresh() { _root_dir = CatalogDir::load(_root_path, nullptr); }

FR_NAMESPACE_END
