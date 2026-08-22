#include <OUIF/Application.h>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <utility>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#endif

namespace ouif {

struct Window::Impl {
    GLFWwindow* handle = nullptr;
    WindowConfig config {};
    bool owned = false;
};

struct Application::WindowSlot {
    Window window;
    std::unique_ptr<Widget> owned_root;
    Widget* root = nullptr;
    Color clear_color = Color::rgba(18, 20, 24, 255);
};

namespace {

int g_glfw_users = 0;

void ensure_glfw()
{
    if (g_glfw_users == 0 && glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("GLFW initialization failed");
    }
    ++g_glfw_users;
}

void release_glfw() noexcept
{
    if (g_glfw_users <= 0) {
        return;
    }
    --g_glfw_users;
    if (g_glfw_users == 0) {
        glfwTerminate();
    }
}

void* native_window(GLFWwindow* window)
{
#if defined(_WIN32)
    return glfwGetWin32Window(window);
#elif defined(__APPLE__)
    return glfwGetCocoaWindow(window);
#elif defined(__linux__)
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(glfwGetX11Window(window)));
#else
    (void)window;
    return nullptr;
#endif
}

MouseButton mouse_button_from_glfw(int button)
{
    switch (button) {
    case GLFW_MOUSE_BUTTON_RIGHT:
        return MouseButton::Right;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        return MouseButton::Middle;
    default:
        return MouseButton::Left;
    }
}

KeyAction key_action_from_glfw(int action)
{
    switch (action) {
    case GLFW_RELEASE:
        return KeyAction::Release;
    case GLFW_REPEAT:
        return KeyAction::Repeat;
    default:
        return KeyAction::Press;
    }
}

Application* app_from(GLFWwindow* window)
{
    return static_cast<Application*>(glfwGetWindowUserPointer(window));
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    if (auto* app = app_from(window)) {
        app->dispatch_event(MouseMoveEvent { { static_cast<float>(x), static_cast<float>(y) }, {} });
    }
}

void cursor_enter_callback(GLFWwindow* window, int entered)
{
    if (entered == GLFW_TRUE) {
        return;
    }

    if (auto* app = app_from(window)) {
        double x = 0.0;
        double y = 0.0;
        glfwGetCursorPos(window, &x, &y);
        app->dispatch_event(MouseEvent {
            MouseEventType::Leave,
            { static_cast<float>(x), static_cast<float>(y) },
            {},
            MouseButton::Left,
        });
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int)
{
    if (auto* app = app_from(window)) {
        double x = 0.0;
        double y = 0.0;
        glfwGetCursorPos(window, &x, &y);
        app->dispatch_event(MouseButtonEvent {
            { static_cast<float>(x), static_cast<float>(y) },
            {},
            mouse_button_from_glfw(button),
            action == GLFW_PRESS,
        });
    }
}

void scroll_callback(GLFWwindow* window, double x, double y)
{
    if (auto* app = app_from(window)) {
        double cursor_x = 0.0;
        double cursor_y = 0.0;
        glfwGetCursorPos(window, &cursor_x, &cursor_y);
        app->dispatch_event(MouseWheelEvent {
            { static_cast<float>(cursor_x), static_cast<float>(cursor_y) },
            {},
            static_cast<float>(x),
            static_cast<float>(y),
        });
    }
}

void key_callback(GLFWwindow* window, int key, int, int action, int)
{
    if (auto* app = app_from(window)) {
        app->dispatch_event(KeyEvent {
            static_cast<std::uint32_t>(key),
            key_action_from_glfw(action),
            (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS),
            (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS),
            (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS),
            (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS),
        });
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if (auto* app = app_from(window)) {
        app->dispatch_event(ResizeEvent { { static_cast<float>(width), static_cast<float>(height) } });
    }
}

void apply_window_hints(const WindowConfig& config)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, config.visible ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, config.decorated ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, config.always_on_top ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, config.focused ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, config.transparent_framebuffer || config.material == WindowMaterial::Transparent ? GLFW_TRUE : GLFW_FALSE);
    if (config.mode == WindowMode::Maximized) {
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    }
}

void install_callbacks(GLFWwindow* window, Application* app)
{
    glfwSetWindowUserPointer(window, app);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetCursorEnterCallback(window, cursor_enter_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

Window create_native_window(WindowConfig config, Application* app)
{
    ensure_glfw();
    apply_window_hints(config);

    GLFWmonitor* monitor = config.mode == WindowMode::Fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    GLFWwindow* handle = glfwCreateWindow(
        static_cast<int>(config.width),
        static_cast<int>(config.height),
        config.title.c_str(),
        monitor,
        nullptr
    );

    if (handle == nullptr) {
        release_glfw();
        throw std::runtime_error("GLFW window creation failed");
    }

    if (config.position) {
        glfwSetWindowPos(handle, static_cast<int>(config.position->x), static_cast<int>(config.position->y));
    }

    install_callbacks(handle, app);

    auto impl = std::make_shared<Window::Impl>();
    impl->handle = handle;
    impl->owned = true;
    impl->config = std::move(config);
    return Window(std::move(impl));
}

} // namespace

Window::Window() noexcept = default;

Window::Window(std::shared_ptr<Impl> impl) noexcept
    : impl_(std::move(impl))
{
}

Window::~Window()
{
    if (impl_ && impl_->owned && impl_->handle != nullptr) {
        glfwDestroyWindow(impl_->handle);
        impl_->handle = nullptr;
        release_glfw();
    }
}

Window::Window(Window&&) noexcept = default;
Window& Window::operator=(Window&&) noexcept = default;

bool Window::valid() const noexcept
{
    return impl_ && impl_->handle != nullptr;
}

void* Window::native_handle() const noexcept
{
    return valid() ? native_window(impl_->handle) : nullptr;
}

const WindowConfig& Window::config() const noexcept
{
    static const WindowConfig empty;
    return impl_ ? impl_->config : empty;
}

std::string_view Window::title() const noexcept
{
    return config().title;
}

Size Window::size() const noexcept
{
    if (!valid()) {
        return { static_cast<float>(config().width), static_cast<float>(config().height) };
    }
    int width = 0;
    int height = 0;
    glfwGetWindowSize(impl_->handle, &width, &height);
    return { static_cast<float>(width), static_cast<float>(height) };
}

Point Window::position() const noexcept
{
    if (!valid()) {
        return config().position.value_or(Point {});
    }
    int x = 0;
    int y = 0;
    glfwGetWindowPos(impl_->handle, &x, &y);
    return { static_cast<float>(x), static_cast<float>(y) };
}

bool Window::visible() const noexcept
{
    return valid() ? glfwGetWindowAttrib(impl_->handle, GLFW_VISIBLE) == GLFW_TRUE : config().visible;
}

bool Window::should_close() const noexcept
{
    return valid() && glfwWindowShouldClose(impl_->handle) == GLFW_TRUE;
}

bool Window::decorated() const noexcept
{
    return valid() ? glfwGetWindowAttrib(impl_->handle, GLFW_DECORATED) == GLFW_TRUE : config().decorated;
}

bool Window::resizable() const noexcept
{
    return valid() ? glfwGetWindowAttrib(impl_->handle, GLFW_RESIZABLE) == GLFW_TRUE : config().resizable;
}

bool Window::always_on_top() const noexcept
{
    return valid() ? glfwGetWindowAttrib(impl_->handle, GLFW_FLOATING) == GLFW_TRUE : config().always_on_top;
}

WindowTheme Window::theme() const noexcept
{
    return config().theme;
}

WindowMaterial Window::material() const noexcept
{
    return config().material;
}

void Window::set_title(std::string title)
{
    if (!impl_) {
        impl_ = std::make_shared<Impl>();
    }
    impl_->config.title = std::move(title);
    if (valid()) {
        glfwSetWindowTitle(impl_->handle, impl_->config.title.c_str());
    }
}

void Window::set_size(std::uint32_t width, std::uint32_t height)
{
    if (!impl_) {
        impl_ = std::make_shared<Impl>();
    }
    impl_->config.width = width;
    impl_->config.height = height;
    if (valid()) {
        glfwSetWindowSize(impl_->handle, static_cast<int>(width), static_cast<int>(height));
    }
}

void Window::set_position(float x, float y)
{
    if (!impl_) {
        impl_ = std::make_shared<Impl>();
    }
    impl_->config.position = Point { x, y };
    if (valid()) {
        glfwSetWindowPos(impl_->handle, static_cast<int>(x), static_cast<int>(y));
    }
}

void Window::show()
{
    if (impl_) {
        impl_->config.visible = true;
    }
    if (valid()) {
        glfwShowWindow(impl_->handle);
    }
}

void Window::hide()
{
    if (impl_) {
        impl_->config.visible = false;
    }
    if (valid()) {
        glfwHideWindow(impl_->handle);
    }
}

void Window::focus()
{
    if (valid()) {
        glfwFocusWindow(impl_->handle);
    }
}

void Window::request_close()
{
    if (valid()) {
        glfwSetWindowShouldClose(impl_->handle, GLFW_TRUE);
    }
}

void Window::set_decorated(bool decorated)
{
    if (!impl_) {
        impl_ = std::make_shared<Impl>();
    }
    impl_->config.decorated = decorated;
    if (valid()) {
        glfwSetWindowAttrib(impl_->handle, GLFW_DECORATED, decorated ? GLFW_TRUE : GLFW_FALSE);
    }
}

void Window::set_resizable(bool resizable)
{
    if (!impl_) {
        impl_ = std::make_shared<Impl>();
    }
    impl_->config.resizable = resizable;
    if (valid()) {
        glfwSetWindowAttrib(impl_->handle, GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
    }
}

void Window::set_always_on_top(bool always_on_top)
{
    if (!impl_) {
        impl_ = std::make_shared<Impl>();
    }
    impl_->config.always_on_top = always_on_top;
    if (valid()) {
        glfwSetWindowAttrib(impl_->handle, GLFW_FLOATING, always_on_top ? GLFW_TRUE : GLFW_FALSE);
    }
}

void Window::set_opacity(float opacity)
{
    if (valid()) {
        glfwSetWindowOpacity(impl_->handle, std::clamp(opacity, 0.0f, 1.0f));
    }
}

void Window::set_theme(WindowTheme theme) noexcept
{
    if (impl_) {
        impl_->config.theme = theme;
    }
}

void Window::set_material(WindowMaterial material) noexcept
{
    if (impl_) {
        impl_->config.material = material;
    }
}

Application::Application(ApplicationConfig config)
    : config_(std::move(config))
{
    config_.window.title = config_.title;
    config_.window.width = config_.width;
    config_.window.height = config_.height;
    config_.window.background = config_.clear_color;
}

Application::~Application()
{
    root_ = nullptr;
    owned_root_.reset();
    windows_.clear();
    primary_window_ = {};
}

Widget& Application::set_root(Widget& root)
{
    if (root.parent() != nullptr) {
        throw std::invalid_argument("Application root cannot already be a child of another widget");
    }

    owned_root_.reset();
    root_ = &root;
    return root;
}

Widget& Application::set_root(std::unique_ptr<Widget> root)
{
    if (!root) {
        throw std::invalid_argument("Cannot set a null application root");
    }

    auto& reference = *root;
    if (reference.parent() != nullptr) {
        throw std::invalid_argument("Application root cannot already be a child of another widget");
    }

    owned_root_ = std::move(root);
    root_ = &reference;
    return reference;
}

Widget* Application::root() noexcept
{
    return root_;
}

const Widget* Application::root() const noexcept
{
    return root_;
}

Window& Application::window()
{
    return primary_window_;
}

const Window& Application::window() const
{
    return primary_window_;
}

Window& Application::create_window(WindowConfig config)
{
    auto slot = std::make_unique<WindowSlot>();
    slot->clear_color = config.background;
    slot->window = create_native_window(std::move(config), this);
    auto& reference = slot->window;
    windows_.push_back(std::move(slot));
    return reference;
}

Widget& Application::set_root(Window& target_window, Widget& root)
{
    if (&target_window == &primary_window_) {
        return set_root(root);
    }

    auto& slot = slot_for(target_window);
    if (root.parent() != nullptr) {
        throw std::invalid_argument("Window root cannot already be a child of another widget");
    }
    slot.owned_root.reset();
    slot.root = &root;
    return root;
}

Widget& Application::set_root(Window& target_window, std::unique_ptr<Widget> root)
{
    if (!root) {
        throw std::invalid_argument("Cannot set a null window root");
    }
    if (&target_window == &primary_window_) {
        return set_root(std::move(root));
    }

    auto& slot = slot_for(target_window);
    auto& reference = *root;
    if (reference.parent() != nullptr) {
        throw std::invalid_argument("Window root cannot already be a child of another widget");
    }
    slot.owned_root = std::move(root);
    slot.root = &reference;
    return reference;
}

Widget* Application::root(Window& target_window) noexcept
{
    if (&target_window == &primary_window_) {
        return root_;
    }
    for (auto& slot : windows_) {
        if (&slot->window == &target_window) {
            return slot->root;
        }
    }
    return nullptr;
}

const Widget* Application::root(const Window& target_window) const noexcept
{
    if (&target_window == &primary_window_) {
        return root_;
    }
    for (const auto& slot : windows_) {
        if (&slot->window == &target_window) {
            return slot->root;
        }
    }
    return nullptr;
}

Window& Application::show_dialog(DialogConfig config, std::unique_ptr<Widget> root)
{
    WindowConfig window_config;
    window_config.title = std::move(config.title);
    window_config.width = config.width;
    window_config.height = config.height;
    window_config.decorated = true;
    window_config.resizable = false;
    window_config.always_on_top = config.modal;
    window_config.theme = config.theme;
    window_config.material = config.material;
    window_config.background = config.background;
    window_config.owner = config.owner != nullptr ? config.owner : (primary_window_.valid() ? &primary_window_ : nullptr);

    auto& dialog = create_window(std::move(window_config));
    if (root) {
        set_root(dialog, std::move(root));
    }
    return dialog;
}

Window& Application::show_dialog(const DialogBuilder& builder, std::unique_ptr<Widget> root)
{
    return show_dialog(builder.config(), std::move(root));
}

bool Application::load_font(std::string family, std::filesystem::path path)
{
    return renderer_.load_font(std::move(family), std::move(path));
}

bool Application::load_default_system_font()
{
    return renderer_.load_default_system_font();
}

void Application::set_default_font_family(std::string family)
{
    renderer_.set_default_font_family(std::move(family));
}

std::string_view Application::default_font_family() const noexcept
{
    return renderer_.default_font_family();
}

int Application::run()
{
    if (config_.native_window == nullptr && config_.create_window && !primary_window_.valid()) {
        primary_window_ = create_native_window(config_.window, this);
    }

    initialize_renderer(primary_window_.valid() ? primary_window_.native_handle() : config_.native_window);

    running_ = true;
    while (running_ && (!primary_window_.valid() || !primary_window_.should_close())) {
        glfwPollEvents();
        windows_.erase(std::remove_if(windows_.begin(), windows_.end(), [](const auto& slot) {
            return slot->window.should_close();
        }), windows_.end());
        frame();
    }

    renderer_.shutdown();
    return 0;
}

void Application::frame()
{
    if (!renderer_.initialized()) {
        initialize_renderer(config_.native_window);
        running_ = true;
    }

    const auto size = renderer_.size();
    if (root_ != nullptr) {
        root_->set_bounds({ 0.0f, 0.0f, size.width, size.height });
        root_->layout(size);
    }

    renderer_.begin_frame(config_.clear_color);
    if (root_ != nullptr) {
        root_->render(renderer_);
    }
    renderer_.end_frame();
}

bool Application::dispatch_event(const Event& event)
{
    if (const auto* resize = std::get_if<ResizeEvent>(&event)) {
        renderer_.resize(
            static_cast<std::uint32_t>(std::max(0.0f, resize->size.width)),
            static_cast<std::uint32_t>(std::max(0.0f, resize->size.height))
        );
    }

    return root_ != nullptr && root_->event(event);
}

void Application::request_exit() noexcept
{
    running_ = false;
}

void Application::initialize_renderer(void* target_native_window)
{
    if (renderer_.initialized()) {
        return;
    }

    renderer_.initialize({
        target_native_window,
        config_.width,
        config_.height,
        nullptr,
        true,
        config_.render_quality,
    });
}

Application::WindowSlot& Application::main_slot()
{
    throw std::logic_error("Primary window uses Application::root() storage");
}

Application::WindowSlot& Application::slot_for(Window& target_window)
{
    for (auto& slot : windows_) {
        if (&slot->window == &target_window) {
            return *slot;
        }
    }
    throw std::invalid_argument("Window does not belong to this application");
}

const Application::WindowSlot& Application::slot_for(const Window& target_window) const
{
    for (const auto& slot : windows_) {
        if (&slot->window == &target_window) {
            return *slot;
        }
    }
    throw std::invalid_argument("Window does not belong to this application");
}

} // namespace ouif
