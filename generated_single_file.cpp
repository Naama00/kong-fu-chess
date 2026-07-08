#include <algorithm>
#include <cmath>
#include <cctype>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Position {
    int row = 0;
    int col = 0;
};

struct MoveRequest {
    Position from;
    Position to;
    int arrival_time_ms = 1000;
};

struct JumpRequest {
    Position cell;          // The cell the piece is airborne over (stays here).
    std::string piece;      // The piece token that is airborne.
    int land_time_ms = 0;  // Time at which the jump resolves (airborne_start + 1000).
};

using Board = std::vector<std::vector<std::string>>;

bool is_valid_piece_token(const std::string& token) {
    static const std::vector<std::string> valid = {
        ".", "wK", "wQ", "wR", "wB", "wN", "wP",
        "bK", "bQ", "bR", "bB", "bN", "bP"
    };
    return std::find(valid.begin(), valid.end(), token) != valid.end();
}

std::vector<std::string> split_ws(const std::string& text) {
    std::istringstream ss(text);
    std::vector<std::string> tokens;
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool is_inside(const Position& pos, const Board& board) {
    return pos.row >= 0 && pos.row < static_cast<int>(board.size()) &&
           pos.col >= 0 && pos.col < static_cast<int>(board[pos.row].size());
}

bool same_color(const std::string& a, const std::string& b) {
    if (a == "." || b == ".") {
        return false;
    }
    return a[0] == b[0];
}

char piece_type(const std::string& token) {
    return token == "." ? '.' : token[1];
}

bool is_legal_move(const Board& board, const Position& from, const Position& to,
                   const std::optional<JumpRequest>& airborne = std::nullopt) {
    if (!is_inside(from, board) || !is_inside(to, board) || (from.row == to.row && from.col == to.col)) {
        return false;
    }

    const std::string& piece = board[from.row][from.col];
    // For the target: if there's an airborne enemy piece there, treat the cell as
    // empty for movement purposes (the airborne piece will handle the capture).
    const bool target_is_airborne_enemy =
        airborne.has_value() &&
        airborne->cell.row == to.row &&
        airborne->cell.col == to.col &&
        !same_color(piece, airborne->piece);
    const std::string effective_target = target_is_airborne_enemy ? "." : board[to.row][to.col];
    const std::string& target = effective_target;

    if (piece == ".") {
        return false;
    }
    if (target != "." && same_color(piece, target)) {
        return false;
    }

    const int row_delta = to.row - from.row;
    const int col_delta = to.col - from.col;
    const int abs_row = std::abs(row_delta);
    const int abs_col = std::abs(col_delta);

    switch (piece_type(piece)) {
    case 'K':
        return abs_row <= 1 && abs_col <= 1;
    case 'Q':
        if (row_delta == 0 || col_delta == 0 || abs_row == abs_col) {
            int row_step = row_delta == 0 ? 0 : (row_delta > 0 ? 1 : -1);
            int col_step = col_delta == 0 ? 0 : (col_delta > 0 ? 1 : -1);
            for (int r = from.row + row_step, c = from.col + col_step; r != to.row || c != to.col; r += row_step, c += col_step) {
                if (board[r][c] != ".") {
                    return false;
                }
            }
            return true;
        }
        return false;
    case 'R':
        if (row_delta == 0 || col_delta == 0) {
            int row_step = row_delta == 0 ? 0 : (row_delta > 0 ? 1 : -1);
            int col_step = col_delta == 0 ? 0 : (col_delta > 0 ? 1 : -1);
            for (int r = from.row + row_step, c = from.col + col_step; r != to.row || c != to.col; r += row_step, c += col_step) {
                if (board[r][c] != ".") {
                    return false;
                }
            }
            return true;
        }
        return false;
    case 'B':
        if (abs_row == abs_col) {
            int row_step = row_delta > 0 ? 1 : -1;
            int col_step = col_delta > 0 ? 1 : -1;
            for (int r = from.row + row_step, c = from.col + col_step; r != to.row || c != to.col; r += row_step, c += col_step) {
                if (board[r][c] != ".") {
                    return false;
                }
            }
            return true;
        }
        return false;
    case 'N':
        return (abs_row == 2 && abs_col == 1) || (abs_row == 1 && abs_col == 2);
    case 'P':
        if (piece[0] == 'w') {
            // White pawns move upward: row decreases (toward row 0).
            if (col_delta != 0) {
                return false;
            }
            if (row_delta == -1 && target == ".") {
                return true;
            }
            // Double-step allowed only from white's start row (last row of the board).
            if (row_delta == -2 && from.row == static_cast<int>(board.size()) - 1 &&
                target == "." && board[from.row - 1][from.col] == ".") {
                return true;
            }
            return false;
        } else {
            // Black pawns move downward: row increases (toward last row).
            if (col_delta != 0) {
                return false;
            }
            if (row_delta == 1 && target == ".") {
                return true;
            }
            // Double-step allowed only from black's start row (first row of the board).
            if (row_delta == 2 && from.row == 0 &&
                target == "." && board[from.row + 1][from.col] == ".") {
                return true;
            }
            return false;
        }
    default:
        return false;
    }
}

Board parse_board(const std::vector<std::string>& rows) {
    Board board;
    for (const auto& row_text : rows) {
        std::vector<std::string> cells = split_ws(row_text);
        if (cells.empty()) {
            continue;
        }
        bool valid = true;
        for (const auto& cell : cells) {
            if (!is_valid_piece_token(cell)) {
                valid = false;
                break;
            }
        }
        if (valid) {
            board.push_back(cells);
        }
    }
    return board;
}

std::string render_board(const Board& board) {
    std::ostringstream out;
    for (size_t row = 0; row < board.size(); ++row) {
        for (size_t col = 0; col < board[row].size(); ++col) {
            if (col > 0) {
                out << ' ';
            }
            out << board[row][col];
        }
        if (row + 1 < board.size()) {
            out << '\n';
        }
    }
    return out.str();
}

bool board_has_king(const Board& board, char color) {
    for (const auto& row : board) {
        for (const auto& cell : row) {
            if (cell.size() == 2 && cell[0] == color && cell[1] == 'K') {
                return true;
            }
        }
    }
    return false;
}

class BoardGame {
public:
    explicit BoardGame(Board board) : board_(std::move(board)), game_over_(false) {}

    // Marks the piece at (x,y) as airborne for 1000 ms.
    void jump(int x, int y) {
        if (game_over_ || airborne_.has_value()) {
            return;
        }
        Position pos{y / 100, x / 100};
        if (!is_inside(pos, board_)) {
            return;
        }
        const std::string& token = board_[pos.row][pos.col];
        if (token == "." || pending_move_.has_value()) {
            // Empty cell or piece already moving — cannot jump.
            return;
        }
        airborne_ = JumpRequest{pos, token, current_time_ms_ + 1000};
    }

    void click(int x, int y) {
        if (game_over_ || pending_move_.has_value()) {
            return;
        }

        Position pos{y / 100, x / 100};
        if (!is_inside(pos, board_)) {
            return;
        }

        const std::string& token = board_[pos.row][pos.col];
        if (!selected_.has_value()) {
            if (token != ".") {
                selected_ = pos;
            }
            return;
        }

        // Clicking the same cell as selected → jump.
        if (pos.row == selected_->row && pos.col == selected_->col) {
            const std::string& piece_token = board_[pos.row][pos.col];
            // A moving piece (pending_move_) cannot jump — already guarded above.
            // A captured piece cannot jump — piece_token would be "." (guarded by selection).
            if (piece_token != "." && !airborne_.has_value()) {
                airborne_ = JumpRequest{pos, piece_token, current_time_ms_ + 1000};
            }
            selected_.reset();
            return;
        }

        if (token != ".") {
            const std::string& selected_token = board_[selected_->row][selected_->col];
            if (selected_token != "." && selected_token[0] == token[0]) {
                selected_ = pos;
                return;
            }
        }

        if (is_legal_move(board_, *selected_, pos, airborne_)) {
            pending_move_ = MoveRequest{*selected_, pos, current_time_ms_ + 1000};
            selected_.reset();
        }
    }

    void wait(int ms) {
        if (game_over_) {
            return;
        }
        current_time_ms_ += ms;

        // Resolve a pending regular move.
        if (pending_move_.has_value() && current_time_ms_ >= pending_move_->arrival_time_ms) {
            apply_pending_move();
        }

        // Resolve a pending jump.
        if (airborne_.has_value() && current_time_ms_ >= airborne_->land_time_ms) {
            apply_jump();
        }
    }

    void print_board(std::ostream& out) const {
        out << render_board(board_) << '\n';
    }

private:
    void apply_pending_move() {
        if (!pending_move_.has_value()) {
            return;
        }

        const auto& move = *pending_move_;
        if (!is_inside(move.from, board_) || !is_inside(move.to, board_)) {
            pending_move_.reset();
            return;
        }
        if (!is_legal_move(board_, move.from, move.to, airborne_)) {
            pending_move_.reset();
            return;
        }

        const std::string piece = board_[move.from.row][move.from.col];
        const std::string captured = board_[move.to.row][move.to.col];

        // If the destination is occupied by an airborne piece of the same color,
        // the move is blocked (the airborne piece is "there").
        if (airborne_.has_value() &&
            airborne_->cell.row == move.to.row &&
            airborne_->cell.col == move.to.col) {
            if (same_color(piece, airborne_->piece)) {
                // Friendly airborne piece — cannot land here.
                pending_move_.reset();
                return;
            }
            // Enemy moving piece arrives at airborne piece's cell:
            // the airborne piece captures it — remove the arriving piece, keep airborne.
            board_[move.from.row][move.from.col] = ".";
            pending_move_.reset();
            // Check if the arriving (captured) piece was a king.
            if (piece.size() == 2 && piece[1] == 'K') {
                game_over_ = true;
            }
            // Airborne piece successfully captured — jump resolves immediately.
            airborne_.reset();
            return;
        }

        board_[move.from.row][move.from.col] = ".";
        board_[move.to.row][move.to.col] = piece;
        pending_move_.reset();

        // Pawn promotion: a pawn reaching the last row becomes a queen.
        const bool is_white_pawn = (piece == "wP");
        const bool is_black_pawn = (piece == "bP");
        const int last_row = static_cast<int>(board_.size()) - 1;
        if ((is_white_pawn && move.to.row == 0) || (is_black_pawn && move.to.row == last_row)) {
            board_[move.to.row][move.to.col] = std::string(1, piece[0]) + "Q";
        }

        // Game over only if a king was actually captured.
        if (captured.size() == 2 && captured[1] == 'K') {
            game_over_ = true;
        }
    }

    // Called when the 1000ms jump window expires with no enemy arriving.
    // The piece lands on its original cell (it never left).
    void apply_jump() {
        if (!airborne_.has_value()) {
            return;
        }
        // The piece is already on its cell — nothing to move.
        // Just clear the airborne state.
        airborne_.reset();
    }

    Board board_;
    std::optional<Position> selected_;
    std::optional<MoveRequest> pending_move_;
    std::optional<JumpRequest> airborne_;
    int current_time_ms_ = 0;
    bool game_over_;
};

}  // namespace

