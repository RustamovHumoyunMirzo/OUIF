#pragma once

#include <OUIF/Geometry.h>

#include <cstdint>
#include <variant>

namespace ouif {

enum class MouseButton : std::uint8_t {
    Left,
    Right,
    Middle,
};

enum class KeyAction : std::uint8_t {
    Press,
    Release,
    Repeat,
};

enum class MouseEventType : std::uint8_t {
    Move,
    Enter,
    Leave,
    Down,
    Up,
    Click,
};

struct MouseMoveEvent {
    Point position;
    Point local_position;
};

struct MouseButtonEvent {
    Point position;
    Point local_position;
    MouseButton button = MouseButton::Left;
    bool pressed = false;
};

struct MouseEvent {
    MouseEventType type = MouseEventType::Move;
    Point position;
    Point local_position;
    MouseButton button = MouseButton::Left;
};

struct KeyEvent {
    std::uint32_t key = 0;
    KeyAction action = KeyAction::Press;
};

struct ResizeEvent {
    Size size;
};

using Event = std::variant<MouseMoveEvent, MouseButtonEvent, MouseEvent, KeyEvent, ResizeEvent>;

} // namespace ouif
