#pragma once

#include <functional>

#include "../types.h"

namespace dsr {
    namespace input {
        typedef U8 Key;

        enum class MouseButton : U8 {
            LEFT,
            MIDDLE,
            RIGHT
        };

        enum class InputAction {
            PRESSED,
            RELEASED
        };

        void update();

        Key fromSdlKey(S32 keycode);

        void initInput();

        void setCharCallback(std::function<void(Key, InputAction)> callback);

        void setKeyCallback(std::function<void(Key, InputAction)> callback);

        void setMouseButtonCallback(std::function<void(MouseButton, InputAction)> callback);

        void setMouseWheelCallback(std::function<void(F32, F32)> callback);

        F32 mouseX();

        F32 mouseY();

        F32 wheelX();

        F32 wheelY();

        bool keyDown(Key key);

        bool keyPressed(Key key);

        bool keyReleased(Key key);

        bool mouseButtonDown(MouseButton button);

        bool mouseButtonPressed(MouseButton button);

        bool mouseButtonReleased(MouseButton button);

        bool frameCloseRequested();
    }
}