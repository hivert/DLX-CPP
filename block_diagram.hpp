//****************************************************************************//
//       Copyright (C) 2020 Florent Hivert <Florent.Hivert@lri.fr>,           //
//                                                                            //
//    Distributed under the terms of the GNU General Public License (GPL)     //
//                                                                            //
//    This code is distributed in the hope that it will be useful,            //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of          //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       //
//    General Public License for more details.                                //
//                                                                            //
//    The full text of the GPL is available at:                               //
//                                                                            //
//                  http://www.gnu.org/licenses/                              //
//****************************************************************************//

// A class for block diagram with zones such as (Generalized) Sudokus or
// Tectonics.

#ifndef BLOCK_DIAGRAM_HPP_
#define BLOCK_DIAGRAM_HPP_

#include <iostream>
#include <string>
#include <vector>

class BlockDiagram {
 public:
  BlockDiagram(size_t h, size_t w);

  size_t nb_cols() const { return nb_cols_; }
  size_t nb_rows() const { return nb_rows_; }
  const std::vector<int> &operator[](size_t r) const { return contents_[r]; }
  std::vector<int> &operator[](size_t r) { return contents_[r]; }
  int get_block(size_t r, size_t c) const { return blocks_[r][c]; }
  int &get_block(size_t r, size_t c) { return blocks_[r][c]; }

  std::string to_string(std::vector<std::vector<int>> fill) const;
  std::string to_string() const { return to_string(contents_); }

  std::istream &read_blocks_char(std::istream &);
  std::istream &read_blocks_int(std::istream &);
  std::istream &read_contents_char(std::istream &);
  std::istream &read_contents_int(std::istream &);

  friend std::istream &operator>>(std::istream &, BlockDiagram &);
  friend std::ostream &operator<<(std::ostream &, const BlockDiagram &);

 private:
  size_t nb_rows_, nb_cols_;
  std::vector<std::vector<int>> blocks_, contents_;
};

#endif  // BLOCK_DIAGRAM_HPP_
