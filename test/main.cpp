#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <redux/redux.hpp>

TEST_CASE("it works")
{
    using State = int;
    using Action = std::string;

    auto reducer = [](State state, Action action) -> State {
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
}