int main() {
    std::vector<std::string> board_lines;
    std::vector<std::string> command_lines;
    bool reading_board = false;
    bool reading_commands = false;

    std::string line;
    while (std::getline(std::cin, line)) {
        std::string trimmed = line;
        trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        trimmed.erase(std::find_if(trimmed.rbegin(), trimmed.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), trimmed.end());
        if (trimmed.empty()) {
            continue;
        }
        if (trimmed == "Board:") {
            reading_board = true;
            reading_commands = false;
            continue;
        }
        if (trimmed == "Commands:") {
            reading_board = false;
            reading_commands = true;
            continue;
        }
        if (reading_board) {
            board_lines.push_back(trimmed);
        } else if (reading_commands) {
            command_lines.push_back(trimmed);
        }
    }

    Board board = parse_board(board_lines);
    BoardGame game(std::move(board));

    for (const std::string& command : command_lines) {
        std::istringstream ss(command);
        std::string verb;
        ss >> verb;
        if (verb == "click") {
            int x = 0;
            int y = 0;
            ss >> x >> y;
            game.click(x, y);
        } else if (verb == "jump") {
            int x = 0;
            int y = 0;
            ss >> x >> y;
            game.jump(x, y);
        } else if (verb == "wait") {
            int ms = 0;
            ss >> ms;
            game.wait(ms);
        } else if (verb == "print") {
            std::string target;
            ss >> target;
            if (target == "board") {
                game.print_board(std::cout);
            }
        }
    }

    return 0;
}
