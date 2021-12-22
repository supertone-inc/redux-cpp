#include <doctest.h>
#include <doctest/trompeloeil.hpp>

#include "mock.hpp"

#include <redux/redux.hpp>

namespace middleware
{
using std::bind;
using std::placeholders::_1;
using trompeloeil::_;

using State = int;
using Action = std::string;
using Store = redux::Store<State, Action>;

class Mock : public BaseMock<State, Action>
{
    MAKE_MOCK3(middleware_listener, void(Store *, Store::Next, Action));
};

Mock mock;
trompeloeil::sequence seq;

TEST_CASE("store applies middlewares")
{
    auto reducer = [](State state, Action action) { return state; };
    auto store = redux::create_store(reducer);
    auto middleware = [&](auto store, auto next, auto action) {
        mock.middleware_listener(store, next, action);
        next(action);
    };
    store.apply_middleware(middleware);

    REQUIRE_CALL(mock, middleware_listener(&store, _, "action"));
    store.dispatch("action");

    store.close();
}

TEST_CASE("store updates state via middlewares")
{
    auto reducer = [&](State state, Action action) {
        mock.reducer(state, action);

        if (action == "increase")
        {
            return state + 1;
        }

        if (action == "double")
        {
            return state * 2;
        }

        return state;
    };
    auto store = redux::create_store(reducer);
    auto middleware = [&](auto store, auto next, auto action) {
        if (action == "increase and double")
        {
            next("increase");
            next("double");
        }
    };
    store.apply_middleware(middleware);

    REQUIRE_CALL(mock, state_listener(0)).IN_SEQUENCE(seq);
    store.subscribe(bind(&Mock::state_listener, &mock, _1));

    REQUIRE_CALL(mock, reducer(0, "increase")).RETURN(1).IN_SEQUENCE(seq);
    REQUIRE_CALL(mock, state_listener(1)).IN_SEQUENCE(seq);
    REQUIRE_CALL(mock, reducer(1, "double")).RETURN(2).IN_SEQUENCE(seq);
    REQUIRE_CALL(mock, state_listener(2)).IN_SEQUENCE(seq);
    store.dispatch("increase and double");

    store.close();
}
} // namespace middleware
