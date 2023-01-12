#include "catalog.h"

#include "core/timer.h"
#include "core/imageio.h"

#include <fmt/std.h>
#include <spdlog/spdlog.h>
#include <BS_thread_pool.hpp>

FR_NAMESPACE_BEGIN

static BS::thread_pool s_thread_pool;

inline void load_image(std::filesystem::path path) {
    auto image = ImageInput::open(path);
    if (!image) {
        spdlog::warn("failed to open {}", path);
        return;
    }

    const ImageSpec &spec = image->spec();
    spdlog::info("loading {}", path);
    std::vector<uint8_t> pixels(spec.width * spec.height * spec.component_count);
    image->read_image(pixels.data(), pixels.size());
}

Catalog::Catalog(const std::filesystem::path &root_path) : _root_path(root_path) { refresh(); }

Catalog::~Catalog() {}

CatalogDirPtr CatalogDir::load(const std::filesystem::path &path, CatalogDir *parent) {
    CatalogDirPtr dir = std::make_shared<CatalogDir>(path, parent);

    for (const auto &it : std::filesystem::directory_iterator(path)) {
        if (it.is_directory()) {
            // spdlog::info("directory {}", it.path());
            dir->_dirs.push_back(load(it.path(), dir.get()));
        } else if (it.is_regular_file()) {
            if (!it.path().has_extension())
                continue;

            std::string ext = it.path().extension().string();
            for (auto &c : ext)
                c = ::tolower(c);

            if (ext != ".jpg" && ext != ".jpeg")
                continue;

            // spdlog::info("file {}", it.path().generic_string());
            CatalogFile file(it.path(), dir.get());
            dir->_files.push_back(file);

            s_thread_pool.push_task(load_image, it.path());
        }
    }

    return dir;
}

void Catalog::refresh() {
    Timer timer;
    _root_dir = CatalogDir::load(_root_path, nullptr);
    spdlog::info("enumerating catalog files took {}s", timer.elapsed());

    s_thread_pool.wait_for_tasks();
    spdlog::info("loading all images took {}s", timer.elapsed());
}

FR_NAMESPACE_END
