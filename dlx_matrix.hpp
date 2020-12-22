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

// Implementation of Knuth dancing links backtrack algorithm
//////////////////////////////////////////////////////////////
#ifndef DLX_MATRIX_HPP_
#define DLX_MATRIX_HPP_

#include <algorithm>      // transform
#include <iostream>       // cout
#include <limits>         // numeric_limits
#include <string>         //
#include <tuple>          // tie, ignore
#include <type_traits>    // invoke_result_t
#include <unordered_map>  //
#include <vector>         //

#define DLX_INLINE __attribute__((always_inline)) inline

namespace DLX_backtrack {

namespace details {

template <typename A, typename Fun>
inline std::vector<std::invoke_result_t<Fun, A>> vector_transform(
    const std::vector<A> &v, Fun fun) {
  std::vector<std::invoke_result_t<Fun, A>> res;
  res.reserve(v.size());
  std::transform(v.cbegin(), v.cend(), std::back_inserter(res), fun);
  return res;
}

};

/////////////////
class DLXMatrix {
 public:
  using ind_t = std::size_t;

 private:
  struct Header;
  struct Node {
    ind_t row_id;
    Node *left, *right, *up, *down;
    Header *head;
  };

  struct Header {
    ind_t size;
    Node node;
    Header *left, *right;
  };

  ind_t nb_primary_, depth_;
  std::vector<Header> heads_;
  std::vector<std::vector<Node>> rows_;

  std::vector<Node *> work_;
  bool search_down_;

 public:
  using Vect1D = std::vector<ind_t>;
  using Vect2D = std::vector<Vect1D>;

  DLXMatrix() : DLXMatrix(0) {}
  explicit DLXMatrix(ind_t nb_col) : DLXMatrix(nb_col, nb_col) {}
  DLXMatrix(ind_t nb_col, ind_t nb_primary);
  DLXMatrix(ind_t nb_col, const Vect2D &rows)
      : DLXMatrix(nb_col, nb_col, rows) {}
  DLXMatrix(ind_t nb_col, ind_t nb_primary, const Vect2D &rows);
  DLXMatrix(const DLXMatrix &);
  DLXMatrix &operator=(const DLXMatrix &other);
  DLXMatrix(DLXMatrix &&) noexcept = default;
  DLXMatrix &operator=(DLXMatrix &&other) noexcept = default;
  ~DLXMatrix() = default;

  size_t nb_cols() const { return heads_.size() - 1; }
  size_t nb_rows() const { return rows_.size(); }
  size_t nb_primary() const { return nb_primary_; }

  void check_sizes() const;

  ind_t add_row(const Vect1D &r) { return add_row_sparse(r); }
  ind_t add_row_sparse(const Vect1D &r);
  ind_t add_row_dense(const std::vector<bool> &r);
  Vect1D ith_row_sparse(ind_t i) const;
  std::vector<bool> ith_row_dense(ind_t i) const;

  Vect1D row_to_sparse(const std::vector<bool> &row) const;
  std::vector<bool> row_to_dense(Vect1D row) const;

  ind_t choose(ind_t i);

  Vect2D search_rec(size_t max_sol = std::numeric_limits<size_t>::max());
  bool search_iter();
  bool search_iter(Vect1D &);
  Vect1D get_solution();
  bool search_random(Vect1D &);
  bool is_solution(const Vect1D &) const;

  bool is_row_active(ind_t i) const { return is_active(rows_.at(i).data()); }
  bool is_col_active(ind_t i) const { return is_active(&heads_.at(i + 1)); }

  void reset(size_t depth = 0);

  DLXMatrix permuted_columns(const Vect1D &perm) const;
  DLXMatrix permuted_inv_columns(const Vect1D &perm) const;
  DLXMatrix permuted_rows(const Vect1D &perm) const;

  std::string to_string() const;

  unsigned long int nb_choices, nb_dances;  // Computation statistics

 protected:
  Header *master() { return &heads_[0]; }
  const Header *master() const { return &heads_[0]; }

  ind_t get_row_id(const Node *n) const { return n->row_id; }
  ind_t get_col_id(const Header *h) const {
    // Ensure that the following - 1 wraps
    return static_cast<size_t>(std::distance(master(), h)) - 1;
  }
  bool is_primary(const Header *h) const { return get_col_id(h) < nb_primary_; }
  bool is_active(const Node *nd) const;
  bool is_active(const Header *h) const;

  Vect1D row_sparse(const std::vector<Node> &) const;
  std::vector<bool> row_dense(const std::vector<Node> &) const;

 private:

  Header *choose_min();
  DLX_INLINE void hide(Node *row);
  DLX_INLINE void unhide(Node *row);
  DLX_INLINE void cover(Header *col);
  DLX_INLINE void uncover(Header *col);
  DLX_INLINE void choose(Node *nd);
  DLX_INLINE void unchoose(Node *nd);
  void search_rec_internal(size_t, Vect2D &);
};

// Concept check
static_assert(std::is_move_constructible<DLXMatrix>::value,
              "DLXMatrix should be move constructible");
static_assert(std::is_move_assignable<DLXMatrix>::value,
              "DLXMatrix should bt move assignable");

/////////////////
template <typename Item, typename OptId, typename ItemHash = std::hash<Item>>
class DLXMatrixIdent : private DLXMatrix {
  std::vector<Item> items_;
  std::vector<OptId> optids_;
  std::unordered_map<Item, int, ItemHash> item_ind_;

