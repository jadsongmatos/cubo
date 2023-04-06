#include <iostream>
#include <cstdlib>
#include <wayland-client.h>
#include <vulkan/vulkan.h>

static void registry_global(void *data, struct wl_registry *registry, uint32_t name,
                            const char *interface, uint32_t version) {
  auto app = reinterpret_cast<wayland_data *>(data);
  if (strcmp(interface, "wl_compositor") == 0) {
    app->compositor = static_cast<wl_compositor *>(wl_registry_bind(registry, name,
                                                &wl_compositor_interface, 1));
  } else if (strcmp(interface, "wl_shell") == 0) {
    app->shell = static_cast<wl_shell *>(wl_registry_bind(registry, name,
                                            &wl_shell_interface, 1));
  }
}

static const struct wl_registry_listener registry_listener = {
    registry_global,
    nullptr,
};

int main() {
  // Initialize Wayland
  wl_display *display = wl_display_connect(nullptr);
  if (display == nullptr) {
    std::cerr << "Failed to connect to Wayland display" << std::endl;
    return 1;
  }

  wl_registry *registry = wl_display_get_registry(display);
  wayland_data data;
  wl_registry_add_listener(registry, &registry_listener, &data);
  wl_display_roundtrip(display);

  if (data.compositor == nullptr || data.shell == nullptr) {
    std::cerr << "Failed to find required Wayland interfaces" << std::endl;
    return 1;
  }

  wl_surface *surface = wl_compositor_create_surface(data.compositor);
  wl_shell_surface *shell_surface = wl_shell_get_shell_surface(data.shell, surface);
  wl_shell_surface_set_toplevel(shell_surface);

  // Initialize Vulkan
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Vulkan Wayland Example";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  VkInstance instance;
  if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
    std::cerr << "Failed to create Vulkan instance" << std::endl;
    return 1;
  }

  // Main loop
  while (true) {
    wl_display_dispatch_pending(display);
  }

  // Cleanup
  vkDestroyInstance(instance, nullptr);
  wl_shell_surface_destroy(shell_surface);
  wl_surface_destroy(surface);
  wl_shell_destroy(data.shell);
  wl_compositor_destroy(data.compositor);
  wl_registry_destroy(registry);
  wl_display_disconnect(display);

  return 0;
}
