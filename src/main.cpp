#include <fmt/format.h>
#include <spdlog/spdlog.h>

int main() {
    spdlog::info("just a test {}", 123);

    fmt::print("{} {}\n", "hello", "world");
    return 0;
}
