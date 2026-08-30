# Input

`ouif::Input` is a leaf widget for editable text. Like `Label`, the text is drawn through the renderer text pipeline; `Input` adds focus, caret movement, selection, keyboard editing, and pointer hit testing around that draw feature.

## C++

```cpp
ouif::Input name("Humoy");
name.set_placeholder("Name");
name.set_font_size(16.0f);
name.set_text_color("#ffffff");
name.set_background("#151b24");
name.set_background_focused("#1d2735");
name.set_border_focused("#8dc7ff", 2.0f);
```

`Input` is focusable by default and does not accept children.

## Editing

```cpp
name.set_text("Hello");
name.set_caret(name.text().size());
name.insert_text(" OUIF");
name.select(0, 5);
name.copy_selection();
name.cut_selection();
name.paste_clipboard();
```

Supported editing operations:

- text: `set_text`, `text`, `get_text`, `clear_text`
- placeholder: `set_placeholder`, `placeholder`, `get_placeholder`
- composition: `set_composition_text`, `composition_text`, `get_composition_text`, `clear_composition`
- caret: `set_caret`, `caret`, `get_caret`
- selection: `select`, `select_all`, `clear_selection`, `has_selection`, `selection`
- editing: `insert_text`, `erase_selection`, `erase_previous`, `erase_next`
- clipboard: `copy_selection`, `cut_selection`, `paste_text`, `paste_clipboard`, `set_clipboard_text`, `clipboard_text`

Keyboard input supports character events, printable-key fallback for basic text, arrows, Home/End, Backspace, Delete, and Ctrl+A/C/X/V. The fallback is guarded so a GLFW key event followed by its matching character event does not insert the same character twice.

IME and platform composition can be fed through `set_composition_text(...)`; composition text is drawn next to committed text without mutating `text()` until the backend sends committed characters.

## Text Style

```cpp
name.set_text_style(ouif::TextStyle()
    .with_font_family("OUIF Sans")
    .with_font_size(16.0f)
    .with_color(ouif::Gradient::Linear(90.0f, {
        { 0.0f, "#ffffff" },
        { 1.0f, "#8dc7ff" },
    })));
```

Use `set_placeholder_color(...)` for empty, unfocused placeholder text.

## CSS

```css
.field {
    text: "Hello";
    placeholder: "Name";
    font-size: 16px;
    font-family: OUIF Sans;
    text-color: gradient(linear 90deg (0% #ffffff) (100% #8dc7ff));
    background: #151b24;
    background-focused: #1d2735;
    border-focused: 2px solid #8dc7ff;
}
```

Supported Input-specific properties:

- `text` / `value`
- `placeholder`
- `font-size`
- `font-family`
- `text-color`
- `placeholder-color`

All regular widget style, layout, focus, transform, animation, and effect properties also work.

## XML

```xml
<Input
    id="name"
    value="Humoy"
    placeholder="Name"
    font-size="16"
    text-color="#ffffff"
    style="background: #151b24; border-focused: 2px solid #8dc7ff;" />
```

`<InputField>` and `<TextInput>` are accepted as aliases. Text content is also used as the value when no `text` or `value` attribute is present.
