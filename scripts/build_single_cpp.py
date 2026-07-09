#!/usr/bin/env python3
import argparse
import re
from pathlib import Path

# נתיבי בסיס של הפרויקט
ROOT = Path(__file__).resolve().parents[1]
OUTPUT_FILE = ROOT / "generated_single_file.cpp"

# סדר פתרון התלויות עבור קובצי ה-CPP (מבטיח הגדרות לפני שימושים)
CPP_ORDER = [
    "src/common/Position.cpp",
    "src/pieces/Piece.cpp",
    "src/pieces/King.cpp",
    "src/pieces/Queen.cpp",
    "src/pieces/Rook.cpp",
    "src/pieces/Bishop.cpp",
    "src/pieces/Knight.cpp",
    "src/pieces/Pawn.cpp",
    "src/board/Board.cpp",
    "src/io/BoardParser.cpp",
    "src/io/BoardPrinter.cpp",
    "src/input/BoardMapper.cpp",
    "src/input/Controller.cpp",
    "src/rules/PieceRules.cpp",
    "src/rules/RuleEngine.cpp",
    "src/realtime/Motion.cpp",
    "src/realtime/RealTimeArbiter.cpp",
    "src/engine/GameEngine.cpp"
]

# קוד ה-Driver האינטראקטיבי שיוזרק בסוף הקובץ המאוחד
DRIVER_CODE = """
// ============================================================================
// PLATFORM HARNESS DRIVER (AUTOMATICALLY APPENDED BY BUILD_SINGLE_CPP.PY)
// ============================================================================

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
    kungfu::Controller controller(gameEngine, 100); // 100 pixels per cell

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
"""


class Amalgamator:
    def __init__(self, root_dir: Path):
        self.root_dir = root_dir
        self.visited_headers = set()
        # אתחול מראש של ספריות המערכת הנדרשות עבור ה-Driver המרכזי
        self.system_includes = {"sstream", "string", "cctype", "vector", "iostream", "memory", "algorithm"}

    def resolve_file(self, file_path: Path) -> str:
        if not file_path.exists():
            return ""

        content = file_path.read_text(encoding="utf-8")
        lines = content.splitlines()
        result = []

        for line in lines:
            # דילוג על #pragma once
            if line.strip().startswith("#pragma once"):
                continue

            # ריכוז ספריות מערכת: #include <vector>
            system_match = re.match(r'^\s*#include\s*<([^>]+)>', line)
            if system_match:
                self.system_includes.add(system_match.group(1))
                continue

            # החלפת #include מקומי בתוכן הקובץ המקורי (פעם אחת בלבד)
            local_match = re.match(r'^\s*#include\s*"([^"]+)"', line)
            if local_match:
                inc_rel_path = local_match.group(1)
                # חיפוש תחת תיקיית include
                inc_path = self.root_dir / "include" / inc_rel_path
                if not inc_path.exists():
                    # חיפוש יחסי למיקום הקובץ הנוכחי
                    inc_path = (file_path.parent / inc_rel_path).resolve()

                if inc_path.exists():
                    resolved_abs = inc_path.resolve()
                    if resolved_abs not in self.visited_headers:
                        self.visited_headers.add(resolved_abs)
                        # קריאה רקורסיבית לפענוח ה-Header
                        header_content = self.resolve_file(resolved_abs)
                        result.append(header_content)
                continue

            result.append(line)

        return "\n".join(result)


def build_output(output_path: Path) -> None:
    amalgamator = Amalgamator(ROOT)
    cpp_bodies = []

    # עיבוד כל קובצי ה-CPP לפי הסדר שהוגדר
    for cpp_rel in CPP_ORDER:
        cpp_path = ROOT / cpp_rel
        if cpp_path.exists():
            resolved_cpp = amalgamator.resolve_file(cpp_path)
            cpp_bodies.append(f"// === Source: {cpp_rel} ===")
            cpp_bodies.append(resolved_cpp)
            cpp_bodies.append("\n")

    # בניית בלוק ההכללות של ספריות מערכת
    sorted_systems = sorted(list(amalgamator.system_includes))
    system_block = "\n".join(f"#include <{sys}>" for sys in sorted_systems)

    # הרכבת הקובץ המאוחד הסופי
    final_output = []
    final_output.append("// ============================================================================")
    final_output.append("// AUTOMATICALLY GENERATED SINGLE-FILE REPRESENTATION BY BUILD_SINGLE_CPP.PY")
    final_output.append("// ============================================================================\n")
    final_output.append(system_block)
    final_output.append("\n")
    final_output.extend(cpp_bodies)
    final_output.append(DRIVER_CODE)

    # כתיבה לקובץ היעד
    output_path.write_text("\n".join(final_output), encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(description="Build a single consolidated C++ file from the modular source.")
    parser.add_argument("--output", default=str(OUTPUT_FILE), help="Path to the generated .cpp file")
    args = parser.parse_args()

    output_path = Path(args.output).resolve()
    build_output(output_path)
    print(f"[Success] Single unified file generated at: {output_path}")


if __name__ == "__main__":
    main()