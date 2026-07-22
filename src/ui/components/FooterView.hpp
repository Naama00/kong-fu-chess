// ui/components/FooterView.hpp
#pragma once

#include "ui/framework/IRenderer.hpp"
#include "ui/framework/InputEvents.hpp"
#include "engine/common/Enums.hpp"
#include <string>

class FooterView {
public:
    FooterView() = default;
    ~FooterView() = default;

    FooterView(const FooterView&) = delete;
    FooterView& operator=(const FooterView&) = delete;

    /**
     * Draw the bottom information panel symmetrically and style it for both players
     */
    void draw(IRenderer& renderer,
              const std::string& whiteName,
              int whiteScore,
              const std::string& blackName,
              int blackScore,
              bool isGameOver,
              bool isPaused,
              bool allowSimultaneous,
              kungfu::PlayerColor currentTurn,
              Color bgColor,
              Color borderColor) const
    {
        // Draw the panel background and top border line
        renderer.drawRectangle({0.0f, 905.0f}, {1000.0f, 95.0f}, bgColor, true);
        renderer.drawLine({0.0f, 905.0f}, {1000.0f, 905.0f}, borderColor, 2.0f);

        // Display player names and material scores symmetrically for both players
        std::string whiteText = "WHITE (" + (whiteName.empty() ? "White" : whiteName) + "): " + std::to_string(whiteScore) + " / 39";
        std::string blackText = "BLACK (" + (blackName.empty() ? "Black" : blackName) + "): " + std::to_string(blackScore) + " / 39";

        renderer.drawText(whiteText, {30.0f, 960.0f}, 14, {220, 225, 235, 255});
        renderer.drawText(blackText, {380.0f, 960.0f}, 14, {220, 225, 235, 255});

        // Display the current game status on the right
        renderer.drawText("STATUS: ", {680.0f, 960.0f}, 13, {140, 145, 160, 255});
        
        if (isGameOver)
        {
            renderer.drawText("Match Ended", {750.0f, 960.0f}, 14, {240, 80, 80, 255});
        }
        else if (isPaused)
        {
            renderer.drawText("Paused", {750.0f, 960.0f}, 14, {240, 200, 80, 255});
        }
        else
        {
            if (!allowSimultaneous)
            {
                if (currentTurn == kungfu::PlayerColor::White)
                {
                    renderer.drawText("White's Turn", {750.0f, 960.0f}, 14, {240, 240, 250, 255});
                }
                else
                {
                    renderer.drawText("Black's Turn", {750.0f, 960.0f}, 14, {150, 150, 160, 255});
                }
            }
            else
            {
                renderer.drawText("KUNG-FU BATTLE!", {750.0f, 960.0f}, 14, {80, 180, 240, 255});
            }
        }
    }
};
