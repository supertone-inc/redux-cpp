#pragma once

#include <doctest/trompeloeil.hpp>

template <typename State, typename Action> class BaseMock
{
public:
    MAKE_MOCK1(action_listener, void(Action));
    MAKE_MOCK1(state_listener, void(State));
    MAKE_MOCK2(reducer, State(State, Action));
};
