#include <fmt/std.h>
#include <spdlog/spdlog.h>
#include <nanogui/nanogui.h>

#include <filesystem>

#include "core/imageio.h"
#include "core/timer.h"

static void guitest() {
    nanogui::init();

    {
        nanogui::ref<nanogui::Screen> screen;
        screen = new nanogui::Screen(nanogui::Vector2i(500, 700), "fotorite");

        screen->set_visible(true);
        screen->perform_layout();

        nanogui::mainloop();
    }

    nanogui::shutdown();
}

int main() {
    spdlog::info("just a test {}", 123);

    guitest();

    std::filesystem::path root_dir{"C:/projects/fotorite/photos"};

    for (const auto &it : std::filesystem::directory_iterator{root_dir}) {
        if (it.is_regular_file()) {
            spdlog::info("reading {}", it.path());

            fr::Timer timer;

            auto image = fr::ImageInput::open(it.path());
            if (!image) {
                spdlog::warn("failed to open {}", it.path());
                continue;
            }

            const fr::ImageSpec &spec = image->get_spec();

            spdlog::info("image size: {}x{}", spec.width, spec.height);

            std::vector<uint8_t> pixels(spec.width * spec.height * spec.component_count);
            image->read_image(pixels.data(), pixels.size());

            spdlog::info("loading took {} ms", timer.elapsed() * 1000.0);
        }
    }

    return 0;
}
