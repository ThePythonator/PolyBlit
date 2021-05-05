#pragma once

#include "32blit.hpp"
#include "linalg.h"

#include "Camera.hpp"
#include "World.hpp"

using namespace blit;
using namespace linalg::aliases;


struct ButtonStates {
    uint8_t UP;
    uint8_t DOWN;
    uint8_t LEFT;
    uint8_t RIGHT;
    uint8_t A;
    uint8_t B;
    uint8_t X;
    uint8_t Y;
    uint8_t JOYSTICK;
};
