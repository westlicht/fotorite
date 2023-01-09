#pragma once

#include "core/defs.h"

#include <filesystem>
#include <memory>

FR_NAMESPACE_BEGIN

class CatalogDir;
using CatalogDirPtr = std::shared_ptr<CatalogDir>;

class CatalogFile {
public:
    CatalogFile(const std::filesystem::path &path, CatalogDir *parent) : _path(path), _parent(parent) {
        _name = _path.filename().string();
    }

    const std::filesystem::path &path() const { return _path; }
    const std::string &name() const { return _name; }

    CatalogDir *parent() const { return _parent; }

private:
    std::filesystem::path _path;
    std::string _name;
    CatalogDir *_parent;
};

class CatalogDir {
public:
    CatalogDir(const std::filesystem::path &path, CatalogDir *parent) : _path(path), _parent(parent) {
        _name = _path.filename().string();
    }

    const std::filesystem::path &path() const { return _path; }
    const std::string &name() const { return _name; }

    CatalogDir *parent() const { return _parent; }

    static CatalogDirPtr load(const std::filesystem::path &path, CatalogDir *parent);

private:
    std::filesystem::path _path;
    std::string _name;
    CatalogDir *_parent;

    std::vector<CatalogDirPtr> _dirs;
    std::vector<CatalogFile> _files;
};

class Catalog {
public:
    Catalog(const std::filesystem::path &root_path);
    ~Catalog();

    void refresh();

private:
    std::filesystem::path _root_path;

    CatalogDirPtr _root_dir;
};

FR_NAMESPACE_END
