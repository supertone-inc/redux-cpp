#pragma once

#include <doctest.h>
#include <doctest/trompeloeil.hpp>

template <typename State, typename Action>
class BaseMock
{
public:
    MAKE_MOCK2(reducer, State(State, Action));
    MAKE_MOCK1(state_listener, void(State));
};
