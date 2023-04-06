#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-egl.h>

// Global variables for Wayland and EGL
struct wl_display* display;
struct wl_compositor* compositor;
struct wl_shell* shell;
struct wl_surface* surface;
struct wl_shell_surface* shell_surface;
struct wl_egl_window* egl_window;
struct wl_egl_pixmap* egl_pixmap;
struct wl_region* region;

// Function prototypes
void global_registry_handler(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
void global_registry_remover(void* data, struct wl_registry* registry, uint32_t name);

// Wayland registry listener
const struct wl_registry_listener registry_listener = {
    global_registry_handler,
    global_registry_remover
};

void init_wayland() {
    display = wl_display_connect(NULL);
    if (display == NULL) {
        throw std::runtime_error("Failed to connect to Wayland display");
    }

    struct wl_registry* registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);

    wl_display_dispatch(display);
    wl_display_roundtrip(display);

    if (compositor == NULL || shell == NULL) {
        throw std::runtime_error("Failed to bind Wayland compositor and shell");
    }

    wl_registry_destroy(registry);
}

void create_wayland_window() {
    surface = wl_compositor_create_surface(compositor);
    if (surface == NULL) {
        throw std::runtime_error("Failed to create Wayland surface");
    }

    shell_surface = wl_shell_get_shell_surface(shell, surface);
    if (shell_surface == NULL) {
        throw std::runtime_error("Failed to create Wayland shell surface");
    }

    wl_shell_surface_set_toplevel(shell_surface);

    // Set window size
    int width = 800;
    int height = 600;

    egl_window = wl_egl_window_create(surface, width, height);
    if (egl_window == NULL) {
        throw std::runtime_error("Failed to create Wayland EGL window");
    }

    // Create a region to set the window as opaque
    region = wl_compositor_create_region(compositor);
    wl_region_add(region, 0, 0, width, height);
    wl_surface_set_opaque_region(surface, region);

    wl_display_flush(display);
}

void global_registry_handler(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    if (strcmp(interface, "wl_compositor") == 0) {
        compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, 1));
    } else if (strcmp(interface, "wl_shell") == 0) {
        shell = static_cast<wl_shell*>(wl_registry_bind(registry, name, &wl_shell_interface, 1));
    }
}

void global_registry_remover(void* data, struct wl_registry* registry, uint32_t name) {
}

