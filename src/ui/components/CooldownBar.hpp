// A visual cooldown bar intended to be shown above or below active chess pieces. 
// It receives the progress percentage and draws a rectangle that fills or empties accordingly
#pragma once
#include "ui/framework/InputEvents.hpp"
#include "ui/framework/IRenderer.hpp"

class CooldownBar {
private:
    Vector2D m_size{40.0f, 6.0f}; // Default size of the cooldown bar
    Color m_backgroundColor{40, 40, 40, 200};
    Color m_fillColor{0, 220, 100, 255}; // Green color for the ready or loading state
    Color m_cooldownColor{220, 50, 50, 255}; // Red color during active cooldown

public:
    CooldownBar() = default;

    /**
     * Draw the cooldown bar.
     * @param renderer the abstract renderer.
     * @param centerPosition the central position of the piece (the bar will be drawn slightly below it).
     * @param cooldownProgress a value between 0.0f and 1.0f (where 1.0f represents a fully active cooldown and 0.0f represents a ready piece).
     */
    void draw(IRenderer& renderer, Vector2D centerPosition, float cooldownProgress) const {
        if (cooldownProgress <= 0.0f) {
            return; // If the piece is not on cooldown, there's no need to draw the bar
        }

        // Calculate the position of the rectangle so it's centered under the piece
        Vector2D barPos{
            centerPosition.x - (m_size.x / 2.0f),
            centerPosition.y + 25.0f // Vertical offset below the piece
        };

        // 1. Draw the background of the bar (dark gray rectangle)
        renderer.drawRectangle(barPos, m_size, m_backgroundColor, true);

        // 2. Calculate the width of the filled portion based on the remaining cooldown
        float filledWidth = m_size.x * (1.0f - cooldownProgress);
        Vector2D fillSize{filledWidth, m_size.y};

        // 3. Draw the filled portion of the bar (in the cooldown color)
        renderer.drawRectangle(barPos, fillSize, m_cooldownColor, true);

        // 4. Draw the border around the bar
        renderer.drawRectangle(barPos, m_size, {200, 200, 200, 255}, false);
    }

    void setSize(Vector2D size) {
        m_size = size;
    }
};