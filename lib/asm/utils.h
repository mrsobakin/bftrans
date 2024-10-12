#pragma once

#include <expected>
#include <tuple>
#include <variant>


template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <typename V, typename T>
bool variant_is(const V& variant, const T& value) {
    const T* control = std::get_if<T>(&variant);
    if (!control) return false;
    return *control == value;
}


template <typename T1, typename T2>
using tuple_types_cat_t = decltype(std::tuple_cat(std::declval<T1>(), std::declval<T2>()));

template <typename Element, typename Tuple>
using tuple_prepend_t = decltype(std::tuple_cat(std::declval<std::tuple<Element>>(), std::declval<Tuple>()));


template <typename NotExpected>
struct is_expected {
    static constexpr bool value = false;
};

template <typename Value, typename Error>
struct is_expected<std::expected<Value, Error>> {
    static constexpr bool value = true;
};

template <typename MaybeExpected>
inline constexpr bool is_expected_v = is_expected<MaybeExpected>::value;


template <typename MaybeCallable>
struct is_invocable_or_member {
    static constexpr bool value = std::is_invocable_v<MaybeCallable>;
};

template <typename Return, typename Class>
struct is_invocable_or_member<Return (Class::*)()> {
    static constexpr bool value = true;
};

template <typename MaybeInvokable>
inline constexpr bool is_invocable_or_member_v = is_invocable_or_member<MaybeInvokable>::value;


template <typename Callable>
struct invoke_maybe_member_result {
    using type = std::invoke_result_t<Callable>;
};

template <typename Return, typename Class>
struct invoke_maybe_member_result<Return (Class::*)()> {
    using type = Return;
};

template <typename Callable>
using invoke_maybe_member_result_t = invoke_maybe_member_result<Callable>::type;


template <bool Concat, typename Curr, typename Other>
struct _prepend_return {};

template <typename Curr, typename Other>
struct _prepend_return<true, Curr, Other> {
    using type = tuple_types_cat_t<
        std::tuple<invoke_maybe_member_result_t<Curr>>,
        Other
    >;
};

template <typename Curr, typename Other>
struct _prepend_return<false, Curr, Other> {
    using type = Other;
};

template <typename ...Args>
struct extract_callables {};

template <typename First, typename ...Other>
struct extract_callables<First, Other...> {
    using type = _prepend_return<
        is_invocable_or_member_v<First>,
        First,
        typename extract_callables<Other...>::type
    >::type;
};

template <>
struct extract_callables<> {
    using type = std::tuple<>;
};

template <typename ...Args>
using extract_callables_t = extract_callables<Args...>::type;


template <typename BaseError, typename Tuple>
struct remove_expected {};

template <typename BaseError, typename Tuple>
using remove_expected_t = remove_expected<BaseError, Tuple>::type;

template <typename BaseError, typename Inner, typename ThisError, typename ...Other>
struct remove_expected<BaseError, std::tuple<std::expected<Inner, ThisError>, Other...>> {
    using type = tuple_prepend_t<Inner, remove_expected_t<BaseError, std::tuple<Other...>>>;
    static_assert(std::is_same_v<BaseError, ThisError>, "Tuple error types diverge");
};

template <typename BaseError, typename Value, typename ...Other>
struct remove_expected<BaseError, std::tuple<Value, Other...>> {
    using type = tuple_prepend_t<Value, remove_expected_t<BaseError, std::tuple<Other...>>>;
};

template <typename BaseError>
struct remove_expected<BaseError, std::tuple<>> {
    using type = std::tuple<>;
};
