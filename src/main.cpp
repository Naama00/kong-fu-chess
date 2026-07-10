#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cctype>

#include "board/Board.hpp"
#include "io/BoardParser.hpp"
#include "io/BoardPrinter.hpp"
#include "rules/RuleEngine.hpp"
#include "engine/GameEngine.hpp"
#include "input/Controller.hpp"

namespace {

// פונקציית עזר להסרת רווחים מקצוות שורה
std::string trim(const std::string& str) {
    auto first = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
        return std::isspace(ch);
    });
    if (first == str.end()) {
        return "";
    }
    auto last = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
        return std::isspace(ch);
    }).base();
    return std::string(first, last);
}

} // namespace

int main() {
    // הגדרת תמיכה באופטימיזציה של מהירות קלט/פלט במידת הצורך
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::vector<std::string> allLines;
    std::string line;
    bool hasSections = false;

    // קריאת קלט מלא משורת הפקודה / קובץ בדיקה
    while (std::getline(std::cin, line)) {
        std::string trimmed = trim(line);
        if (trimmed == "Board:" || trimmed == "Commands:") {
            hasSections = true;
        }
        allLines.push_back(trimmed);
    }

    // --- תרחיש א': שלב א' בלבד (קלט לוח ישיר ללא הגדרת פקודות) ---
    if (!hasSections) {
        std::string boardRaw;
        for (const auto& l : allLines) {
            if (!l.empty()) {
                boardRaw += l + "\n";
            }
        }

        auto board = kungfu::BoardParser::parse(boardRaw);
        if (!board) {
            std::cout << "Invalid board\n";
            return 1;
        }

        // הדפסת הלוח הלוגי כפי שהתפרסר
        std::cout << kungfu::BoardPrinter::print(*board);
        return 0;
    }

    // --- תרחיש ב': משחק מלא עם כותרות "Board:" ו-"Commands:" ---
    std::string boardSection;
    std::vector<std::string> commandLines;
    bool readingBoard = false;
    bool readingCommands = false;

    for (const auto& l : allLines) {
        if (l.empty()) {
            continue;
        }
        if (l == "Board:") {
            readingBoard = true;
            readingCommands = false;
            continue;
        }
        if (l == "Commands:") {
            readingBoard = false;
            readingCommands = true;
            continue;
        }

        if (readingBoard) {
            boardSection += l + "\n";
        } else if (readingCommands) {
            commandLines.push_back(l);
        }
    }

    // 1. יצירת הלוח
    auto board = kungfu::BoardParser::parse(boardSection);
    if (!board) {
        std::cout << "Invalid board\n";
        return 1;
    }

    // 2. אתחול מנוע החוקים, מנוע המשחק והבקר (Controller)
    auto ruleEngine = std::make_shared<kungfu::RuleEngine>(board);
    auto gameEngine = std::make_shared<kungfu::GameEngine>(board, ruleEngine);
    kungfu::Controller controller(gameEngine); 
    // 3. עיבוד שורות פקודה
    for (const auto& cmd : commandLines) {
        std::istringstream stream(cmd);
        std::string action;
        stream >> action;

        if (action == "click") {
            int x = 0;
            int y = 0;
            if (stream >> x >> y) {
                controller.click(x, y);
            }
        } 
        else if (action == "wait") {
            int ms = 0;
            if (stream >> ms) {
                gameEngine->wait(ms);
            }
        } 
        else if (action == "print") {
            std::string target;
            if (stream >> target && target == "board") {
                std::cout << kungfu::BoardPrinter::print(*board);
            }
        }
    }

    return 0;
}