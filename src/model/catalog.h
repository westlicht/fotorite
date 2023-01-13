#pragma once

#include "core/defs.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

FR_NAMESPACE_BEGIN

class CatalogDir;
using CatalogDirPtr = std::shared_ptr<CatalogDir>;

class CatalogFile {
public:
    CatalogFile(const std::filesystem::path &path, CatalogDir *parent) : m_path(path), m_parent(parent)
    {
        m_name = m_path.filename().string();
    }

    const std::filesystem::path &path() const { return m_path; }
    const std::string &name() const { return m_name; }

    CatalogDir *parent() const { return m_parent; }

private:
    std::filesystem::path m_path;
    std::string m_name;
    CatalogDir *m_parent;
};

class CatalogDir {
public:
    CatalogDir(const std::filesystem::path &path, CatalogDir *parent) : m_path(path), m_parent(parent)
    {
        m_name = m_path.filename().string();
    }

    const std::filesystem::path &path() const { return m_path; }
    const std::string &name() const { return m_name; }

    CatalogDir *parent() const { return m_parent; }

    static CatalogDirPtr load(const std::filesystem::path &path, CatalogDir *parent);

private:
    std::filesystem::path m_path;
    std::string m_name;
    CatalogDir *m_parent;

    std::vector<CatalogDirPtr> m_dirs;
    std::vector<CatalogFile> m_files;
};

class Catalog {
public:
    Catalog(const std::filesystem::path &root_path);
    ~Catalog();

    void refresh();

private:
    std::filesystem::path m_root_path;

    CatalogDirPtr m_root_dir;
};

FR_NAMESPACE_END
