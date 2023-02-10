#include "player.h"
#include "iostream"

namespace cell {

    using namespace undicht;
    using namespace tools;

    void Player::init() {

        setPosition(glm::vec3(0, -2, 2));
    }

    void Player::cleanUp() {
    }

    void Player::enableMouseInput(bool enable) {

        _use_mouse_input = enable;
    }

    void Player::move(double delta_t, const undicht::graphics::Window &input_window) {
        /// @brief move the player
        /// @param delta_t time since last call in milliseconds

        float step = float(delta_t) * 0.02f;

        // keyboard input
        if (input_window.isKeyPressed(GLFW_KEY_W))
            setPosition(getPosition() + getViewDirection() * step);

        if (input_window.isKeyPressed(GLFW_KEY_S))
            setPosition(getPosition() - getViewDirection() * step);

        if (input_window.isKeyPressed(GLFW_KEY_A))
            setPosition(getPosition() - getRightDirection() * step);

        if (input_window.isKeyPressed(GLFW_KEY_D))
            setPosition(getPosition() + getRightDirection() * step);

        // mouse input
        if (_use_mouse_input) {
            const float mouse_speed = 0.1f;
            float cursor_offset_x = 0, cursor_offset_y = 0;
            double cursor_x, cursor_y;
            input_window.getCursorPos(cursor_x, cursor_y);

            if (_cursor_initialized) {
                cursor_offset_x = cursor_x - _last_cursor_x;
                cursor_offset_y = cursor_y - _last_cursor_y;
            }

            _yaw -= cursor_offset_x * mouse_speed;
            _pitch += cursor_offset_y * mouse_speed;

            setAxesRotation({0, _pitch, _yaw}, {UND_Z_AXIS, UND_X_AXIS, UND_Y_AXIS});

            _last_cursor_x = cursor_x;
            _last_cursor_y = cursor_y;
            _cursor_initialized = true;
        } else {
            _cursor_initialized = false;
        }
    }

} // namespace cell