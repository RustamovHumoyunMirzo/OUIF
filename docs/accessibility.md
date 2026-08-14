# Accessibility And Keyboard

OUIF exposes accessibility metadata now so custom widgets can be designed with semantics from the start.

## Roles

```cpp
set_accessibility_role(ouif::AccessibilityRole::Button);
```

Current roles:

- `None`
- `Widget`
- `Button`
- `Checkbox`
- `Radio`
- `Slider`
- `TextInput`
- `Label`
- `Container`

## Labels And Descriptions

```cpp
set_accessibility_label("Color tile");
set_accessibility_description("Toggles the selected state");
```

Or set everything at once:

```cpp
set_accessibility({
    ouif::AccessibilityRole::Button,
    "Save",
    "Saves the current document",
});
```

## Focus

```cpp
set_focusable(true);
focus();
blur();
```

Keyboard navigation:

- `Tab`: next focusable widget.
- `Shift+Tab`: previous focusable widget.

## Keyboard Activation

```cpp
set_keyboard_activation_enabled(true);
```

This makes a widget focusable and lets Enter/Space trigger `on_keyboard_activate`. The default implementation calls `on_click`, so button-like widgets usually only need one behavior implementation.
