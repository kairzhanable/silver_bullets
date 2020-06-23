#pragma once

#include "function_traits.hpp"
#include "indexed_tuple.hpp"

namespace silver_bullets {

namespace detail {

template<class T> struct FuncArgConvertCaller;

template<class ... Ai>
struct FuncArgConvertCaller<std::tuple<Ai...>> {
    template<class F, class InputRange, class ArgConverter>
    static auto convertAndCall(F f, InputRange in, const ArgConverter& argConverter) {
        return f(argConverter.template operator()<typename Ai::type>(in[Ai::index])...);
    }
};

} // namespace detail


template<
        class OutputType,
        class InputRange,
        class F,
        class ReturnValConverter,
        class ArgConverter>
inline auto func_arg_convert(
        F f,
        const ReturnValConverter& returnValConverter,
        const ArgConverter& argConverter)
{
    return [f, argConverter, returnValConverter](OutputType out, InputRange in) {
        using FT = function_traits<F>;
        using args_type = typename FT::args_type;
        using iargs_type = indexed_tuple_t<args_type>;
        returnValConverter(out, detail::FuncArgConvertCaller<iargs_type>::convertAndCall(f, in, argConverter));
    };
}

} // namespace silver_bullets

