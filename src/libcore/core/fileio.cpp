#include "fileio.h"

#include <cstdio>
#include <stdexcept>

#if FR_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif FR_LINUX
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#elif FR_MACOS
#define _DARWIN_USE_64_BIT_INODE
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#error "Unknown OS"
#endif

FR_NAMESPACE_BEGIN

MemoryMappedFile::MemoryMappedFile(const std::filesystem::path &path, size_t mapped_size, AccessHint access_hint) {
    open(path, mapped_size, access_hint);
}

MemoryMappedFile::~MemoryMappedFile() { close(); }

bool MemoryMappedFile::open(const std::filesystem::path &path, size_t mapped_size, AccessHint access_hint) {
    if (is_open())
        return false;

    _path = path;
    _access_hint = access_hint;

#if FR_WINDOWS
    // Handle access hint.
    DWORD flags = 0;
    switch (_access_hint) {
        case AccessHint::Normal:
            flags = FILE_ATTRIBUTE_NORMAL;
            break;
        case AccessHint::SequentialScan:
            flags = FILE_FLAG_SEQUENTIAL_SCAN;
            break;
        case AccessHint::RandomAccess:
            flags = FILE_FLAG_RANDOM_ACCESS;
            break;
        default:
            break;
    }

    // Open file.
    _file = ::CreateFile(_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, flags, NULL);
    if (!_file)
        return false;

    // Get file size.
    LARGE_INTEGER size;
    if (!::GetFileSizeEx(_file, &size)) {
        close();
        return false;
    }
    _size = static_cast<size_t>(size.QuadPart);

    // Create file mapping.
    _mapped_file = ::CreateFileMapping(_file, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!_mapped_file) {
        close();
        return false;
    }

#elif FR_LINUX

    // Open file.
    _file = ::open(path.c_str(), O_RDONLY | O_LARGEFILE);
    if (_file == -1) {
        _file = 0;
        return false;
    }

    // Get file size.
    struct stat64 stat_info;
    if (fstat64(_file, &stat_info) < 0) {
        close();
        return false;
    }
    _size = stat_info.st_size;

#elif FR_MACOS

    // Open file.
    _file = ::open(path.c_str(), O_RDONLY);
    if (_file == -1) {
        _file = 0;
        return false;
    }

    // Get file size.
    struct stat stat_info;
    if (fstat(_file, &stat_info) < 0) {
        close();
        return false;
    }
    _size = stat_info.st_size;

#endif

    // Initial mapping.
    if (!remap(0, mapped_size)) {
        close();
        return false;
    }

    return true;
}

void MemoryMappedFile::close() {
    // Unmap memory.
    if (_mapped_data) {
#if FR_WINDOWS
        ::UnmapViewOfFile(_mapped_data);
#elif FR_LINUX || FR_MACOS
        ::munmap(_mapped_data, _size);
#endif
        _mapped_data = nullptr;
    }

#if FR_WINDOWS
    if (_mapped_file) {
        ::CloseHandle(_mapped_file);
        _mapped_file = nullptr;
    }
#endif

    // Close file.
    if (_file) {
#if FR_WINDOWS
        ::CloseHandle(_file);
#elif FR_LINUX || FR_MACOS
        ::close(_file);
#endif
        _file = 0;
    }

    _size = 0;
}

size_t MemoryMappedFile::page_size() {
#if FR_WINDOWS
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwAllocationGranularity;
#elif FR_LINUX || FR_MACOS
    return sysconf(_SC_PAGESIZE);
#endif
}

bool MemoryMappedFile::remap(uint64_t offset, size_t mapped_size) {
    if (!_file)
        return false;
    if (offset >= _size)
        return false;

    // Close previous mapping.
    if (_mapped_data) {
#if FR_WINDOWS
        ::UnmapViewOfFile(_mapped_data);
#elif FR_LINUX || FR_MACOS
        ::munmap(_mapped_data, _mapped_size);
#endif
        _mapped_data = nullptr;
        _mapped_size = 0;
    }

    // Clamp mapped range.
    if (offset + mapped_size > _size)
        mapped_size = size_t(_size - offset);

#if FR_WINDOWS
    DWORD offsetLow = DWORD(offset & 0xFFFFFFFF);
    DWORD offsetHigh = DWORD(offset >> 32);

    // Create new mapping.
    _mapped_data = ::MapViewOfFile(_mapped_file, FILE_MAP_READ, offsetHigh, offsetLow, mapped_size);
    if (!_mapped_data)
        _mapped_size = 0;
    _mapped_size = mapped_size;
#elif FR_LINUX || FR_MACOS
    // Create new mapping.
#if FR_LINUX
    _mapped_data = ::mmap64(NULL, mapped_size, PROT_READ, MAP_SHARED, _file, offset);
#elif FR_MACOS
    _mapped_data = ::mmap(NULL, mapped_size, PROT_READ, MAP_SHARED, _file, offset);
#endif
    if (_mapped_data == MAP_FAILED) {
        _mapped_data = nullptr;
        return false;
    }
    _mapped_size = mapped_size;

    // Handle access hint.
    int advice = 0;
    switch (_access_hint) {
        case AccessHint::Normal:
            advice = MADV_NORMAL;
            break;
        case AccessHint::SequentialScan:
            advice = MADV_SEQUENTIAL;
            break;
        case AccessHint::RandomAccess:
            advice = MADV_RANDOM;
            break;
        default:
            break;
    }
    ::madvise(_mapped_data, _mapped_size, advice);

    return true;
#endif
}

FR_NAMESPACE_END
