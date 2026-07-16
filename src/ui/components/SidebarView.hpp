#pragma once

#include "ui/framework/IRenderer.hpp"
#include "ui/framework/InputEvents.hpp"
#include <vector>
#include <string>

class SidebarView {
public:
    SidebarView() = default;
    ~SidebarView() = default;

    // מניעת העתקה מטעמי יעילות
    SidebarView(const SidebarView&) = delete;
    SidebarView& operator=(const SidebarView&) = delete;
    SidebarView(SidebarView&&) noexcept = default;
    SidebarView& operator=(SidebarView&&) noexcept = default;

    /**
     * ציור פאנל המהלכים הצידי בהתבסס על ההיסטוריה הלוגית שהתקבלה מהמסך.
     */
    void draw(IRenderer& renderer, 
              const std::vector<std::string>& whiteHistory, 
              const std::vector<std::string>& blackHistory,
              Color bgThemeColor,
              Color borderThemeColor) const 
    {
        // 1. רקע הפאנל הראשי וקו הגבול השמאלי
        renderer.drawRectangle({800.0f, 100.0f}, {200.0f, 800.0f}, bgThemeColor, true);
        renderer.drawLine({800.0f, 100.0f}, {800.0f, 900.0f}, borderThemeColor, 2.0f);

        // --- 2. אזור מהלכי השחקן הלבן ---
        renderer.drawRectangle({810.0f, 110.0f}, {180.0f, 360.0f}, {30, 30, 40, 255}, true);
        renderer.drawRectangle({810.0f, 110.0f}, {180.0f, 360.0f}, borderThemeColor, false);
        renderer.drawText("WHITE MOVES", {825.0f, 145.0f}, 13, {255, 255, 255, 255});
        renderer.drawLine({815.0f, 160.0f}, {985.0f, 160.0f}, {50, 50, 65, 255}, 1.0f);

        float whiteLogY = 195.0f;
        for (const auto &logEntry : whiteHistory)
        {
            Color textColor = {200, 200, 210, 255};
            if (logEntry.find("rejected") != std::string::npos || logEntry.find("failed") != std::string::npos)
            {
                textColor = {240, 100, 100, 255}; // אדום לכישלון
            }
            else if (logEntry.find("ok") != std::string::npos || logEntry.find("Connected") != std::string::npos || logEntry.find(":") != std::string::npos)
            {
                textColor = {100, 210, 130, 255}; // ירוק להצלחה / מהלך תקין
            }
            renderer.drawText(logEntry, {825.0f, whiteLogY}, 10, textColor);
            whiteLogY += 35.0f;
        }

        // --- 3. אזור מהלכי השחקן השחור ---
        renderer.drawRectangle({810.0f, 490.0f}, {180.0f, 360.0f}, {30, 30, 40, 255}, true);
        renderer.drawRectangle({810.0f, 490.0f}, {180.0f, 360.0f}, borderThemeColor, false);
        renderer.drawText("BLACK MOVES", {825.0f, 525.0f}, 13, {160, 160, 175, 255});
        renderer.drawLine({815.0f, 540.0f}, {985.0f, 540.0f}, {50, 50, 65, 255}, 1.0f);

        float blackLogY = 575.0f;
        for (const auto &logEntry : blackHistory)
        {
            Color textColor = {170, 170, 185, 255};
            if (logEntry.find("rejected") != std::string::npos || logEntry.find("failed") != std::string::npos)
            {
                textColor = {240, 100, 100, 255};
            }
            else if (logEntry.find("ok") != std::string::npos || logEntry.find("Connected") != std::string::npos || logEntry.find(":") != std::string::npos)
            {
                textColor = {100, 210, 130, 255};
            }
            renderer.drawText(logEntry, {825.0f, blackLogY}, 10, textColor);
            blackLogY += 35.0f;
        }
    }
};