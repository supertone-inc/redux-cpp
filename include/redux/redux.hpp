#pragma once

#include <functional>

namespace redux
{
namespace detail
{
template <typename T> struct function_traits : public function_traits<decltype(&T::operator())>
{
};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType (ClassType::*)(Args...) const>
{
    typedef ReturnType result_type;
    enum
    {
        arity = sizeof...(Args)
    };
    template <size_t i> struct arg
    {
        typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
    };
};
} // namespace detail

template <typename State, typename Action, typename Reducer = std::function<State(State, Action)>> class Store
{
public:
    Store()
    {
    }

    Store(Reducer reducer, State initial_state = State())
    {
    }
};

template <typename Reducer, typename State = typename detail::function_traits<Reducer>::template arg<0>::type,
          typename Action = typename detail::function_traits<Reducer>::template arg<1>::type>
auto create_store(Reducer reducer, State initial_state = State())
{
    return Store<State, Action>(reducer, initial_state);
}
} // namespace redux
