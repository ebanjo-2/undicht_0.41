#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "string"

#include "monitor.h"

namespace undicht {

    namespace graphics {

        class Window {
            
            private:

            GLFWwindow* m_window = 0;
            VkSurfaceKHR m_surface;
            VkInstance m_instance;

            bool m_fullscreen = false;
            uint32_t m_width = 0;
            uint32_t m_height = 0;
            bool m_has_resized = false; // since the last frame

            public:

            Window() = default;
            Window(const VkInstance& instance, const std::string& title = "", uint32_t width = 800, uint32_t height = 600);

            void open(const VkInstance& instance, const std::string& title = "", uint32_t width = 800, uint32_t height = 600);
            void close();

            void setTitle(const std::string& title);
            std::string getTitle() const;

            void setSize(uint32_t width, uint32_t height);
            void getSize(uint32_t &width, uint32_t &height) const;
            uint32_t getWidth() const;
            uint32_t getHeight() const;

            void setFullscreen(Monitor* monitor);
            void setWindowed(uint32_t &width, uint32_t &height); // undo fullscreen
            bool isFullscreen() const;

            void setCursorEnabled(bool enabled);

            void update();
            void waitForEvent();

            // events
            bool shouldClose() const;
            bool hasResized() const;
            bool isMinimized() const;

            // user input
            bool isKeyPressed(int key_id) const;
            void getCursorPos(double& x, double& y) const;

            const VkSurfaceKHR& getSurface() const;
            const GLFWwindow* getWindow() const;

        };

    } // namespace graphics

} // namespace undicht

#endif // WINDOW_H
