#include "mock.hpp"

#include <doctest.h>
#include <doctest/trompeloeil.hpp>
#include <redux/redux.hpp>

namespace basic
{
using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;
using trompeloeil::_;

using State = int;
using Action = std::string;
using Mock = BaseMock<State, Action>;

Mock mock;
trompeloeil::sequence seq;

std::function<State(State, Action)> reducer = bind(&Mock::reducer, &mock, _1, _2);

TEST_CASE("store calls reducer on action dispatch")
{
    auto store = redux::create_store(reducer);

    REQUIRE_CALL(mock, reducer(_, "action")).RETURN(_1);
    store.dispatch("action");

    store.close();
}

TEST_CASE("store publishes state on action dispatch")
{
    ALLOW_CALL(mock, reducer(_, _)).RETURN(_1);

    auto store = redux::create_store(reducer);

    REQUIRE_CALL(mock, state_listener(_)).IN_SEQUENCE(seq);
    store.subscribe(bind(&Mock::state_listener, &mock, _1));

    REQUIRE_CALL(mock, state_listener(_)).IN_SEQUENCE(seq);
    store.dispatch("action");

    store.close();
}

TEST_CASE("state listeners can unsubscribe")
{
    ALLOW_CALL(mock, reducer(_, _)).RETURN(_1);

    auto store = redux::create_store(reducer);

    REQUIRE_CALL(mock, state_listener(_)).IN_SEQUENCE(seq);
    auto unsubscribe = store.subscribe(bind(&Mock::state_listener, &mock, _1));

    REQUIRE_CALL(mock, state_listener(_)).IN_SEQUENCE(seq);
    store.dispatch("action");

    unsubscribe();

    FORBID_CALL(mock, state_listener(_));
    store.dispatch("action");

    store.close();
}

TEST_CASE("store updates state")
{
    ALLOW_CALL(mock, reducer(_, "increase")).RETURN(_1 + 1);
    ALLOW_CALL(mock, reducer(_, "decrease")).RETURN(_1 - 1);

    auto store = redux::create_store(reducer);

    REQUIRE_CALL(mock, state_listener(0)).IN_SEQUENCE(seq);
    store.subscribe(bind(&Mock::state_listener, &mock, _1));
    REQUIRE(store.get_state() == 0);

    REQUIRE_CALL(mock, state_listener(1)).IN_SEQUENCE(seq);
    store.dispatch("increase");
    REQUIRE(store.get_state() == 1);

    REQUIRE_CALL(mock, state_listener(2)).IN_SEQUENCE(seq);
    store.dispatch("increase");
    REQUIRE(store.get_state() == 2);

    REQUIRE_CALL(mock, state_listener(1)).IN_SEQUENCE(seq);
    store.dispatch("decrease");
    REQUIRE(store.get_state() == 1);

    REQUIRE_CALL(mock, state_listener(0)).IN_SEQUENCE(seq);
    store.dispatch("decrease");
    REQUIRE(store.get_state() == 0);

    store.close();
}

TEST_CASE("store does not mutate state")
{
    using State = std::vector<int>;

    BaseMock<State, Action> mock;

    std::vector<int> initial_state{0};

    auto reducer = [&](State state, Action action) {
        mock.reducer(state, action);

        REQUIRE(&state != &initial_state);
        REQUIRE(state == initial_state);
        state.push_back(0);
        REQUIRE(state != initial_state);

        return state;
    };

    auto store = redux::create_store(reducer, initial_state);

    REQUIRE_CALL(mock, reducer(_, _)).RETURN(_1);
    store.dispatch("update");

    store.close();
}

TEST_CASE("store calls reducer on single thread")
{
    const auto DISPATCH_COUNT = 100;

    REQUIRE_CALL(mock, reducer(_, _)).RETURN(0).TIMES(DISPATCH_COUNT);

    std::thread::id non_thread_id;
    std::thread::id last_thread_id = non_thread_id;

    auto reducer = [&](State state, Action action) {
        mock.reducer(state, action);

        auto this_thread_id = std::this_thread::get_id();

        if (last_thread_id == non_thread_id)
        {
            last_thread_id = this_thread_id;
        }

        REQUIRE(this_thread_id == last_thread_id);

        return state;
    };
    auto store = redux::create_store(reducer);

    std::vector<std::thread> threads;

    for (int i = 0; i < DISPATCH_COUNT; i++)
    {
        threads.push_back(std::thread([&]() { store.dispatch(std::to_string(i)); }));
    }

    for (auto &thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    store.close();
}
} // namespace basic
