//****************************************************************************//
//       Copyright (C) 2020 Florent Hivert <Florent.Hivert@lri.fr>,           //
//                                                                            //
//    Distributed under the terms of the GNU General Public License (GPL)     //
//                                                                            //
//    This code is distributed in the hope that it will be useful,            //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of          //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       //
//   General Public License for more details.                                 //
//                                                                            //
//  The full text of the GPL is available at:                                 //
//                                                                            //
//                  http://www.gnu.org/licenses/                              //
//****************************************************************************//

// A class for block diagram with zones such as (Generalized) Sudokus or
// Tectonics.

#include <iostream>
#include <string>
#include <vector>

class BlockDiagram {
public:
    BlockDiagram(size_t height, size_t width);

    size_t get_width() const { return width; }
    size_t get_height() const { return height; }
    const std::vector<int> &operator[](size_t r) const { return contents[r]; }
    std::vector<int> &operator[](size_t r) { return contents[r]; }
    int get_block(size_t r, size_t c) const { return blocks[r][c]; }
    int &get_block(size_t r, size_t c) { return blocks[r][c]; }

    std::string to_string(std::vector<std::vector<int>> fill) const;
    std::string to_string() const { return to_string(contents); };

    std::istream &read_blocks_char(std::istream &);
    std::istream &read_blocks_int(std::istream &);
    std::istream &read_contents_char(std::istream &);
    std::istream &read_contents_int(std::istream &);

    friend std::istream &operator>>(std::istream &, BlockDiagram &);
    friend std::ostream &operator<<(std::ostream &, const BlockDiagram &);

private:
    size_t height, width;
    std::vector<std::vector<int>> blocks, contents;
};
