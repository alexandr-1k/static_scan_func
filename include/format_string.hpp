#pragma once

#include "types.hpp"
#include <array>
#include <cstddef>
#include <expected>
#include <utility>
#include <variant>

namespace stdx::details {
struct Placeholder {
    struct EMPTY {};
    struct INTEGER {};
    struct UNSIGNED {};
    struct STRING {};
    using Type = std::variant<EMPTY, INTEGER, UNSIGNED, STRING>;
};
// Шаблонный класс для хранения форматирующей строчки и ее особенностей
template <fixed_string str>
class format_string {
public:
    // Функция для получения количества плейсхолдеров и проверки корректности формирующей строки
    static consteval std::expected<size_t, parse_error> get_number_placeholders() {
        constexpr size_t N = str.size();
        if (!N)
            return 0;
        size_t placeholder_count = 0;
        size_t pos = 0;
        const size_t size = N - 1;  // -1 для игнорирования нуль-терминатора

        while (pos < size) {
            // Пропускаем все символы до '{'
            if (str.data[pos] != '{') {
                ++pos;
                continue;
            }

            // Проверяем незакрытый плейсхолдер
            if (pos + 1 >= size) {
                return std::unexpected(parse_error{"Unclosed last placeholder"});
            }

            // Начало плейсхолдера
            ++placeholder_count;
            ++pos;

            // Проверка спецификатора формата
            if (str.data[pos] == '%') {
                ++pos;
                if (pos >= size) {
                    return std::unexpected(parse_error{"Unclosed last placeholder"});
                }

                // Проверяем допустимые спецификаторы
                const char spec = str.data[pos];
                constexpr char valid_specs[] = {'d', 'u', 'f', 's'};
                bool valid = false;

                for (const char s : valid_specs) {
                    if (spec == s) {
                        valid = true;
                        break;
                    }
                }

                if (!valid) {
                    return std::unexpected(parse_error{"Invalid specifier."});
                }
                ++pos;
            }

            // Проверяем закрывающую скобку
            if (pos >= size || str.data[pos] != '}') {
                return std::unexpected(parse_error{"\'}\' hasn't been found in appropriate place"});
            }
            ++pos;
        }

        return placeholder_count;
    }

    static constexpr auto fmt = str;
    static constexpr auto parsed_num_of_plhold = get_number_placeholders();

    static_assert(parsed_num_of_plhold.has_value(), "Could not parse format string");
    static constexpr size_t number_placeholders = parsed_num_of_plhold.value();

    using PosArray = std::array<std::pair<std::size_t, std::size_t>, parsed_num_of_plhold.value()>;

    // Функция для получения позиций плейсхолдеров
    static consteval PosArray get_placeholder_positions() {
        size_t pos = 0;
        constexpr size_t N = str.size();
        const size_t size = N - 1;

        PosArray arr{};

        size_t idx = 0;
        while (pos < size) {
            if (str.data[pos] != '{') {
                ++pos;
                continue;
            }

            auto start_pos = pos++;

            if (str.data[pos] == '%') {
                pos += 2;  // specificators were already validated
            }

            auto end_pos = pos++;
            arr[idx] = std::pair(start_pos, end_pos);  // [s, e]
            ++idx;
        }
        return arr;
    }

    static constexpr auto placeholder_positions = get_placeholder_positions();

    template <size_t Idx>
    static consteval bool IsEmptyPlaceholder() {
        static_assert(Idx <= placeholder_positions.size(), "Placeholder index is out of range");
        constexpr auto pos = placeholder_positions[Idx];
        return pos.second - pos.first == 1;
    }
    template <size_t Idx>
    static consteval bool IsSignedIntPlaceholder() {
        static_assert(Idx <= placeholder_positions.size(), "Placeholder index is out of range");
        constexpr auto pos = placeholder_positions[Idx];
        constexpr char specifier = fmt.data[pos.second - 1];
        return specifier == 'd';
    }
    template <size_t Idx>
    static consteval bool IsUnsignedIntPlaceholder() {
        static_assert(Idx <= placeholder_positions.size(), "Placeholder index is out of range");
        constexpr auto pos = placeholder_positions[Idx];
        constexpr char specifier = fmt.data[pos.second - 1];
        return specifier == 'u';
    }
    template <size_t Idx>
    static consteval bool IsStringPlaceholder() {
        static_assert(Idx <= placeholder_positions.size(), "Placeholder index is out of range");
        constexpr auto pos = placeholder_positions[Idx];
        constexpr char specifier = fmt.data[pos.second - 1];
        return specifier == 's';
    }
};

// Пользовательский литерал

template <fixed_string str>
constexpr auto operator""_fs() {
    return str;
}

}  // namespace stdx::details
