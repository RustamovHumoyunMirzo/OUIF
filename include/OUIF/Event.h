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

enum class Key : std::uint32_t {
    Space = 32,
    Enter = 257,
    Escape = 256,
    Tab = 258,
};

enum class MouseEventType : std::uint8_t {
    Move,
    Enter,
    Leave,
    Down,
    Up,
    Click,
};

enum class DragEventType : std::uint8_t {
    Start,
    Move,
    Drop,
    End,
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

struct MouseWheelEvent {
    Point position;
    Point local_position;
    float delta_x = 0.0f;
    float delta_y = 0.0f;
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
    bool shift = false;
    bool control = false;
    bool alt = false;
    bool super = false;
};

struct ResizeEvent {
    Size size;
};

struct DragEvent {
    DragEventType type = DragEventType::Move;
    Point position;
    Point start_position;
    Point delta;
};

using Event = std::variant<MouseMoveEvent, MouseButtonEvent, MouseWheelEvent, MouseEvent, KeyEvent, ResizeEvent, DragEvent>;

} // namespace ouif
