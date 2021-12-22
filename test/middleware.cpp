#include <doctest.h>
#include <doctest/trompeloeil.hpp>

#include <redux/redux.hpp>

using State = int;
using Action = std::string;

TEST_CASE("store applies middlewares")
{
    auto reducer = [](State state, Action action) { return state; };
    auto store = redux::create_store(reducer);

    auto middleware = [&outer_store = store](auto store, auto next, auto action) { next(action); };

    store.apply_middleware(middleware);
}
