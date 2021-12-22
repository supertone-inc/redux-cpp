#include <doctest.h>
#include <doctest/trompeloeil.hpp>

#include "mock.hpp"

#include <redux/redux.hpp>

namespace middleware
{
using State = int;
using Action = std::string;
using Mock = BaseMock<State, Action>;

Mock mock;

TEST_CASE("store applies middlewares")
{
    auto reducer = [](State state, Action action) { return state; };
    auto store = redux::create_store(reducer);

    auto middleware = [](auto store, auto next, auto action) { next(action); };

    store.apply_middleware(middleware);
}
} // namespace middleware
