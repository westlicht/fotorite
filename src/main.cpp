#include <fmt/std.h>
#include <spdlog/spdlog.h>

#include <filesystem>

int main() {
    spdlog::info("just a test {}", 123);

    std::filesystem::path root_dir{"C:/projects/fotorite/photos"};

    for (const auto it : std::filesystem::directory_iterator{root_dir}) {
        if (it.is_regular_file()) {
            spdlog::info("reading {}", it.path());

            // auto inp = OIIO::ImageInput::open(it.path());
            // if (!inp)
            //     continue;
            // const OIIO::ImageSpec &spec = inp->spec();
            // int xres = spec.width;
            // int yres = spec.height;
            // int channels = spec.nchannels;
            // std::vector<float> pixels(xres * yres * channels);
            // spdlog::info("image data size {}", pixels.size());
            // inp->read_image(OIIO::TypeDesc::FLOAT, &pixels[0]);
            // inp->close();
        }
    }

    return 0;
}
