#pragma once

#include <rxcpp/rx.hpp>

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
        : state(initial_state), action_stream(action_bus.get_observable()),
          state_stream(action_stream.scan(initial_state, reducer).publish().ref_count())
    {
        state_stream.subscribe([&](State state) { this->state = state; });
    }

    auto getState() const
    {
        return state;
    }

    void dispatch(Action action)
    {
        action_bus.get_subscriber().on_next(action);
    }

    void subscribe(std::function<void(State)> listener)
    {
        listener(getState());
        state_stream.subscribe(listener);
    }

    auto get_action_stream() const
    {
        return action_stream;
    }

    auto get_state_stream() const
    {
        return state_stream;
    }

private:
    State state;
    rxcpp::subjects::subject<Action> action_bus;
    rxcpp::observable<Action> action_stream;
    rxcpp::observable<State> state_stream;
};

template <typename Reducer, typename State = typename detail::function_traits<Reducer>::template arg<0>::type,
          typename Action = typename detail::function_traits<Reducer>::template arg<1>::type>
auto create_store(Reducer reducer, State initial_state = State())
{
    return Store<State, Action>(reducer, initial_state);
}
} // namespace redux
