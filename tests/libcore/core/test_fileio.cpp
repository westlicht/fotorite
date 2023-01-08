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
        CHECK_EQ(file.is_open(), false);
        CHECK_EQ(file.size(), 0);
        CHECK_EQ(file.data(), nullptr);
        CHECK_EQ(file.mapped_size(), 0);

        // Allowed to close a closed file.
        file.close();
        CHECK_EQ(file.is_open(), false);
    }

    SUBCASE("non existing") {
        {
            MemoryMappedFile file;
            CHECK_EQ(file.open("__file_that_does_not_exist__"), false);
            CHECK_EQ(file.is_open(), false);
        }
        {
            MemoryMappedFile file("__file_that_does_not_exist__");
            CHECK_EQ(file.is_open(), false);
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
            CHECK_EQ(file.is_open(), true);
            CHECK_EQ(file.size(), randomData.size());
            CHECK_NE(file.data(), nullptr);
            CHECK_GE(file.mapped_size(), randomData.size());
            CHECK(std::memcmp(file.data(), randomData.data(), file.size()) == 0);
        }

        {
            // Map first 1024 bytes.
            MemoryMappedFile file(tempPath, 1024);
            CHECK_EQ(file.is_open(), true);
            CHECK_EQ(file.size(), randomData.size());
            CHECK_NE(file.data(), nullptr);
            CHECK_GE(file.mapped_size(), 1024);
            CHECK(std::memcmp(file.data(), randomData.data(), 1024) == 0);
        }

        {
            // Map first page.
            size_t pageSize = MemoryMappedFile::page_size();
            CHECK_GE(pageSize, 4096);
            REQUIRE_LE(pageSize, randomData.size());
            MemoryMappedFile file(tempPath, pageSize);
            CHECK_EQ(file.is_open(), true);
            CHECK_EQ(file.size(), randomData.size());
            CHECK_NE(file.data(), nullptr);
            CHECK_GE(file.mapped_size(), pageSize);
            CHECK(std::memcmp(file.data(), randomData.data(), pageSize) == 0);
        }

        // Cleanup.
        std::filesystem::remove(tempPath);
    }
}

TEST_SUITE_END();
