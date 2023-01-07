#include <fmt/std.h>
#include <spdlog/spdlog.h>

#include <filesystem>

#include "core/imageio.h"

int main() {
    spdlog::info("just a test {}", 123);

    std::filesystem::path root_dir{"C:/projects/fotorite/photos"};

    for (const auto &it : std::filesystem::directory_iterator{root_dir}) {
        if (it.is_regular_file()) {
            spdlog::info("reading {}", it.path());

            auto image = fr::ImageInput::open(it.path());
            if (!image) {
                spdlog::warn("failed to open {}", it.path());
                continue;
            }

            const fr::ImageSpec &spec = image->get_spec();

            spdlog::info("image size: {}x{}", spec.width, spec.height);

            std::vector<uint8_t> pixels(spec.width * spec.height * spec.component_count);
            image->read_image(pixels.data(), pixels.size());
        }
    }

    return 0;
}
