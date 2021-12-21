#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <doctest/trompeloeil.hpp>

#include <redux/redux.hpp>

using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using trompeloeil::_;

template <typename State, typename Action> class Mock
{
public:
    MAKE_MOCK1(action_listener, void(Action));
    MAKE_MOCK1(state_listener, void(State));
    MAKE_MOCK2(reducer, State(State, Action));
};

TEST_CASE("store dispatches actions")
{
    using State = int;
    using Action = std::string;
    using Mock = Mock<State, Action>;

    Mock mock;
    auto reducer = [](State state, Action action) { return state; };
    auto store = redux::create_store(reducer);
    store.get_action_stream().subscribe(bind(&Mock::action_listener, &mock, _1));

    REQUIRE_CALL(mock, action_listener("an action"));
    store.dispatch("an action");

    store.close();
}

TEST_CASE("store calls reducer")
{
    using State = int;
    using Action = std::string;
    using Mock = Mock<State, Action>;

    Mock mock;
    auto reducer = [&](State state, Action action) { return mock.reducer(state, action); };
    auto store = redux::create_store(reducer);

    REQUIRE_CALL(mock, reducer(0, "an action")).RETURN(1);
    store.dispatch("an action");

    store.close();
}

TEST_CASE("store publishes states")
{
    using State = int;
    using Action = std::string;
    using Mock = Mock<State, Action>;

    Mock mock;
    trompeloeil::sequence seq;
    auto reducer = [](State state, Action action) { return state + 1; };
    auto store = redux::create_store(reducer);

    REQUIRE_CALL(mock, state_listener(0)).IN_SEQUENCE(seq);
    store.subscribe(bind(&Mock::state_listener, &mock, _1));

    REQUIRE_CALL(mock, state_listener(1)).IN_SEQUENCE(seq);
    store.dispatch("an action");

    store.close();
}

TEST_CASE("state listeners can unsubscribe")
{
    using State = int;
    using Action = std::string;
    using Mock = Mock<State, Action>;

    Mock mock;
    trompeloeil::sequence seq;
    auto reducer = [](State state, Action action) { return state + 1; };
    auto store = redux::create_store(reducer);

    REQUIRE_CALL(mock, state_listener(0)).IN_SEQUENCE(seq);
    auto unsubscribe = store.subscribe(bind(&Mock::state_listener, &mock, _1));

    REQUIRE_CALL(mock, state_listener(1)).IN_SEQUENCE(seq);
    store.dispatch("an action");

    unsubscribe();

    FORBID_CALL(mock, state_listener(_));
    store.dispatch("an action");

    store.close();
}

TEST_CASE("store updates states")
{
    using State = int;
    using Action = std::string;
    using Mock = Mock<State, Action>;

    Mock mock;
    trompeloeil::sequence seq;
    auto reducer = [](State state, Action action) {
        if (action == "increase")
        {
            return state + 1;
        }

        if (action == "decrease")
        {
            return state - 1;
        }

        return state;
    };
    auto store = redux::create_store(reducer);

    REQUIRE_CALL(mock, state_listener(0)).IN_SEQUENCE(seq);
    store.subscribe(bind(&Mock::state_listener, &mock, _1));

    REQUIRE_CALL(mock, state_listener(1)).IN_SEQUENCE(seq);
    store.dispatch("increase");

    REQUIRE_CALL(mock, state_listener(0)).IN_SEQUENCE(seq);
    store.dispatch("decrease");

    store.close();
}

TEST_CASE("store exposes states")
{
    using State = int;
    using Action = std::string;
    using Mock = Mock<State, Action>;

    Mock mock;
    trompeloeil::sequence seq;
    auto reducer = [](State state, Action action) {
        if (action == "increase")
        {
            return state + 1;
        }

        if (action == "decrease")
        {
            return state - 1;
        }

        return state;
    };

    auto store = redux::create_store(reducer);

    REQUIRE(store.getState() == 0);

    REQUIRE_CALL(mock, state_listener(0)).IN_SEQUENCE(seq);
    store.subscribe([&](State _) { mock.state_listener(store.getState()); });

    REQUIRE_CALL(mock, state_listener(1)).IN_SEQUENCE(seq);
    store.dispatch("increase");

    REQUIRE_CALL(mock, state_listener(0)).IN_SEQUENCE(seq);
    store.dispatch("decrease");

    store.close();
}

TEST_CASE("store calls reducer on single thread")
{
    using State = int;
    using Action = std::string;

    std::thread::id non_thread_id;
    std::thread::id last_thread_id = non_thread_id;

    auto reducer = [&](State state, Action action) {
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
    for (int i = 0; i < 100; i++)
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
}