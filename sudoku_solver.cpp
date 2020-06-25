/*
    A sudoku solver modeled as a constraint-satisfaction problem.

    Usage: ./sudoku_solver.exe <input_file_path>

    Input should be formatted in a 9x9 grid as a regular sudoku grid would look. Unknown spaces should be relaced with a period "."
    If an answer comes back same as the input, then either there is no answer or the input is formatted incorrectly.

    Brandon Luk
*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>

static const int board_nrows = 9;
static const int board_ncols = 9;


// Keep the top-left index of each "subgrid" in an array.
static const std::pair<int, int> subgrid_locs[9] = {{0,0}, {0,3}, {0,6},
                                                    {3,0}, {3,3}, {3,6},
                                                    {6,0}, {6,3}, {6,6}};

struct Cell
{
    // Initially a few cells will be given as part of the puzzle. As we deduce what unkown cells must be we will also mark them as given.
    bool given = false;
    int value = 0;
    std::vector<int> possible = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    void set_not_possible(int n)
    {
        auto found = std::find(possible.begin(), possible.end(), n);
        if(found != possible.end())
            possible.erase(found);
    }

    bool is_possible(int n)
    {
        auto found = std::find(possible.begin(), possible.end(), n);
        return found == possible.end() ? false : true;
    }
};

typedef std::vector<std::vector<Cell>> Board;

// Load the puzzle 
Board load_file(std::string filename)
{
    std::ifstream file(filename, std::ios::in);
    if(!file)
    {
        std::cout << "Error loading file..." << std::endl;
        exit(1);
    }

    Board board = Board(board_nrows);
    for(std::vector<Cell> &row : board)
    {
        row = std::vector<Cell>(board_ncols);
    }

    char c;
    for(int row = 0; row < board_nrows; ++row)
    {
        for(int col = 0; col < board_ncols; ++col)
        {
            while((c = file.get()) == '\n' || c == '\r');
            switch(c)
            {
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    board.at(row).at(col) = Cell{true, static_cast<int>(c - '0')};
                    break;

                case EOF:
                    std::cout << "File not formatted properly..." << std::endl;
                    exit(2);

                default:
                    board.at(row).at(col) = Cell{false};
                    break;
            }
        }
    }

    file.close();

    return board;
}

// Isolate and return a row
std::vector<Cell*> get_row(Board &board, int row)
{
    std::vector<Cell*> cells;
    for(int col = 0; col < board_ncols; ++col)
    {
        cells.push_back(&board[row][col]);
    }
    return cells;
}

// Isolate and return a column
std::vector<Cell*> get_col(Board &board, int col)
{
    std::vector<Cell*> cells;
    for(int row = 0; row < board_nrows; ++row)
    {
        cells.push_back(&board[row][col]);
    }
    return cells;
}

// Isolate and return a subgrid.
// Subgrids are numbered left-to-right, top-to-bottom.
std::vector<Cell*> get_subgrid(Board &board, int subgrid)
{
    std::vector<Cell*> cells;
    int cur_row, cur_col;
    for(int row_offset = 0; row_offset <= 2; ++row_offset)
    {
        for(int col_offset = 0; col_offset <= 2; ++col_offset)
        {
            cur_row = subgrid_locs[subgrid].first + row_offset;
            cur_col = subgrid_locs[subgrid].second + col_offset;

            cells.push_back(&board[cur_row][cur_col]);
        }
    }
    return cells;
}

// Find which subgrid a coordinate is a part of
int find_subgrid(std::pair<int, int> coord)
{
    int row_offset, col_offset;
    if(coord.first < 3)
        row_offset = 0;
    else if(coord.first >= 3 && coord.first < 6)
        row_offset = 1;
    else
        row_offset = 2;

    if(coord.second < 3)
        col_offset = 0;
    else if(coord.second >= 3 && coord.second < 6)
        col_offset = 1;
    else
        col_offset = 2;

    return (row_offset * 3) + col_offset;
}

// Return which numbers are marked as given
std::vector<int> get_given_from_vec(std::vector<Cell*> &cells)
{
    std::vector<int> given;
    for(Cell *c : cells)
    {
        if(c->given)
        {
            given.push_back(c->value);
        }
    }
    return given;
}

// Check to see if the puzzle is solved
bool is_solved(Board &board)
{
    std::vector<Cell*> cells;
    std::vector<int> given;

    for(int row = 0; row < board_nrows; ++row)
    {
        cells = get_row(board, row);
        given = get_given_from_vec(cells);
        std::sort(given.begin(), given.end());
        auto last = std::unique(given.begin(), given.end());
        given.erase(last, given.end());
        if(given.size() != 9)
            return false;
    }
    for(int col = 0; col < board_ncols; ++col)
    {
        cells = get_col(board, col);
        given = get_given_from_vec(cells);
        std::sort(given.begin(), given.end());
        auto last = std::unique(given.begin(), given.end());
        given.erase(last, given.end());
        if(given.size() != 9)
            return false;
    }
    for(int subgrid = 0; subgrid < 9; ++subgrid)
    {
        cells = get_subgrid(board, subgrid);
        given = get_given_from_vec(cells);
        std::sort(given.begin(), given.end());
        auto last = std::unique(given.begin(), given.end());
        given.erase(last, given.end());
        if(given.size() != 9)
            return false;
    }
    return true;
}

void print(Board &board)
{
    static const int divider_one = 3;
    static const int divider_two = 6;

    Cell *cell;
    int vertical_counter = 0;
    int horizontal_counter = 0;

    std::cout << std::endl;
    for(int row = 0; row < board_nrows; ++row)
    {
        horizontal_counter = 0;

        for(int col = 0; col < board_ncols; ++col)
        {
            cell = &board.at(row).at(col);
            if(cell->given)
            {
                std::cout << cell->value << " ";
            }
            else
            {
                std::cout << "* ";
            }
            
            horizontal_counter++;
            if(horizontal_counter == divider_one || horizontal_counter == divider_two)
            {
                std::cout << "| ";
            }

        }

        vertical_counter++;
        if(vertical_counter == divider_one || vertical_counter == divider_two)
        {
            std::cout << std::endl;
            for(int i = 0; i < board_ncols + 2; ++i)
            {
                std::cout << "--";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// For debugging. Print the possibilities of a cell.
void print_possibilities(Cell* cell)
{
    std::cout << std::endl;
    for(int &g : cell->possible)
    {
        std::cout << g << ", ";
    }
    std::cout << "\n\n";
}

// Assign possible values an unknown cell could be with respect to their row
void assign_possibilities_for_rows(Board &board)
{
    for(int row = 0; row < board_nrows; ++row)
    {
        std::vector<Cell*> cells_in_row = get_row(board, row);
        std::vector<int> given_values = get_given_from_vec(cells_in_row);

        for(Cell *c : cells_in_row)
        {
            if(!c->given)
            {
                for(int &g : given_values)
                {
                    c->set_not_possible(g);
                }
            }
        }
    }
}

// Assign possible values an unknown cell could be with respect to their column
void assign_possibilities_for_cols(Board &board)
{
    for(int col = 0; col < board_ncols; ++col)
    {
        std::vector<Cell*> cells_in_col = get_col(board, col);
        std::vector<int> given_values = get_given_from_vec(cells_in_col);

        
        for(Cell *c : cells_in_col)
        {
            if(!c->given)
            {
                for(int &g : given_values)
                {
                    c->set_not_possible(g);
                }
            }
        }
    }
}

// Assign possible values an unknown cell could be with respect to their subgrid
void assign_possibilities_for_subgrids(Board &board)
{
    for(int i = 0; i < 9; ++i)
    {
        std::vector<Cell*> cells_in_subgrid = get_subgrid(board, i);
        std::vector<int> given_values = get_given_from_vec(cells_in_subgrid);

        for(Cell *c : cells_in_subgrid)
        {
            if(!c->given)
            {
                for(int &g : given_values)
                {
                    c->set_not_possible(g);
                }
            }
        }
    }
}

void assign_possibilities(Board &board)
{
    assign_possibilities_for_cols(board);
    assign_possibilities_for_rows(board);
    assign_possibilities_for_subgrids(board);
}

// Mark a cell as a number, then go through the row, column, and subgrid that the cell is a part of and remove its
// value from the possibilities of the other cells.
void mark(Board &board, std::pair<int, int> coord, int n)
{
    board[coord.first][coord.second].given = true;
    board[coord.first][coord.second].value = n;

    std::vector<Cell*> to_set = get_row(board, coord.first);
    for(Cell *c : to_set)
    {
        if(!c->given)
            c->set_not_possible(n);
    }

    to_set = get_col(board, coord.second);
    for(Cell *c : to_set)
    {
        if(!c->given)
            c->set_not_possible(n);
    }

    to_set = get_subgrid(board, find_subgrid(coord));
    for(Cell *c : to_set)
    {
        if(!c->given)
            c->set_not_possible(n);
    }
}

void add_to_map(std::map<int, std::vector<std::pair<int, int>>>& p_map, int g, std::pair<int, int> coord)
{
    if(!p_map.contains(g))
    {
        p_map[g] = std::vector<std::pair<int, int>>{coord};
    }
    else
    {
        p_map[g].push_back(coord);
    }
}

// Scan each row to find if there is a number that can only possibly be in a single cell, and mark that call as that number
bool scan_rows_for_lone_possibility(Board &board)
{
    bool at_least_one_mark = false;

    for(int row = 0; row < board_nrows; ++row)
    {
        std::map<int, std::vector<std::pair<int, int>>> p_map;
        for(int col = 0; col < board_ncols; ++col)
        {
            if(!board[row][col].given)
            {
                for(int &g : board[row][col].possible)
                {
                    add_to_map(p_map, g, {row, col});
                }
            }
        }

        // Check the entries in the possibility map for a possibility that exists only in one cell of the row
        for(auto it = p_map.begin(); it != p_map.end(); ++it)
        {
            // Found one
            if(it->second.size() == 1)
            {
                mark(board, it->second.front(), it->first);
                at_least_one_mark = true;
            }
        }
    }

    return at_least_one_mark;
}

// Scan each column to find if there is a number that can only possibly be in a single cell, and mark that call as that number
bool scan_cols_for_lone_possibility(Board &board)
{
    bool at_least_one_mark = false;

    for(int col = 0; col < board_ncols; ++col)
    {
        std::map<int, std::vector<std::pair<int, int>>> p_map;
        for(int row = 0; row < board_nrows; ++row)
        {
            if(!board[row][col].given)
            {
                for(int &g : board[row][col].possible)
                {
                    add_to_map(p_map, g, {row, col});
                }
            }
        }

        // Check the entries in the possibility map for a possibility that exists only in one cell of the row
        for(auto it = p_map.begin(); it != p_map.end(); ++it)
        {
            // Found one
            if(it->second.size() == 1)
            {
                mark(board, it->second.front(), it->first);
                at_least_one_mark = true;
            }
        }
    }

    return at_least_one_mark;
}

// Scan each subgrid to find if there is a number that can only possibly be in a single cell, and mark that call as that number
bool scan_subgrids_for_lone_possibility(Board &board)
{
    bool at_least_one_mark = false;

    for(int subgrid = 0; subgrid < 9; ++subgrid)
    {
        int cur_row, cur_col;
        std::map<int, std::vector<std::pair<int, int>>> p_map;
        for(int row_offset = 0; row_offset <= 2; ++row_offset)
        {
            for(int col_offset = 0; col_offset <= 2; ++col_offset)
            {
                cur_row = subgrid_locs[subgrid].first + row_offset;
                cur_col = subgrid_locs[subgrid].second + col_offset;

                if(!board[cur_row][cur_col].given)
                {
                    for(int &g : board[cur_row][cur_col].possible)
                    {
                        add_to_map(p_map, g, {cur_row, cur_col});
                    }
                }
            }
        }

        // Check the entries in the possibility map for a possibility that exists only in one cell of the row
        for(auto it = p_map.begin(); it != p_map.end(); ++it)
        {
            // Found one
            if(it->second.size() == 1)
            {
                mark(board, it->second.front(), it->first);
                at_least_one_mark = true;
            }
        }
    }

    return at_least_one_mark;
}

void scan(Board &board)
{
    while(scan_rows_for_lone_possibility(board) ||
            scan_cols_for_lone_possibility(board) ||
            scan_subgrids_for_lone_possibility(board)){}  
}

// Find the cell that has the fewest number of possibilities
std::pair<int, int> find_fewest_possibility_cell_index(Board &board)
{
    std::pair<int, int> index;
    int least_so_far = 10;

    for(int row = 0; row < board_nrows; ++row)
    {
        for(int col = 0; col < board_ncols; ++col)
        {
            if(!board.at(row).at(col).given && board.at(row).at(col).possible.size() < least_so_far)
            {
                index = {row, col};
                least_so_far = board.at(row).at(col).possible.size();
            }
        }
    }
    return index;
}

Board solve(Board board)
{
    bool solved = false;
    Board solved_board;

    assign_possibilities(board);
    scan(board);

    std::function<void(Board)> solve_r = [&solve_r, &solved, &solved_board](Board b){
        if(is_solved(b))
        {
            solved = true;
            solved_board = b;
            return;
        }
        std::pair<int, int> target = find_fewest_possibility_cell_index(b);
        std::vector<int> target_possibilities = b.at(target.first).at(target.second).possible;
        auto it = target_possibilities.begin();

        while(it != target_possibilities.end() && !solved)
        {
            Board temp_board = b;
            mark(temp_board, target, *it);
            scan(temp_board);
            solve_r(temp_board);
            ++it;
        }

    };

    solve_r(board);
    return solved_board;
}

int main(int argc, char **args)
{
    Board board = load_file(std::string(args[1]));
    std::cout << "Input:" << std::endl;
    print(board);

    Board solved = solve(board);
    std::cout << "Solution:" << std::endl;
    print(solved);

    exit(0);
}