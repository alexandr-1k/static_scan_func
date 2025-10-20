#pragma once

#include "format_string.hpp"
#include "parse.hpp"
#include "types.hpp"

namespace stdx {

template <details::format_string fmt, details::fixed_string source, typename... Ts, std::size_t... Is>
consteval details::scan_result<Ts...> scan_impl(std::index_sequence<Is...>) {
    return details::scan_result<Ts...>{std::tuple<Ts...>(details::parse_input<Is, fmt, source, Ts>()...)};
}

// Главная функция
template <details::format_string fmt, details::fixed_string source, typename... Ts>
consteval details::scan_result<Ts...> scan() {
    static_assert(fmt.number_placeholders == sizeof...(Ts), "Number of placeholders and types do not match");
    constexpr std::size_t N = sizeof...(Ts);
    return scan_impl<fmt, source, Ts...>(std::make_index_sequence<N>{});
}

}  // namespace stdx