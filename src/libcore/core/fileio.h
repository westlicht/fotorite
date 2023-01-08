#pragma once

#include "defs.h"

#include <cstdint>
#include <filesystem>
#include <limits>

FR_NAMESPACE_BEGIN

/**
 * Utility class for reading memory-mapped files.
 */
class MemoryMappedFile {
public:
    enum class AccessHint {
        Normal,          ///< Good overall performance.
        SequentialScan,  ///< Read file once with few seeks.
        RandomAccess     ///< Good for random access.
    };

    static constexpr size_t WHOLE_FILE = std::numeric_limits<size_t>::max();

    /**
     * Default constructor. Use open() for opening a file.
     */
    MemoryMappedFile() = default;

    /**
     * Constructor opening a file. Use is_open() to check if successful.
     * @param path Path to open.
     * @param mapped_size Number of bytes to map into memory (automatically clamped to the file size).
     * @param access_hint Hint on how memory is accessed.
     */
    MemoryMappedFile(const std::filesystem::path &path, size_t mapped_size = WHOLE_FILE,
                     AccessHint access_hint = AccessHint::Normal);

    /// Destructor. Closes the file.
    ~MemoryMappedFile();

    /**
     * Open a file.
     * @param path Path to open.
     * @param mapped_size Number of bytes to map into memory (automatically clamped to the file size).
     * @param access_hint Hint on how memory is accessed.
     * @return True if file was successfully opened.
     */
    bool open(const std::filesystem::path &path, size_t mapped_size = WHOLE_FILE,
              AccessHint access_hint = AccessHint::Normal);

    /// Close the file.
    void close();

    /// True, if file successfully opened.
    bool is_open() const { return _mapped_data != nullptr; }

    /// Get the file size in bytes.
    size_t size() const { return _size; }

    /// Get the mapped data.
    const void *data() const { return _mapped_data; };

    /// Get the mapped memory size in bytes.
    size_t mapped_size() const { return _mapped_size; };

    /// Get the OS page size (for remap).
    static size_t page_size();

private:
    MemoryMappedFile(const MemoryMappedFile &) = delete;
    MemoryMappedFile(MemoryMappedFile &) = delete;
    MemoryMappedFile &operator=(const MemoryMappedFile &) = delete;
    MemoryMappedFile &operator=(const MemoryMappedFile &&) = delete;

    /**
     * Replace mapping by a new one of the same file.
     * @param offset Offset from start of the file in bytes (must be multiple of page size).
     * @param mapped_size Size of mapping in bytes.
     * @return True if successful.
     */
    bool remap(uint64_t offset, size_t mapped_size);

    std::filesystem::path _path;
    AccessHint _access_hint = AccessHint::Normal;
    size_t _size = 0;

#if FR_WINDOWS
    using FileHandle = void *;
    void *_mapped_file{nullptr};
#elif FR_LINUX || FR_MACOS
    using FileHandle = int;
#else
#error "Unknown OS"
#endif

    FileHandle _file = 0;
    void *_mapped_data = 0;
    size_t _mapped_size = 0;
};

FR_NAMESPACE_END
