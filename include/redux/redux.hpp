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
    using StateListener = std::function<void(State)>;
    using Next = std::function<void(Action)>;
    using Middleware = std::function<void(Store<State, Action> *, Next, Action)>;

    Store()
    {
    }

    Store(Reducer reducer, State initial_state = State())
        : action_stream(action_bus.get_observable()), state_bus(initial_state),
          state_stream(state_bus.get_observable().publish().ref_count()),
          next([s = action_bus.get_subscriber()](Action action) { s.on_next(action); })
    {
        action_stream.observe_on(rxcpp::observe_on_event_loop())
            .scan(initial_state, reducer)
            .subscribe(state_bus.get_subscriber());
    }

    virtual ~Store()
    {
        close();
    }

    auto get_state() const
    {
        flush();
        return state_bus.get_value();
    }

    void dispatch(Action action)
    {
        next(action);
    }

    auto subscribe(StateListener listener)
    {
        auto subscription = state_stream.subscribe(listener);

        return [=]() {
            flush();
            subscription.unsubscribe();
        };
    }

    void apply_middleware(Middleware middleware)
    {
        next = [middleware, this, next = next](auto action) { middleware(this, next, action); };
    }

    auto get_state_stream() const
    {
        return state_stream;
    }

    void close()
    {
        action_bus.get_subscriber().on_completed();
        state_stream.ignore_elements().as_blocking().subscribe();
    }

private:
    void flush() const
    {
        rxcpp::observable<>::from(state_stream.map([](auto _) { return 0; }).as_dynamic(), rxcpp::observable<>::from(0))
            .amb(rxcpp::observe_on_event_loop())
            .as_blocking()
            .subscribe();
    }

private:
    rxcpp::subjects::subject<Action> action_bus;
    rxcpp::observable<Action> action_stream;
    rxcpp::subjects::behavior<State> state_bus;
    rxcpp::observable<State> state_stream;
    Next next;
};

template <typename Reducer, typename State = typename detail::function_traits<Reducer>::template arg<0>::type,
          typename Action = typename detail::function_traits<Reducer>::template arg<1>::type>
auto create_store(Reducer reducer, State initial_state = State())
{
    return Store<State, Action>(reducer, initial_state);
}
} // namespace redux
