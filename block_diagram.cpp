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
#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#ifndef DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_DISABLE
#endif
#endif

#include "doctest/doctest.h"

#include "block_diagram.hpp"

#include <iostream>
#include <sstream>
#include <vector>

//////////////////////////////////////////////////////
TEST_SUITE_BEGIN("[block_diagram]class BlockDiagram");
//////////////////////////////////////////////////////

std::istream &read_matrix_int(std::istream &in,
                              std::vector<std::vector<int>> &mat) {
    for (auto &r : mat) {
        for (auto &v : r)
            in >> v;
    }
    return in;
}
std::istream &read_matrix_char(std::istream &in,
                               std::vector<std::vector<int>> &mat) {
    for (auto &r : mat) {
        for (auto &v : r) {
            char c;
            in >> c;
            v = c == '.' ? 0 : c;
        }
    }
    return in;
}

BlockDiagram::BlockDiagram(size_t h, size_t w)
    : height(h), width(w), blocks(h, std::vector<int>(w)),
      contents(h, std::vector<int>(w)) {}
TEST_CASE("BlockDiagram(size_t, size_t)") {
    BlockDiagram Blk(5, 4);
    CHECK(Blk.get_height() == 5);
    CHECK(Blk.get_width() == 4);
}

std::istream &BlockDiagram::read_contents_int(std::istream &in) {
    return read_matrix_int(in, contents);
}
TEST_CASE("read_contents_int") {
    BlockDiagram Blk(2, 3);
    std::istringstream in{"3 25 7\n 12 8 9\n"};
    CHECK(Blk.read_contents_int(in));
    CHECK(Blk[0][0] == 3);
    CHECK(Blk[0][1] == 25);
    CHECK(Blk[0][2] == 7);
    CHECK(Blk[1][1] == 8);
}

std::istream &BlockDiagram::read_contents_char(std::istream &in) {
    return read_matrix_char(in, contents);
}
TEST_CASE("read_contents_char") {
    BlockDiagram Blk(2, 3);
    std::istringstream in{"a c .\n . 8 9\n"};
    CHECK(Blk.read_contents_char(in));
    CHECK(Blk[0][0] == 'a');
    CHECK(Blk[0][1] == 'c');
    CHECK(Blk[0][2] == '\0');
    CHECK(Blk[1][0] == '\0');
    CHECK(Blk[1][1] == '8');
}

std::istream &BlockDiagram::read_blocks_int(std::istream &in) {
    return read_matrix_int(in, blocks);
}
TEST_CASE("read_blocks_int") {
    SUBCASE("Block 2x3") {
        BlockDiagram Blk(2, 3);
        std::istringstream in{"3 25 7\n 12 8 9\n"};
        CHECK(Blk.read_blocks_int(in));
        CHECK(Blk.get_block(0, 0) == 3);
        CHECK(Blk.get_block(0, 1) == 25);
        CHECK(Blk.get_block(0, 2) == 7);
        CHECK(Blk.get_block(1, 1) == 8);
    }
    SUBCASE("Failed read 4x3 too short") {
        BlockDiagram Blk(4, 3);
        std::istringstream inbl{"1 2 1 3 4 2"};
        CHECK_FALSE(Blk.read_blocks_int(inbl));
    }
    SUBCASE("Failed read 2x2 bad char") {
        BlockDiagram Blk(2, 2);
        std::istringstream inbl{"1 2 a 3"};
        CHECK_FALSE(Blk.read_blocks_int(inbl));
    }
}

std::istream &BlockDiagram::read_blocks_char(std::istream &in) {
    return read_matrix_char(in, blocks);
}
TEST_CASE("read_blocks_char") {
    SUBCASE("Block 2x3") {
        BlockDiagram Blk(2, 3);
        std::istringstream in{"a c b\n e 8 9\n"};
        CHECK(Blk.read_blocks_char(in));
        CHECK(Blk.get_block(0, 0) == 'a');
        CHECK(Blk.get_block(0, 1) == 'c');
        CHECK(Blk.get_block(0, 2) == 'b');
        CHECK(Blk.get_block(1, 1) == '8');
    }
    SUBCASE("Failed read 4x3 too short") {
        BlockDiagram Blk(4, 3);
        std::istringstream inbl{"a a b a c b"};
        CHECK_FALSE(Blk.read_blocks_char(inbl));
    }
}

// std::istream &operator>>(std::istream &in, BlockDiagram &Blk) { return in; }
// std::ostream &operator<<(std::ostream &out, const BlockDiagram &Blk) {
//     return out;
// }

