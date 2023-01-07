#include "imageio.h"

// clang-format off
#include <cstdio> // must be included before jpeglib.h
#include <jpeglib.h>
#include <png.h>
#include <TinyEXIF.h>
// clang-format on

#include <string>
#include <cctype>
#include <cstring>

FR_NAMESPACE_BEGIN

class ImageReader {
public:
    virtual ~ImageReader() {}

    virtual bool open(const std::filesystem::path &path) = 0;
    virtual bool read_spec(ImageSpec &spec) = 0;
    virtual bool read_image(void *buffer, size_t len) = 0;
};

class JPEGReader : public ImageReader {
public:
    JPEGReader() {
        _info.err = jpeg_std_error(&_err);
        _err.error_exit = [](j_common_ptr info) { throw info->err; };
        jpeg_create_decompress(&_info);
    }

    ~JPEGReader() {
        close();
        jpeg_destroy_decompress(&_info);
    }

    bool open(const std::filesystem::path &path) override {
        _file = fopen(path.string().c_str(), "rb");
        if (_file == NULL)
            return false;

        jpeg_stdio_src(&_info, _file);

        try {
            jpeg_read_header(&_info, TRUE);
            return true;
        } catch (jpeg_error_mgr *) {
            return false;
        }
    }

    void close() {
        if (!_file)
            return;

        fclose(_file);
        _file = NULL;
    }

    bool read_spec(ImageSpec &spec) override {
        spec.width = _info.image_width;
        spec.height = _info.image_height;
        spec.component_type = ComponentType::U8;
        spec.component_count = _info.num_components;

        // TODO read EXIF data

        return true;
    }

    bool read_image(void *buffer, size_t len) override {
        uint8_t *dst = reinterpret_cast<uint8_t *>(buffer);
        try {
            jpeg_start_decompress(&_info);

            int row_stride = _info.output_width * _info.output_components;
            JSAMPARRAY row = (*_info.mem->alloc_sarray)((j_common_ptr)&_info, JPOOL_IMAGE, row_stride, 1);

            while (_info.output_scanline < _info.output_height) {
                jpeg_read_scanlines(&_info, row, 1);

                std::memcpy(dst, row, row_stride);
                dst += row_stride;
                // TODO bounds checking
            }

            jpeg_finish_decompress(&_info);
        } catch (jpeg_error_mgr *) {
            return false;
        }

        ignore_unused(buffer, len);
        return false;
    }

private:
    jpeg_decompress_struct _info;
    jpeg_error_mgr _err;
    FILE *_file{0};
};

std::unique_ptr<ImageInput> ImageInput::open(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path))
        return nullptr;

    std::string ext = path.extension().string();
    for (auto &c : ext)
        c = char(std::tolower(c));

    std::unique_ptr<ImageReader> reader;

    if (ext == ".jpg" || ext == ".jpeg") {
        reader = std::make_unique<JPEGReader>();
    }

    if (!reader)
        return nullptr;

    if (!reader->open(path))
        return nullptr;

    ImageSpec spec;
    if (!reader->read_spec(spec))
        return nullptr;

    std::unique_ptr<ImageInput> image_input = std::make_unique<ImageInput>();
    image_input->_spec = std::move(spec);
    image_input->_reader = std::move(reader);

    return image_input;
}

ImageInput::~ImageInput() {}

bool ImageInput::read_image(void *buffer, size_t len) { return _reader->read_image(buffer, len); }

FR_NAMESPACE_END
