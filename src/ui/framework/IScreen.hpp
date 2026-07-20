// interface for a screen in the application, defining the lifecycle and rendering methods.
#pragma once
#include <vector>

class IRenderer;
struct InputEvent;
class ScreenManager;

class IScreen {
protected:
    ScreenManager& m_screenManager;

public:
    explicit IScreen(ScreenManager& manager) : m_screenManager(manager) {}
    virtual ~IScreen() = default;

    // Called when the screen is entered or activated at the top of the stack
    virtual void onEnter() = 0;

    // Called when the screen is exited or removed from the stack
    virtual void onExit() = 0;

    // Update the internal logic of the screen (deltaTime in seconds)
    virtual void update(float deltaTime) = 0;

    // Render the screen's components to the renderer
    virtual void draw(IRenderer& renderer) = 0;

    // Process input events received in the current frame
    virtual void handleInput(const std::vector<InputEvent>& events) = 0;
};