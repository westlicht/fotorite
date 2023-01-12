#include "imageio.h"
#include "fileio.h"
#include "stringutils.h"

// clang-format off
#include <cstdio> // must be included before jpeglib.h
#include <jpeglib.h>
#include <png.h>
#include <TinyEXIF.h>
// clang-format on

#include <cctype>
#include <cstring>
#include <string>

FR_NAMESPACE_BEGIN

// ----------------------------------------------------------------------------
// ImageReader
// ----------------------------------------------------------------------------

class ImageReader {
public:
    virtual ~ImageReader() {}

    virtual bool open(const std::filesystem::path &path, ImageSpec &out_spec) = 0;
    virtual bool open(const void *buffer, size_t len, ImageSpec &out_spec) = 0;
    virtual bool read_image(void *buffer, size_t len) = 0;
};

class ImageWriter {
public:
    virtual ~ImageWriter() {}

    virtual bool open(const std::filesystem::path &path, const ImageSpec &spec) = 0;
    virtual bool write_image(const void *buffer, size_t len) = 0;
};

// ----------------------------------------------------------------------------
// JPEGReader
// ----------------------------------------------------------------------------

class JPEGReader : public ImageReader {
public:
    JPEGReader()
    {
        m_info.err = jpeg_std_error(&m_err);
        m_err.error_exit = [](j_common_ptr info) { throw info->err; };
        jpeg_create_decompress(&m_info);
    }

    ~JPEGReader()
    {
        jpeg_destroy_decompress(&m_info);
        if (m_file)
            fclose(m_file);
    }

    bool open(const std::filesystem::path &path, ImageSpec &out_spec) override
    {
        m_file = fopen(path.string().c_str(), "rb");
        if (m_file == NULL)
            return false;

        jpeg_stdio_src(&m_info, m_file);

        try {
            jpeg_read_header(&m_info, TRUE);
            return true;
        } catch (jpeg_error_mgr *) {
            return false;
        }
    }

    bool open(const void *buffer, size_t len, ImageSpec &out_spec) override
    {
        jpeg_mem_src(&m_info, reinterpret_cast<const uint8_t *>(buffer), static_cast<unsigned long>(len));

        try {
            jpeg_read_header(&m_info, TRUE);

            out_spec.width = m_info.image_width;
            out_spec.height = m_info.image_height;
            out_spec.component_type = ComponentType::U8;
            out_spec.component_count = m_info.num_components;
            // TODO read exif

            return true;
        } catch (jpeg_error_mgr *) {
            return false;
        }
    }

    bool read_image(void *buffer, size_t len) override
    {
        uint8_t *dst = reinterpret_cast<uint8_t *>(buffer);
        try {
            jpeg_start_decompress(&m_info);

            int row_stride = m_info.output_width * m_info.output_components;
            JSAMPROW row[1];

            while (m_info.output_scanline < m_info.output_height) {
                row[0] = reinterpret_cast<JSAMPROW>(dst);
                jpeg_read_scanlines(&m_info, row, 1);
                dst += row_stride;
                // TODO bounds checking
            }

            jpeg_finish_decompress(&m_info);
        } catch (jpeg_error_mgr *) {
            return false;
        }

        return true;
    }

private:
    jpeg_decompress_struct m_info;
    jpeg_error_mgr m_err;
    FILE *m_file{NULL};
};

// ----------------------------------------------------------------------------
// JPEGWriter
// ----------------------------------------------------------------------------

class JPEGWriter : public ImageWriter {
public:
    JPEGWriter()
    {
        m_info.err = jpeg_std_error(&m_err);
        m_err.error_exit = [](j_common_ptr info) { throw info->err; };
        jpeg_create_compress(&m_info);
    }

    ~JPEGWriter()
    {
        jpeg_destroy_compress(&m_info);
        if (m_file)
            fclose(m_file);
    }

    bool open(const std::filesystem::path &path, const ImageSpec &spec) override
    {
        m_file = fopen(path.string().c_str(), "wb");
        if (m_file == NULL)
            return false;

        jpeg_stdio_dest(&m_info, m_file);

        m_info.image_width = spec.width;
        m_info.image_height = spec.height;
        m_info.input_components = spec.component_count;
        m_info.in_color_space = JCS_RGB;

        jpeg_set_defaults(&m_info);
        int quality = 80;  // TODO make configurable
        jpeg_set_quality(&m_info, quality, TRUE);

        return true;
    }

    bool write_image(const void *buffer, size_t len)
    {
        const uint8_t *src = reinterpret_cast<const uint8_t *>(buffer);
        try {
            jpeg_start_compress(&m_info, TRUE);

            int row_stride = m_info.image_width * m_info.input_components;
            JSAMPROW row[1];
            if (!row)
                throw std::bad_alloc();

            while (m_info.next_scanline < m_info.image_height) {
                row[0] = const_cast<JSAMPROW>(src);
                jpeg_write_scanlines(&m_info, row, 1);
                src += row_stride;
                // TODO bounds checking
            }

            jpeg_finish_compress(&m_info);
        } catch (jpeg_error_mgr *) {
            return false;
        }

        return true;
    }

private:
    jpeg_compress_struct m_info;
    jpeg_error_mgr m_err;
    FILE *m_file{NULL};
};

// ----------------------------------------------------------------------------
// PNGReader
// ----------------------------------------------------------------------------

class PNGReader : public ImageReader {};

// ----------------------------------------------------------------------------
// ImageInput
// ----------------------------------------------------------------------------

std::unique_ptr<ImageInput> ImageInput::open(const std::filesystem::path &path)
{
    if (!std::filesystem::exists(path))
        return nullptr;

    std::string ext = to_lower(path.extension().string());

    std::unique_ptr<ImageReader> reader;

    if (ext == ".jpg" || ext == ".jpeg") {
        reader = std::make_unique<JPEGReader>();
    }

    if (!reader)
        return nullptr;

    std::unique_ptr<MemoryMappedFile> file = std::make_unique<MemoryMappedFile>(path);
    if (!file->is_open())
        return nullptr;

    ImageSpec spec;
#if 0
    if (!reader->open(path, spec))
        return nullptr;
#else
    if (!reader->open(file->data(), file->mapped_size(), spec))
        return nullptr;
#endif

    std::unique_ptr<ImageInput> image_input = std::make_unique<ImageInput>();
    image_input->m_spec = std::move(spec);
    image_input->m_reader = std::move(reader);
    image_input->m_file = std::move(file);

    return image_input;
}

ImageInput::~ImageInput() {}

bool ImageInput::read_image(void *buffer, size_t len)
{
    FR_ASSERT(m_reader);
    return m_reader->read_image(buffer, len);
}

// ----------------------------------------------------------------------------
// ImageOutput
// ----------------------------------------------------------------------------

std::unique_ptr<ImageOutput> ImageOutput::open(const std::filesystem::path &path, ImageSpec spec)
{
    std::string ext = to_lower(path.extension().string());

    std::unique_ptr<ImageWriter> writer;

    if (ext == ".jpg" || ext == ".jpeg") {
        writer = std::make_unique<JPEGWriter>();
    }

    if (!writer)
        return nullptr;

    if (!writer->open(path, spec))
        return nullptr;

    std::unique_ptr<ImageOutput> image_output = std::make_unique<ImageOutput>();
    image_output->m_spec = std::move(spec);
    image_output->m_writer = std::move(writer);

    return image_output;
}

ImageOutput::~ImageOutput() {}

bool ImageOutput::write_image(const void *buffer, size_t len)
{
    FR_ASSERT(m_writer);
    return m_writer->write_image(buffer, len);
}

FR_NAMESPACE_END