std::string BlockDiagram::to_string(std::vector<std::vector<int>> fill) const {
    std::string res = "+";
    for (size_t c = 0; c < width; ++c)
        res += "---+";
    res += "\n";
    for (size_t r = 0; r < height; ++r) {
        res += "|";
        for (size_t c = 0; c < width; ++c) {
            if (fill[r][c] == 0) {
                res += "   ";
            } else {
                std::string cont = std::to_string(fill[r][c]);
                if (cont.size() < 3)
                    cont.insert(0, " ");
                if (cont.size() < 3)
                    cont += " ";
                res += cont;
            }
            res +=
                c + 1 < width && blocks[r][c] == blocks[r][c + 1] ? " " : "|";
        }
        res += "\n+";
        for (size_t c = 0; c < width; ++c) {
            res += r + 1 < height && blocks[r][c] == blocks[r + 1][c] ? "   +"
                                                                      : "---+";
        }
        res += "\n";
    }
    return res;
};
TEST_CASE("to_string()") {
    SUBCASE("Standar 3x3 sudoku") {
        BlockDiagram Blk(9, 9);
        std::istringstream inbl{"aaabbbccc aaabbbccc aaabbbccc"
                                "eeefffggg eeefffggg eeefffggg"
                                "hhhiiijjj hhhiiijjj hhhiiijjj"};
        REQUIRE(Blk.read_blocks_char(inbl));
        CHECK(Blk.to_string() == "+---+---+---+---+---+---+---+---+---+\n"
                                 "|           |           |           |\n"
                                 "+   +   +   +   +   +   +   +   +   +\n"
                                 "|           |           |           |\n"
                                 "+   +   +   +   +   +   +   +   +   +\n"
                                 "|           |           |           |\n"
                                 "+---+---+---+---+---+---+---+---+---+\n"
                                 "|           |           |           |\n"
                                 "+   +   +   +   +   +   +   +   +   +\n"
                                 "|           |           |           |\n"
                                 "+   +   +   +   +   +   +   +   +   +\n"
                                 "|           |           |           |\n"
                                 "+---+---+---+---+---+---+---+---+---+\n"
                                 "|           |           |           |\n"
                                 "+   +   +   +   +   +   +   +   +   +\n"
                                 "|           |           |           |\n"
                                 "+   +   +   +   +   +   +   +   +   +\n"
                                 "|           |           |           |\n"
                                 "+---+---+---+---+---+---+---+---+---+\n");
    }
    SUBCASE("Block 2x3") {
        BlockDiagram Blk(2, 3);
        std::istringstream inbl{"a a b\n a c b\n"};
        REQUIRE(Blk.read_blocks_char(inbl));
        CHECK(Blk.to_string() == "+---+---+---+\n"
                                 "|       |   |\n"
                                 "+   +---+   +\n"
                                 "|   |   |   |\n"
                                 "+---+---+---+\n");
        Blk[1][0] = 2;
        Blk[0][2] = 4;
        CHECK(Blk.to_string() == "+---+---+---+\n"
                                 "|       | 4 |\n"
                                 "+   +---+   +\n"
                                 "| 2 |   |   |\n"
                                 "+---+---+---+\n");
    }
    SUBCASE("Block 4x3") {
        BlockDiagram Blk(4, 3);
        std::istringstream inbl{"aab acb ccc cdd"};
        REQUIRE(Blk.read_blocks_char(inbl));
        CHECK(Blk.to_string() == "+---+---+---+\n"
                                 "|       |   |\n"
                                 "+   +---+   +\n"
                                 "|   |   |   |\n"
                                 "+---+   +---+\n"
                                 "|           |\n"
                                 "+   +---+---+\n"
                                 "|   |       |\n"
                                 "+---+---+---+\n");
        std::istringstream inval{"1 2 0  0 3 0  0 0 4  5 1 2"};
        REQUIRE(Blk.read_contents_int(inval));
        CHECK(Blk.to_string() == "+---+---+---+\n"
                                 "| 1   2 |   |\n"
                                 "+   +---+   +\n"
                                 "|   | 3 |   |\n"
                                 "+---+   +---+\n"
                                 "|         4 |\n"
                                 "+   +---+---+\n"
                                 "| 5 | 1   2 |\n"
                                 "+---+---+---+\n");
    }
    SUBCASE("Block 4x3 : big numbers") {
        BlockDiagram Blk(4, 3);
        std::istringstream inbl{"aab acb ccc cdd"};
        REQUIRE(Blk.read_blocks_char(inbl));
        std::istringstream inval{"1 12 0  0 33 0  0 0 4  125 1 2"};
        REQUIRE(Blk.read_contents_int(inval));
        CHECK(Blk.to_string() == "+---+---+---+\n"
                                 "| 1   12|   |\n"
                                 "+   +---+   +\n"
                                 "|   | 33|   |\n"
                                 "+---+   +---+\n"
                                 "|         4 |\n"
                                 "+   +---+---+\n"
                                 "|125| 1   2 |\n"
                                 "+---+---+---+\n");
    }
}
