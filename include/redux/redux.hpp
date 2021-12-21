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
        : state(initial_state), shared_mutex(std::make_shared<std::mutex>()),
          action_stream(action_bus.get_observable().observe_on(rxcpp::observe_on_event_loop()).publish().ref_count()),
          state_stream(action_stream.scan(initial_state, reducer).publish().ref_count())
    {
        state_stream.subscribe([&](State state) {
            std::lock_guard<std::mutex> lock(*shared_mutex);
            this->state = state;
        });
    }

    virtual ~Store()
    {
        close();
    }

    auto get_state() const
    {
        std::lock_guard<std::mutex> lock(*shared_mutex);
        return state;
    }

    void dispatch(Action action)
    {
        action_bus.get_subscriber().on_next(action);
    }

    auto subscribe(std::function<void(State)> listener)
    {
        listener(get_state());

        auto subscription = state_stream.subscribe(listener);

        return [=]() {
            flush();
            subscription.unsubscribe();
        };
    }

    auto get_action_stream() const
    {
        return action_stream;
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
        rxcpp::observable<>::empty<int>(rxcpp::observe_on_event_loop()).as_blocking().subscribe();
    }

private:
    State state;
    std::shared_ptr<std::mutex> shared_mutex;
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
