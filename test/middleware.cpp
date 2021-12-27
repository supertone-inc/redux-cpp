#include <doctest.h>
#include <doctest/trompeloeil.hpp>

#include "mock.hpp"

#include <redux/redux.hpp>

namespace middleware
{
using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;
using trompeloeil::_;

using State = int;
using Action = std::string;
using Store = redux::Store<State, Action>;
using Reducer = Store::Reducer;
using Middleware = Store::Middleware;
using StoreCreator = std::function<Store(Reducer, State)>;
using StoreEnhancer = std::function<StoreCreator(StoreCreator)>;

class Mock : public BaseMock<State, Action>
{
    MAKE_MOCK1(enhancer, StoreCreator(StoreCreator));
    MAKE_MOCK3(middleware, void(Store *, Store::Next, Action));
};

Mock mock;
trompeloeil::sequence seq;

Reducer reducer = bind(&Mock::reducer, &mock, _1, _2);
StoreEnhancer enhancer = bind(&Mock::enhancer, &mock, _1);

TEST_CASE("store can be created using store enhancer")
{
    REQUIRE_CALL(mock, enhancer(_)).RETURN(_1);
    auto store = redux::create_store(reducer, State(), enhancer);
}

TEST_CASE("store applies middlewares")
{
    SUBCASE("single middleware")
    {
        auto store = redux::create_store(reducer);
        store.apply_middleware([&](auto store, auto next, auto action) {
            mock.middleware(store, next, action);
            next(action);
        });

        REQUIRE_CALL(mock, middleware(&store, _, "action")).IN_SEQUENCE(seq);
        REQUIRE_CALL(mock, reducer(_, "action")).RETURN(_1).IN_SEQUENCE(seq);
        store.dispatch("action");

        store.close();
    }

    SUBCASE("multiple middlewares")
    {
        auto store = redux::create_store(reducer);
        for (int i = 0; i < 3; i++)
        {
            store.apply_middleware([&, i](auto store, auto next, auto action) {
                mock.middleware(store, next, action);
                next(action + std::to_string(i));
            });
        }

        REQUIRE_CALL(mock, middleware(&store, _, "action")).IN_SEQUENCE(seq);
        REQUIRE_CALL(mock, middleware(&store, _, "action2")).IN_SEQUENCE(seq);
        REQUIRE_CALL(mock, middleware(&store, _, "action21")).IN_SEQUENCE(seq);
        REQUIRE_CALL(mock, reducer(_, "action210")).RETURN(_1).IN_SEQUENCE(seq);
        store.dispatch("action");

        store.close();
    }
}

TEST_CASE("store updates state via middlewares")
{
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

    REQUIRE_CALL(mock, reducer(0, "increase")).RETURN(_1 + 1).IN_SEQUENCE(seq);
    REQUIRE_CALL(mock, state_listener(1)).IN_SEQUENCE(seq);
    REQUIRE_CALL(mock, reducer(1, "double")).RETURN(_1 * 2).IN_SEQUENCE(seq);
    REQUIRE_CALL(mock, state_listener(2)).IN_SEQUENCE(seq);
    store.dispatch("increase and double");

    store.close();
}
} // namespace middleware