 public:
  using DLXMatrix::Vect1D, DLXMatrix::Vect2D;
  using ind_t = DLXMatrix::ind_t;
  using Option = std::vector<Item>;
  using OptPair = std::pair<OptId, Option>;
  using OptPairs = std::vector<std::pair<OptId, Option>>;

  DLXMatrixIdent() : DLXMatrixIdent({}, 0) {}
  explicit DLXMatrixIdent(const Option &items)
     : DLXMatrixIdent(Option(items), items.size()) {}
  explicit DLXMatrixIdent(Option &&items)
      : DLXMatrixIdent(std::move(items), items.size()) {}
  DLXMatrixIdent(const Option &items, ind_t nb_primary)
      : DLXMatrixIdent(Option(items), nb_primary) {}
  DLXMatrixIdent(Option &&items, ind_t nb_primary)
      : DLXMatrix(items.size(), nb_primary),
        items_(std::move(items)),
        optids_(),
        item_ind_() {
    for (size_t i = 0; i < items_.size(); i++) {
      bool res;
      std::tie(std::ignore, res) = item_ind_.insert({items_[i], i});
      if (!res) throw std::runtime_error("DLXMatrixIdent : Duplicate item");
    }
  }
  // template <typename... Args>
  // DLXMatrixIdent(Args&&... args, const OptPairs &opts)
  //     : DLXMatrixIdent(std::forward<Args>(args)...) {
  //   for (const auto &[id, row] : opts) add_opt(id, row);
  // }
  DLXMatrixIdent(const Option &items, const OptPairs &opts)
      : DLXMatrixIdent(Option(items), items.size(), opts) {}
  DLXMatrixIdent(Option &&items, const OptPairs &opts)
      : DLXMatrixIdent(std::move(items), items.size(), opts) {}
  DLXMatrixIdent(const Option &items, ind_t nb_primary, const OptPairs &opts)
      : DLXMatrixIdent(Option(items), nb_primary, opts) {}
  DLXMatrixIdent(Option &&items, ind_t nb_primary, const OptPairs &opts)
      : DLXMatrixIdent(std::move(items), nb_primary) {
    for (const auto &[id, row] : opts) add_opt(id, row);
  }
  DLXMatrixIdent(const DLXMatrixIdent &) = default;
  DLXMatrixIdent &operator=(const DLXMatrixIdent &other) = default;
  DLXMatrixIdent(DLXMatrixIdent &&) noexcept = default;
  DLXMatrixIdent &operator=(DLXMatrixIdent &&other) noexcept = default;
  ~DLXMatrixIdent() = default;

  using DLXMatrix::nb_choices, DLXMatrix::nb_dances;
  size_t nb_items() const { return nb_cols(); }
  size_t nb_opts() const { return nb_rows(); }
  using DLXMatrix::check_sizes;
  using DLXMatrix::nb_primary;

  using DLXMatrix::reset;
  using DLXMatrix::to_string;

  ind_t add_opt(const OptId &optid, const Option &opt) {
    optids_.push_back(optid);
    return DLXMatrix::add_row_sparse(details::vector_transform(
        opt, [this](const Item &n) -> ind_t { return item_ind_.at(n); }));
  }
  using DLXMatrix::ith_row_sparse, DLXMatrix::ith_row_dense;
  Option ith_opt(ind_t i) const {
    return details::vector_transform(ith_row_sparse(i),
                                     [this](ind_t n) { return items_[n]; });
  }
  ind_t get_opt_ind(const OptId &opt) const {
    auto pos = std::find(optids_.cbegin(), optids_.cend(), opt);
    if (pos == optids_.cend())
      throw std::runtime_error("get_opt_ind: not found");
    return std::distance(optids_.cbegin(), pos);
  }

  ind_t choose(const OptId &opt) { return DLXMatrix::choose(get_opt_ind(opt)); }
  bool is_item_active(const Item &i) const {
    return is_row_active(items_[item_ind_.at(i)]);
  }
  bool is_opt_active(const OptId &i) const { return is_col_active(ith_opt(i)); }

  bool search_iter() { return DLXMatrix::search_iter(); }
  std::vector<OptId> get_solution() {
    return details::vector_transform(DLXMatrix::get_solution(),
                            [this](ind_t n) { return optids_[n]; });
  }

  bool is_solution(const std::vector<OptId> &sol) {
    return DLXMatrix::is_solution(details::vector_transform(
        sol, [this](const OptId &opt) { return get_opt_ind(opt); }));
  }
};

using DLXMatrixNamed = DLXMatrixIdent<std::string, std::string>;


///////////////////////////////////////////////////////////
// Various related functions
std::vector<DLXMatrix::ind_t> inverse_perm(
    const std::vector<DLXMatrix::ind_t> &perm);

inline std::ostream &operator<<(std::ostream &out,
                                const DLX_backtrack::DLXMatrix &M) {
  return out << M.to_string();
}

template <typename Item, typename OptId, typename ItemHash>
inline std::ostream &operator<<(
    std::ostream &out,
    const DLX_backtrack::DLXMatrixIdent<Item, OptId, ItemHash> &M) {
  return out << M.to_string();
}

}  // namespace DLX_backtrack

#endif  // DLX_MATRIX_HPP_
