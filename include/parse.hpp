#pragma once

#include "format_string.hpp"
#include "types.hpp"
#include <charconv>
#include <cstdint>
#include <optional>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

namespace stdx::details {
using namespace std::literals;

template <typename T, typename... Ts>
constexpr bool in_types_v = (std::is_same_v<T, Ts> || ...);

template <typename T>
concept AllowedType =
    !std::is_reference_v<T> && in_types_v<std::remove_cv_t<T>, std::int8_t, std::int16_t, std::int32_t, std::int64_t,
                                          std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t, std::string_view>;

// Шаблонная функция, возвращающая пару позиций в строке с исходными данными, соотвествующих I-ому плейсхолдеру
template <int I, format_string fmt, fixed_string source>
consteval auto get_current_source_for_parsing() {
    static_assert(I >= 0 && I < fmt.number_placeholders, "Invalid placeholder index");

    constexpr auto to_sv = [](const auto &fs) { return std::string_view(fs.data, fs.size() - 1); };

    constexpr auto fmt_sv = to_sv(fmt.fmt);
    constexpr auto src_sv = to_sv(source);
    constexpr auto &positions = fmt.placeholder_positions;

    // Получаем границы текущего плейсхолдера в формате
    constexpr auto pos_i = positions[I];
    constexpr size_t fmt_start = pos_i.first, fmt_end = pos_i.second;

    // Находим начало в исходной строке
    constexpr auto src_start = [&] {
        if constexpr (I == 0) {
            return fmt_start;
        } else {
            // Находим конец предыдущего плейсхолдера в исходной строке
            constexpr auto prev_bounds = get_current_source_for_parsing<I - 1, fmt, source>();
            const auto prev_end = prev_bounds.second;

            // Получаем разделитель между текущим и предыдущим плейсхолдерами
            constexpr auto prev_fmt_end = positions[I - 1].second;
            constexpr auto sep = fmt_sv.substr(prev_fmt_end + 1, fmt_start - (prev_fmt_end + 1));

            // Ищем разделитель после предыдущего значения
            auto pos = src_sv.find(sep, prev_end);
            return pos != std::string_view::npos ? pos + sep.size() : src_sv.size();
        }
    }();

    // Находим конец в исходной строке
    constexpr auto src_end = [&] {
        // Получаем разделитель после текущего плейсхолдера
        if constexpr (fmt_end == (fmt_sv.size() - 1)) {
            return src_sv.size();
        }
        constexpr auto sep =
            fmt_sv.substr(fmt_end + 1, (I < fmt.number_placeholders - 1) ? positions[I + 1].first - (fmt_end + 1)
                                                                         : fmt_sv.size() - (fmt_end + 1));
        // Ищем разделитель после текущего значения
        constexpr auto pos = src_sv.find(sep, src_start);
        return pos != std::string_view::npos ? pos : src_sv.size();
    }();
    return std::pair{src_start, src_end};
}

template <typename T>
concept SignedInt = std::is_integral_v<T> && std::is_signed_v<T>;

template <typename T>
concept UnsignedInt = std::is_integral_v<T> && !std::is_signed_v<T>;

template <typename T>
concept AnyDigit = SignedInt<T> || UnsignedInt<T>;

template <typename T>
concept StringView = std::is_same_v<T, std::string_view>;

template <AnyDigit DigitType>
constexpr std::optional<DigitType> to_digit(std::string_view sv) {
    DigitType result{};
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);

    if (ec == std::errc()) {
        return result;
    }
    return std::nullopt;
}

template <SignedInt RequestedType, fixed_string source>
consteval RequestedType parse_value(Placeholder::EMPTY) {
    constexpr auto res = to_digit<RequestedType>(std::string_view{source.data});
    if constexpr (!res.has_value()) {
        // throw_static_assert<"Failed to parse signed integer at empty placeholder from: ", source.data>();
        static_assert(false, "Failed to parse signed integer at empty placeholder");
    }
    return res.value();
}

template <SignedInt RequestedType, fixed_string source>
consteval RequestedType parse_value(Placeholder::STRING) {
    static_assert(false, "Placeholder contains string specifier, but singed integer requested");
}

