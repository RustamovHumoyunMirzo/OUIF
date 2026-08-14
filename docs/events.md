# Events

OUIF exposes raw events so custom widgets can define their own behavior.

## Event Types

`ouif::Event` is a variant that can hold:

- `MouseMoveEvent`
- `MouseButtonEvent`
- `MouseWheelEvent`
- `MouseEvent`
- `KeyEvent`
- `ResizeEvent`

## Mouse Hooks

```cpp
void on_mouse_enter(const ouif::MouseEvent&) override;
void on_mouse_leave(const ouif::MouseEvent&) override;
bool on_mouse_move(const ouif::MouseEvent&) override;
bool on_mouse_down(const ouif::MouseEvent&) override;
bool on_mouse_up(const ouif::MouseEvent&) override;
bool on_click(const ouif::MouseEvent&) override;
```

The base widget tracks hover and pressed state automatically.

## Keyboard Hooks

```cpp
bool on_key_down(const ouif::KeyEvent&) override;
bool on_key_up(const ouif::KeyEvent&) override;
bool on_keyboard_activate(const ouif::KeyEvent&) override;
```

`on_keyboard_activate` is called for Enter/Space when keyboard activation is enabled.

## Generic Hook

```cpp
bool on_event(const ouif::Event& event) override;
```

Use this for event types that do not have a specialized hook yet, such as mouse wheel.

## Dispatch Order

Events are sent to children in reverse child order so later children behave as visually front-most. If `clip_content` is true, children do not receive mouse events outside the clipped parent bounds.
