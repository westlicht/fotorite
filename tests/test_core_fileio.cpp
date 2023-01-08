#include "core/fileio.h"

#include <doctest/doctest.h>

#include <cstring>
#include <fstream>
#include <random>
#include <vector>

using namespace fr;

TEST_SUITE_BEGIN("fileio");

TEST_CASE("MemoryMappedFile") {
    SUBCASE("closed") {
        MemoryMappedFile file;
        CHECK_EQ(file.isOpen(), false);
        CHECK_EQ(file.getSize(), 0);
        CHECK_EQ(file.getData(), nullptr);
        CHECK_EQ(file.getMappedSize(), 0);

        // Allowed to close a closed file.
        file.close();
        CHECK_EQ(file.isOpen(), false);
    }

    SUBCASE("non existing") {
        {
            MemoryMappedFile file;
            CHECK_EQ(file.open("__file_that_does_not_exist__"), false);
            CHECK_EQ(file.isOpen(), false);
        }
        {
            MemoryMappedFile file("__file_that_does_not_exist__");
            CHECK_EQ(file.isOpen(), false);
        }
    }

    SUBCASE("read") {
        std::vector<uint8_t> randomData(128 * 1024);
        std::mt19937 rng;
        for (size_t i = 0; i < randomData.size(); ++i)
            randomData[i] = rng() & 0xff;

        const std::filesystem::path tempPath = std::filesystem::absolute("test_memory_mapped.bin");

        // Write file with random data.
        std::ofstream ofs(tempPath, std::ios::binary);
        REQUIRE(ofs.good());
        ofs.write(reinterpret_cast<const char *>(randomData.data()), randomData.size());
        ofs.close();

        {
            // Map entire file.
            MemoryMappedFile file(tempPath);
            CHECK_EQ(file.isOpen(), true);
            CHECK_EQ(file.getSize(), randomData.size());
            CHECK_NE(file.getData(), nullptr);
            CHECK_GE(file.getMappedSize(), randomData.size());
            CHECK(std::memcmp(file.getData(), randomData.data(), file.getSize()) == 0);
        }

        {
            // Map first 1024 bytes.
            MemoryMappedFile file(tempPath, 1024);
            CHECK_EQ(file.isOpen(), true);
            CHECK_EQ(file.getSize(), randomData.size());
            CHECK_NE(file.getData(), nullptr);
            CHECK_GE(file.getMappedSize(), 1024);
            CHECK(std::memcmp(file.getData(), randomData.data(), 1024) == 0);
        }

        {
            // Map first page.
            size_t pageSize = MemoryMappedFile::getPageSize();
            CHECK_GE(pageSize, 4096);
            REQUIRE_LE(pageSize, randomData.size());
            MemoryMappedFile file(tempPath, pageSize);
            CHECK_EQ(file.isOpen(), true);
            CHECK_EQ(file.getSize(), randomData.size());
            CHECK_NE(file.getData(), nullptr);
            CHECK_GE(file.getMappedSize(), pageSize);
            CHECK(std::memcmp(file.getData(), randomData.data(), pageSize) == 0);
        }

        // Cleanup.
        std::filesystem::remove(tempPath);
    }
}

TEST_SUITE_END();