template <SignedInt RequestedType, fixed_string source>
consteval RequestedType parse_value(Placeholder::INTEGER) {
    constexpr auto res = to_digit<RequestedType>(std::string_view{source.data});
    if constexpr (!res.has_value()) {
        static_assert(false, "Failed to parse signed integer at `d` placeholder");
    }
    return res.value();
}

template <SignedInt RequestedType, fixed_string source>
consteval RequestedType parse_value(Placeholder::UNSIGNED) {
    constexpr auto res = to_digit<RequestedType>(std::string_view{source.data});
    if constexpr (!res.has_value()) {
        static_assert(false, "Failed to parse signed integer at `u` placeholder");
    }
    return res.value();
}

template <UnsignedInt RequestedType, fixed_string source>
consteval RequestedType parse_value(Placeholder::EMPTY) {
    constexpr auto res = to_digit<RequestedType>(std::string_view{source.data});
    if constexpr (!res.has_value()) {
        static_assert(false, "Failed to parse unsigned integer at empty placeholder");
    }
    return res.value();
}

template <UnsignedInt RequestedType, fixed_string source>
consteval RequestedType parse_value(Placeholder::STRING) {
    static_assert(false, "Placeholder contains string specifier, but unsigned integer requested");
}

template <UnsignedInt RequestedType, fixed_string source>
consteval RequestedType parse_value(Placeholder::INTEGER) {
    constexpr auto res = to_digit<RequestedType>(std::string_view{source.data});
    if constexpr (!res.has_value()) {
        static_assert(false, "Failed to parse unsigned integer at `d` placeholder");
    }
    return res.value();
}

template <UnsignedInt RequestedType, fixed_string source>
consteval RequestedType parse_value(Placeholder::UNSIGNED) {
    constexpr auto res = to_digit<RequestedType>(std::string_view{source.data});
    if constexpr (!res.has_value()) {
        static_assert(false, "Failed to parse unsigned integer at `u` placeholder");
    }
    return res.value();
}

template <StringView RequestedType, fixed_string source>
consteval RequestedType parse_value(Placeholder::EMPTY) {
    return source.data;
}

template <StringView RequestedType, fixed_string source>
consteval RequestedType parse_value(Placeholder::STRING) {
    return source.data;
}

template <StringView RequestedType, fixed_string source>
consteval RequestedType parse_value(Placeholder::INTEGER) {
    static_assert(false, "Placeholder contains integer specifier, but string_view requested");
}

template <StringView RequestedType, fixed_string source>
consteval RequestedType parse_value(Placeholder::UNSIGNED) {
    static_assert(false, "Placeholder contains unsigned specifier, but string_view requested");
}

// Шаблонная функция, выполняющая преобразования исходных данных в конкретный тип на основе I-го плейсхолдера
template <int Idx, format_string fmt, fixed_string source, AllowedType OutType>
consteval OutType parse_input() {
    constexpr auto source_pos = get_current_source_for_parsing<Idx, fmt, source>();
    constexpr auto sv = std::string_view{source.data};
    constexpr auto substr = sv.substr(source_pos.first, source_pos.second - source_pos.first);

    constexpr auto beg = substr.data();
    constexpr auto end = substr.data() + substr.size();

    constexpr fixed_string<substr.size() + 1> substr_fs{beg, end};

    if constexpr (fmt.template IsSignedIntPlaceholder<Idx>()) {
        return parse_value<OutType, substr_fs>(Placeholder::INTEGER{});

    } else if constexpr (fmt.template IsUnsignedIntPlaceholder<Idx>()) {
        return parse_value<OutType, substr_fs>(Placeholder::UNSIGNED{});

    } else if constexpr (fmt.template IsStringPlaceholder<Idx>()) {
        return parse_value<OutType, substr_fs>(Placeholder::STRING{});

    } else if constexpr (fmt.template IsEmptyPlaceholder<Idx>()) {
        return parse_value<OutType, substr_fs>(Placeholder::EMPTY{});
    } else {
        std::unreachable();
    }
}

}  // namespace stdx::details
