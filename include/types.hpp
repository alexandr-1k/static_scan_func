#pragma once
#include <algorithm>
#include <assert.h>
#include <cstddef>
#include <tuple>

namespace stdx::details {

// Шаблонный класс, хранящий C-style строку фиксированной длины
const size_t ERR_MSG_SIZE = 64;

template <std::size_t StrSize>
struct fixed_string {
    char data[StrSize]{};
    std::size_t real_size{};

    constexpr fixed_string(const char (&val)[StrSize]) : real_size(StrSize) { std::copy_n(val, StrSize, data); }

    template <std::size_t N>
        requires(N <= StrSize)
    constexpr fixed_string(const char (&val)[N]) : real_size(N) {
        std::copy_n(val, N, data);
    }

    consteval fixed_string(const char *beg, const char *end) { std::copy(beg, end, data); }

    constexpr std::size_t size() const { return real_size; }
};

// Шаблонный класс, хранящий fixed_string достаточной длины для хранения ошибки парсинга
struct parse_error : fixed_string<ERR_MSG_SIZE> {};

// Шаблонный класс для хранения результатов парсинга
template <typename... Ts>
struct scan_result {
    std::tuple<Ts...> data;
};

}  // namespace stdx::details
