#include <doctest.h>
#include <doctest/trompeloeil.hpp>

#include "mock.hpp"

#include <redux/redux.hpp>

namespace middleware
{
using trompeloeil::_;

using State = int;
using Action = std::string;
using Store = redux::Store<State, Action>;

class Mock : BaseMock<State, Action>
{
    MAKE_MOCK3(middleware_listener, void(Store*, Store::Next, Action));
};

Mock mock;

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
} // namespace middleware
