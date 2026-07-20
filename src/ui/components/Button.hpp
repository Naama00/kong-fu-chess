// This class represents a logical button. It manages its visual state
// (normal, hover, or pressed) and detects clicks on the logical axis system
#pragma once
#include "ui/framework/InputEvents.hpp"
#include <string>
#include <string_view>
#include <functional>
#include "ui/framework/IRenderer.hpp"

class Button {
private:
    Vector2D m_position;
    Vector2D m_size;
    std::string m_text;
    
    Color m_normalColor{60, 60, 60, 255};
    Color m_hoverColor{90, 90, 90, 255};
    Color m_textColor{255, 255, 255, 255};
    
    bool m_isHovered = false;
    std::function<void()> m_onClickCallback;

public:
    Button(Vector2D position, Vector2D size, std::string text, std::function<void()> onClickCallback)
        : m_position(position), m_size(size), m_text(std::move(text)), m_onClickCallback(std::move(onClickCallback)) {}

    // Update the button's state (e.g., for future animations)
    void update(float deltaTime) {
        (void)deltaTime; 
    }

    // Draw the button using the abstract Renderer
    void draw(IRenderer& renderer) const {
        Color currentBg = m_isHovered ? m_hoverColor : m_normalColor;
        
        // Draw the button's background rectangle
        renderer.drawRectangle(m_position, m_size, currentBg, true);
        
        // Draw the button's border
        renderer.drawRectangle(m_position, m_size, {120, 120, 120, 255}, false);
        
        // Calculate text position centered horizontally and vertically within the button
        Vector2D textPos{m_position.x + 15.0f, m_position.y + (m_size.y * 0.3f)};
        renderer.drawText(m_text, textPos, 16, m_textColor);
    }

    // Handle mouse input events to detect hover and click actions
    void handleInput(const MouseEvent& mouseEvent) {
        // Check if the mouse is hovering over the button
        m_isHovered = (mouseEvent.logicalX >= m_position.x && 
                       mouseEvent.logicalX <= m_position.x + m_size.x &&
                       mouseEvent.logicalY >= m_position.y && 
                       mouseEvent.logicalY <= m_position.y + m_size.y);

        // If the button is hovered and the left mouse button is pressed, trigger the click callback
        if (m_isHovered && mouseEvent.action == MouseEvent::Action::Press && mouseEvent.button == MouseButton::Left) {
            if (m_onClickCallback) {
                m_onClickCallback();
            }
        }
    }

    // Set custom colors for the button's normal, hover, and text states
    void setColors(Color normal, Color hover, Color text) {
        m_normalColor = normal;
        m_hoverColor = hover;
        m_textColor = text;
    }
};