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
}

TEST_CASE("store publishes states")
{
    using State = int;
    using Action = std::string;
    using Mock = Mock<State, Action>;

    Mock mock;
    auto reducer = [](State state, Action action) { return state; };
    auto store = redux::create_store(reducer);

    REQUIRE_CALL(mock, state_listener(0));
    store.subscribe(bind(&Mock::state_listener, &mock, _1));

    REQUIRE_CALL(mock, state_listener(0));
    store.dispatch("an action");
}

TEST_CASE("state listeners can unsubscribe")
{
    using State = int;
    using Action = std::string;
    using Mock = Mock<State, Action>;

    Mock mock;
    auto reducer = [](State state, Action action) { return state; };
    auto store = redux::create_store(reducer);

    REQUIRE_CALL(mock, state_listener(0));
    auto unsubscribe = store.subscribe(bind(&Mock::state_listener, &mock, _1));

    REQUIRE_CALL(mock, state_listener(0));
    store.dispatch("an action");

    unsubscribe();

    FORBID_CALL(mock, state_listener(_));
    store.dispatch("an action");
}

TEST_CASE("store updates states")
{
    using State = int;
    using Action = std::string;
    using Mock = Mock<State, Action>;

    Mock mock;
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

    REQUIRE_CALL(mock, state_listener(0));
    store.subscribe(bind(&Mock::state_listener, &mock, _1));

    REQUIRE_CALL(mock, state_listener(1));
    store.dispatch("increase");

    REQUIRE_CALL(mock, state_listener(0));
    store.dispatch("decrease");
}

TEST_CASE("store exposes states")
{
    using State = int;
    using Action = std::string;

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

    {
        auto store = redux::create_store(reducer);

        REQUIRE(store.getState() == 0);

        store.dispatch("increase");
        REQUIRE(store.getState() == 1);

        store.dispatch("decrease");
        REQUIRE(store.getState() == 0);
    }

    {
        auto store = redux::create_store(reducer, 100);

        REQUIRE(store.getState() == 100);

        store.dispatch("increase");
        REQUIRE(store.getState() == 101);

        store.dispatch("decrease");
        REQUIRE(store.getState() == 100);
    }
}
