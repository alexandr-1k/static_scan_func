#include "scan.hpp"
#include "types.hpp"
#include <cstdint>

void TestSignedTypes() {
    constexpr auto fmt1 = stdx::details::format_string<stdx::details::fixed_string{"Value: {%d} {%d} {%d}"}>{};
    constexpr auto src1 = stdx::details::fixed_string{"Value: -3 1500 12"};
    constexpr auto result1 = stdx::scan<fmt1, src1, int8_t, int16_t, int64_t>();
    static_assert(std::get<int8_t>(result1.data) == -3);
    static_assert(std::get<int16_t>(result1.data) == 1500);
    static_assert(std::get<int64_t>(result1.data) == 12);
}

void TestUnsignedTypes() {
    constexpr auto fmt2 = stdx::details::format_string<stdx::details::fixed_string{"Values: {%u} {%u} {%u}"}>{};
    constexpr auto src2 = stdx::details::fixed_string{"Values: 255 65535 18446744073709551615"};
    constexpr auto result2 = stdx::scan<fmt2, src2, uint8_t, uint16_t, uint64_t>();
    static_assert(std::get<uint8_t>(result2.data) == 255);
    static_assert(std::get<uint16_t>(result2.data) == 65535);
}

void TestStringType() {
    constexpr auto fmt3 = stdx::details::format_string<stdx::details::fixed_string{"Name: {%s} {%d}"}>{};
    constexpr auto src3 = stdx::details::fixed_string{"Name: Alex 42"};
    constexpr auto result3 = stdx::scan<fmt3, src3, std::string_view, int>();
    static_assert(std::get<std::string_view>(result3.data) == "Alex");
    static_assert(std::get<int>(result3.data) == 42);
}

void TestEmtpyPlaceholder() {
    constexpr auto fmt4 = stdx::details::format_string<stdx::details::fixed_string{"Data: {} {}"}>{};
    constexpr auto src4 = stdx::details::fixed_string{"Data: Hello 12345"};
    constexpr auto result4 = stdx::scan<fmt4, src4, std::string_view, int>();
    static_assert(std::get<std::string_view>(result4.data) == "Hello");
    static_assert(std::get<int>(result4.data) == 12345);
}

void TestCombinedCase() {
    constexpr auto fmt5 =
        stdx::details::format_string<stdx::details::fixed_string{"User: {%s}, Age: {%d}, Score: {%u}"}>{};
    constexpr auto src5 = stdx::details::fixed_string{"User: Bob, Age: 30, Score: 1000"};
    constexpr auto result5 = stdx::scan<fmt5, src5, std::string_view, int, unsigned int>();
    static_assert(std::get<std::string_view>(result5.data) == "Bob");
    static_assert(std::get<int>(result5.data) == 30);
    static_assert(std::get<unsigned int>(result5.data) == 1000);
}

int main() { return 0; }
