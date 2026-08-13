#include <OUIF/Application.h>

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

namespace {

struct GlfwWindow {
    GLFWwindow* handle = nullptr;
    bool owned = false;
};

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

void key_callback(GLFWwindow* window, int key, int, int action, int)
{
    if (auto* app = app_from(window)) {
        app->dispatch_event(KeyEvent { static_cast<std::uint32_t>(key), key_action_from_glfw(action) });
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if (auto* app = app_from(window)) {
        app->dispatch_event(ResizeEvent { { static_cast<float>(width), static_cast<float>(height) } });
    }
}

GlfwWindow create_window(const ApplicationConfig& config, Application* app)
{
    if (config.native_window != nullptr || !config.create_window) {
        return {};
    }

    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("GLFW initialization failed");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(
        static_cast<int>(config.width),
        static_cast<int>(config.height),
        config.title.c_str(),
        nullptr,
        nullptr
    );

    if (window == nullptr) {
        glfwTerminate();
        throw std::runtime_error("GLFW window creation failed");
    }

    glfwSetWindowUserPointer(window, app);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    return { window, true };
}

void destroy_window(GlfwWindow window) noexcept
{
    if (window.owned && window.handle != nullptr) {
        glfwDestroyWindow(window.handle);
        glfwTerminate();
    }
}

bool should_close(GlfwWindow window) noexcept
{
    return window.owned && window.handle != nullptr && glfwWindowShouldClose(window.handle) == GLFW_TRUE;
}

} // namespace

Application::Application(ApplicationConfig config)
    : config_(std::move(config))
{
}

Application::~Application() = default;
Application::Application(Application&&) noexcept = default;
Application& Application::operator=(Application&&) noexcept = default;

void Application::set_root(std::unique_ptr<Widget> root)
{
    root_ = std::move(root);
}

Widget* Application::root() noexcept
{
    return root_.get();
}

const Widget* Application::root() const noexcept
{
    return root_.get();
}

int Application::run()
{
    auto window = create_window(config_, this);
    initialize_renderer(window.handle != nullptr ? native_window(window.handle) : config_.native_window);

    running_ = true;
    while (running_ && !should_close(window)) {
        glfwPollEvents();
        frame();
    }

    renderer_.shutdown();
    destroy_window(window);
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
            static_cast<std::uint32_t>(resize->size.width),
            static_cast<std::uint32_t>(resize->size.height)
        );
    }

    return root_ != nullptr && root_->event(event);
}

void Application::request_exit() noexcept
{
    running_ = false;
}

void Application::initialize_renderer(void* native_window)
{
    if (renderer_.initialized()) {
        return;
    }

    renderer_.initialize({
        native_window,
        config_.width,
        config_.height,
        nullptr,
        true,
    });
}

} // namespace ouif
