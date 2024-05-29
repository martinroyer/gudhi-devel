/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2022-24 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

/**
 * @file naive_vector_column.h
 * @author Hannah Schreiber
 * @brief Contains the @ref Naive_vector_column class.
 * Also defines the std::hash method for @ref Naive_vector_column.
 */

#ifndef PM_NAIVE_VECTOR_COLUMN_H
#define PM_NAIVE_VECTOR_COLUMN_H

#include <vector>
#include <stdexcept>
#include <type_traits>
#include <algorithm>    //binary_search
#include <utility>      //std::swap, std::move & std::exchange

#include <boost/iterator/indirect_iterator.hpp>

#include <gudhi/Persistence_matrix/allocators/cell_constructors.h>

namespace Gudhi {
namespace persistence_matrix {

/**
 * @class Naive_vector_column naive_vector_column.h gudhi/Persistence_matrix/columns/naive_vector_column.h
 * @ingroup persistence_matrix
 *
 * @brief Column class following the @ref PersistenceMatrixColumn concept.
 *
 * Column based on a vector structure. The cells are always ordered by row index and only non-zero values
 * are stored uniquely in the underlying container.
 * 
 * @tparam Master_matrix An instanciation of @ref Matrix from which all types and options are deduced.
 * @tparam Cell_constructor Factory of @ref Cell classes.
 */
template <class Master_matrix>
class Naive_vector_column : public Master_matrix::Row_access_option,
                            public Master_matrix::Column_dimension_option,
                            public Master_matrix::Chain_column_option 
{
 public:
  using Master = Master_matrix;
  using index = typename Master_matrix::index;
  using id_index = typename Master_matrix::id_index;
  using dimension_type = typename Master_matrix::dimension_type;
  using Field_element_type = typename Master_matrix::element_type;
  using Cell = typename Master_matrix::Cell_type;
  using Column_settings = typename Master_matrix::Column_settings;

 private:
  using Field_operators = typename Master_matrix::Field_operators;
  using Column_type = std::vector<Cell*>;
  using Cell_constructor = typename Master_matrix::Cell_constructor;

 public:
  using iterator = boost::indirect_iterator<typename Column_type::iterator>;
  using const_iterator = boost::indirect_iterator<typename Column_type::const_iterator>;
  using reverse_iterator = boost::indirect_iterator<typename Column_type::reverse_iterator>;
  using const_reverse_iterator = boost::indirect_iterator<typename Column_type::const_reverse_iterator>;

  Naive_vector_column(Column_settings* colSettings = nullptr);
  template <class Container_type = typename Master_matrix::boundary_type>
  Naive_vector_column(const Container_type& nonZeroRowIndices, 
                      Column_settings* colSettings);
  template <class Container_type = typename Master_matrix::boundary_type, class Row_container_type>
  Naive_vector_column(index columnIndex, 
                      const Container_type& nonZeroRowIndices, 
                      Row_container_type* rowContainer,
                      Column_settings* colSettings);
  template <class Container_type = typename Master_matrix::boundary_type>
  Naive_vector_column(const Container_type& nonZeroChainRowIndices, 
                      dimension_type dimension,
                      Column_settings* colSettings);
  template <class Container_type = typename Master_matrix::boundary_type, class Row_container_type>
  Naive_vector_column(index columnIndex, 
                      const Container_type& nonZeroChainRowIndices, 
                      dimension_type dimension,
                      Row_container_type* rowContainer, 
                      Column_settings* colSettings);
  Naive_vector_column(const Naive_vector_column& column, 
                      Column_settings* colSettings = nullptr);
  template <class Row_container_type>
  Naive_vector_column(const Naive_vector_column& column, 
                      index columnIndex, 
                      Row_container_type* rowContainer,
                      Column_settings* colSettings = nullptr);
  Naive_vector_column(Naive_vector_column&& column) noexcept;
  ~Naive_vector_column();

  std::vector<Field_element_type> get_content(int columnLength = -1) const;
  bool is_non_zero(id_index rowIndex) const;
  bool is_empty() const;
  std::size_t size() const;

  template <class Map_type>
  void reorder(const Map_type& valueMap, [[maybe_unused]] index columnIndex = -1);
  void clear();
  void clear(id_index rowIndex);

  id_index get_pivot() const;
  Field_element_type get_pivot_value() const;

  iterator begin() noexcept;
  const_iterator begin() const noexcept;
  iterator end() noexcept;
  const_iterator end() const noexcept;
  reverse_iterator rbegin() noexcept;
  const_reverse_iterator rbegin() const noexcept;
  reverse_iterator rend() noexcept;
  const_reverse_iterator rend() const noexcept;

  template <class Cell_range>
  Naive_vector_column& operator+=(const Cell_range& column);
  Naive_vector_column& operator+=(Naive_vector_column& column);

  Naive_vector_column& operator*=(unsigned int v);

  // this = v * this + column
  template <class Cell_range>
  Naive_vector_column& multiply_target_and_add(const Field_element_type& val, const Cell_range& column);
  Naive_vector_column& multiply_target_and_add(const Field_element_type& val, Naive_vector_column& column);
  // this = this + column * v
  template <class Cell_range>
  Naive_vector_column& multiply_source_and_add(const Cell_range& column, const Field_element_type& val);
  Naive_vector_column& multiply_source_and_add(Naive_vector_column& column, const Field_element_type& val);

  friend bool operator==(const Naive_vector_column& c1, const Naive_vector_column& c2) {
    if (&c1 == &c2) return true;
    if (c1.column_.size() != c2.column_.size()) return false;

    auto it1 = c1.column_.begin();
    auto it2 = c2.column_.begin();
    while (it1 != c1.column_.end() && it2 != c2.column_.end()) {
      if constexpr (Master_matrix::Option_list::is_z2) {
        if ((*it1)->get_row_index() != (*it2)->get_row_index()) return false;
      } else {
        if ((*it1)->get_row_index() != (*it2)->get_row_index() || (*it1)->get_element() != (*it2)->get_element())
          return false;
      }
      ++it1;
      ++it2;
    }
    return true;
  }
  friend bool operator<(const Naive_vector_column& c1, const Naive_vector_column& c2) {
    if (&c1 == &c2) return false;

    auto it1 = c1.column_.begin();
    auto it2 = c2.column_.begin();
    while (it1 != c1.column_.end() && it2 != c2.column_.end()) {
      if ((*it1)->get_row_index() != (*it2)->get_row_index()) return (*it1)->get_row_index() < (*it2)->get_row_index();
      if constexpr (!Master_matrix::Option_list::is_z2) {
        if ((*it1)->get_element() != (*it2)->get_element()) return (*it1)->get_element() < (*it2)->get_element();
      }
      ++it1;
      ++it2;
    }
    return it2 != c2.column_.end();
  }

  // void set_operators(Field_operators* operators){ operators_ = operators; }

  // Disabled with row access.
  Naive_vector_column& operator=(const Naive_vector_column& other);

  friend void swap(Naive_vector_column& col1, Naive_vector_column& col2) {
    swap(static_cast<typename Master_matrix::Row_access_option&>(col1),
         static_cast<typename Master_matrix::Row_access_option&>(col2));
    swap(static_cast<typename Master_matrix::Column_dimension_option&>(col1),
         static_cast<typename Master_matrix::Column_dimension_option&>(col2));
    swap(static_cast<typename Master_matrix::Chain_column_option&>(col1),
         static_cast<typename Master_matrix::Chain_column_option&>(col2));
    col1.column_.swap(col2.column_);
    std::swap(col1.operators_, col2.operators_);
    std::swap(col1.cellPool_, col2.cellPool_);
  }

 private:
  using ra_opt = typename Master_matrix::Row_access_option;
  using dim_opt = typename Master_matrix::Column_dimension_option;
  using chain_opt = typename Master_matrix::Chain_column_option;

  Column_type column_;
  Field_operators* operators_;
  Cell_constructor* cellPool_;

  void _delete_cell(Cell* cell);
  Cell* _insert_cell(const Field_element_type& value, id_index rowIndex, Column_type& column);
  void _insert_cell(id_index rowIndex, Column_type& column);
  void _update_cell(const Field_element_type& value, id_index rowIndex, index position);
  void _update_cell(id_index rowIndex, index position);
  template <class Cell_range>
  bool _add(const Cell_range& column);
  template <class Cell_range>
  bool _multiply_target_and_add(const Field_element_type& val, const Cell_range& column);
  template <class Cell_range>
  bool _multiply_source_and_add(const Cell_range& column, const Field_element_type& val);

};

template <class Master_matrix>
inline Naive_vector_column<Master_matrix>::Naive_vector_column(Column_settings* colSettings)
    : ra_opt(), dim_opt(), chain_opt(), operators_(nullptr), cellPool_(colSettings == nullptr ? nullptr : &(colSettings->cellConstructor))
{
  if (operators_ == nullptr && cellPool_ == nullptr) return;  //to allow default constructor which gives a dummy column
  if constexpr (!Master_matrix::Option_list::is_z2){
    operators_ = &(colSettings->operators);
  }
}

template <class Master_matrix>
template <class Container_type>
inline Naive_vector_column<Master_matrix>::Naive_vector_column(
    const Container_type& nonZeroRowIndices, Column_settings* colSettings)
    : ra_opt(),
      dim_opt(nonZeroRowIndices.size() == 0 ? 0 : nonZeroRowIndices.size() - 1),
      chain_opt(),
      column_(nonZeroRowIndices.size(), nullptr),
      operators_(nullptr),
      cellPool_(&(colSettings->cellConstructor))
{
  static_assert(!Master_matrix::isNonBasic || Master_matrix::Option_list::is_of_boundary_type,
                "Constructor not available for chain columns, please specify the dimension of the chain.");

  if constexpr (!Master_matrix::Option_list::is_z2){
    operators_ = &(colSettings->operators);
  }

  index i = 0;
  if constexpr (Master_matrix::Option_list::is_z2) {
    for (id_index id : nonZeroRowIndices) {
      _update_cell(id, i++);
    }
  } else {
    for (const auto& p : nonZeroRowIndices) {
      _update_cell(operators_->get_value(p.second), p.first, i++);
    }
  }
}

template <class Master_matrix>
template <class Container_type, class Row_container_type>
inline Naive_vector_column<Master_matrix>::Naive_vector_column(
    index columnIndex, 
    const Container_type& nonZeroRowIndices, 
    Row_container_type* rowContainer,
    Column_settings* colSettings)
    : ra_opt(columnIndex, rowContainer),
      dim_opt(nonZeroRowIndices.size() == 0 ? 0 : nonZeroRowIndices.size() - 1),
      chain_opt([&] {
        if constexpr (Master_matrix::Option_list::is_z2) {
          return nonZeroRowIndices.begin() == nonZeroRowIndices.end() ? -1 : *std::prev(nonZeroRowIndices.end());
        } else {
          return nonZeroRowIndices.begin() == nonZeroRowIndices.end() ? -1 : std::prev(nonZeroRowIndices.end())->first;
        }
      }()),
      column_(nonZeroRowIndices.size(), nullptr),
      operators_(nullptr),
      cellPool_(&(colSettings->cellConstructor))
{
  static_assert(!Master_matrix::isNonBasic || Master_matrix::Option_list::is_of_boundary_type,
                "Constructor not available for chain columns, please specify the dimension of the chain.");

  if constexpr (!Master_matrix::Option_list::is_z2){
    operators_ = &(colSettings->operators);
  }

  index i = 0;
  if constexpr (Master_matrix::Option_list::is_z2) {
    for (id_index id : nonZeroRowIndices) {
      _update_cell(id, i++);
    }
  } else {
    for (const auto& p : nonZeroRowIndices) {
      _update_cell(operators_->get_value(p.second), p.first, i++);
    }
  }
}

template <class Master_matrix>
template <class Container_type>
inline Naive_vector_column<Master_matrix>::Naive_vector_column(
    const Container_type& nonZeroRowIndices, 
    dimension_type dimension, 
    Column_settings* colSettings)
    : ra_opt(),
      dim_opt(dimension),
      chain_opt([&] {
        if constexpr (Master_matrix::Option_list::is_z2) {
          return nonZeroRowIndices.begin() == nonZeroRowIndices.end() ? -1 : *std::prev(nonZeroRowIndices.end());
        } else {
          return nonZeroRowIndices.begin() == nonZeroRowIndices.end() ? -1 : std::prev(nonZeroRowIndices.end())->first;
        }
      }()),
      column_(nonZeroRowIndices.size(), nullptr),
      operators_(nullptr),
      cellPool_(&(colSettings->cellConstructor))
{
  if constexpr (!Master_matrix::Option_list::is_z2){
    operators_ = &(colSettings->operators);
  }

  index i = 0;
  if constexpr (Master_matrix::Option_list::is_z2) {
    for (id_index id : nonZeroRowIndices) {
      _update_cell(id, i++);
    }
  } else {
    for (const auto& p : nonZeroRowIndices) {
      _update_cell(operators_->get_value(p.second), p.first, i++);
    }
  }
}

template <class Master_matrix>
template <class Container_type, class Row_container_type>
inline Naive_vector_column<Master_matrix>::Naive_vector_column(
    index columnIndex, 
    const Container_type& nonZeroRowIndices, 
    dimension_type dimension,
    Row_container_type* rowContainer, 
    Column_settings* colSettings)
    : ra_opt(columnIndex, rowContainer),
      dim_opt(dimension),
      chain_opt([&] {
        if constexpr (Master_matrix::Option_list::is_z2) {
          return nonZeroRowIndices.begin() == nonZeroRowIndices.end() ? -1 : *std::prev(nonZeroRowIndices.end());
        } else {
          return nonZeroRowIndices.begin() == nonZeroRowIndices.end() ? -1 : std::prev(nonZeroRowIndices.end())->first;
        }
      }()),
      column_(nonZeroRowIndices.size(), nullptr),
      operators_(nullptr),
      cellPool_(&(colSettings->cellConstructor))
{
  if constexpr (!Master_matrix::Option_list::is_z2){
    operators_ = &(colSettings->operators);
  }

  index i = 0;
  if constexpr (Master_matrix::Option_list::is_z2) {
    for (id_index id : nonZeroRowIndices) {
      _update_cell(id, i++);
    }
  } else {
    for (const auto& p : nonZeroRowIndices) {
      _update_cell(operators_->get_value(p.second), p.first, i++);
    }
  }
}

template <class Master_matrix>
inline Naive_vector_column<Master_matrix>::Naive_vector_column(const Naive_vector_column& column,
                                                                                 Column_settings* colSettings)
    : ra_opt(),
      dim_opt(static_cast<const dim_opt&>(column)),
      chain_opt(static_cast<const chain_opt&>(column)),
      column_(column.column_.size(), nullptr),
      operators_(colSettings == nullptr ? column.operators_ : nullptr),
      cellPool_(colSettings == nullptr ? column.cellPool_ : &(colSettings->cellConstructor))
{
  static_assert(!Master_matrix::Option_list::has_row_access,
                "Simple copy constructor not available when row access option enabled. Please specify the new column "
                "index and the row container.");

  if constexpr (!Master_matrix::Option_list::is_z2){
    if (colSettings != nullptr) operators_ = &(colSettings->operators);
  }

  index i = 0;
  for (const Cell* cell : column.column_) {
    if constexpr (Master_matrix::Option_list::is_z2) {
      _update_cell(cell->get_row_index(), i++);
    } else {
      _update_cell(cell->get_element(), cell->get_row_index(), i++);
    }
  }
}

template <class Master_matrix>
template <class Row_container_type>
inline Naive_vector_column<Master_matrix>::Naive_vector_column(const Naive_vector_column& column,
                                                                                 index columnIndex,
                                                                                 Row_container_type* rowContainer,
                                                                                 Column_settings* colSettings)
    : ra_opt(columnIndex, rowContainer),
      dim_opt(static_cast<const dim_opt&>(column)),
      chain_opt(static_cast<const chain_opt&>(column)),
      column_(column.column_.size(), nullptr),
      operators_(colSettings == nullptr ? column.operators_ : nullptr),
      cellPool_(colSettings == nullptr ? column.cellPool_ : &(colSettings->cellConstructor))
{
  if constexpr (!Master_matrix::Option_list::is_z2){
    if (colSettings != nullptr) operators_ = &(colSettings->operators);
  }

  index i = 0;
  for (const Cell* cell : column.column_) {
    if constexpr (Master_matrix::Option_list::is_z2) {
      _update_cell(cell->get_row_index(), i++);
    } else {
      _update_cell(cell->get_element(), cell->get_row_index(), i++);
    }
  }
}

template <class Master_matrix>
inline Naive_vector_column<Master_matrix>::Naive_vector_column(Naive_vector_column&& column) noexcept
    : ra_opt(std::move(static_cast<ra_opt&>(column))),
      dim_opt(std::move(static_cast<dim_opt&>(column))),
      chain_opt(std::move(static_cast<chain_opt&>(column))),
      column_(std::move(column.column_)),
      operators_(std::exchange(column.operators_, nullptr)),
      cellPool_(std::exchange(column.cellPool_, nullptr)) 
{}

template <class Master_matrix>
inline Naive_vector_column<Master_matrix>::~Naive_vector_column() 
{
  for (auto* cell : column_) {
    _delete_cell(cell);
  }
}

template <class Master_matrix>
inline std::vector<typename Naive_vector_column<Master_matrix>::Field_element_type>
Naive_vector_column<Master_matrix>::get_content(int columnLength) const 
{
  if (columnLength < 0 && column_.size() > 0)
    columnLength = column_.back()->get_row_index() + 1;
  else if (columnLength < 0)
    return std::vector<Field_element_type>();

  std::vector<Field_element_type> container(columnLength, 0);
  for (auto it = column_.begin(); it != column_.end() && (*it)->get_row_index() < static_cast<id_index>(columnLength);
       ++it) {
    if constexpr (Master_matrix::Option_list::is_z2) {
      container[(*it)->get_row_index()] = 1;
    } else {
      container[(*it)->get_row_index()] = (*it)->get_element();
    }
  }
  return container;
}

template <class Master_matrix>
inline bool Naive_vector_column<Master_matrix>::is_non_zero(id_index rowIndex) const 
{
  // cell gets destroyed with the pool at the end, but I don't know if that's a good solution
  // in particular because there is a chance of using another factory later depending on the options.
  return std::binary_search(column_.begin(), column_.end(), cellPool_->construct(rowIndex),
                            [](const Cell* a, const Cell* b) { return a->get_row_index() < b->get_row_index(); });
}

template <class Master_matrix>
inline bool Naive_vector_column<Master_matrix>::is_empty() const 
{
  return column_.empty();
}

template <class Master_matrix>
inline std::size_t Naive_vector_column<Master_matrix>::size() const 
{
  return column_.size();
}

template <class Master_matrix>
template <class Map_type>
inline void Naive_vector_column<Master_matrix>::reorder(const Map_type& valueMap,
                                                                          [[maybe_unused]] index columnIndex) 
{
  static_assert(!Master_matrix::isNonBasic || Master_matrix::Option_list::is_of_boundary_type,
                "Method not available for chain columns.");

  for (Cell* cell : column_) {
    if constexpr (Master_matrix::Option_list::has_row_access) {
      ra_opt::unlink(cell);
      if (columnIndex != static_cast<index>(-1)) cell->set_column_index(columnIndex);
    }
    cell->set_row_index(valueMap.at(cell->get_row_index()));
    if constexpr (Master_matrix::Option_list::has_intrusive_rows && Master_matrix::Option_list::has_row_access)
      ra_opt::insert_cell(cell->get_row_index(), cell);
  }

  // all cells have to be deleted first, to avoid problem with insertion when row is a set
  if constexpr (!Master_matrix::Option_list::has_intrusive_rows && Master_matrix::Option_list::has_row_access) {
    for (Cell* cell : column_) {
      ra_opt::insert_cell(cell->get_row_index(), cell);
    }
  }

  std::sort(column_.begin(), column_.end(), [](const Cell* c1, const Cell* c2) { return *c1 < *c2; });
}

template <class Master_matrix>
inline void Naive_vector_column<Master_matrix>::clear() 
{
  static_assert(!Master_matrix::isNonBasic || Master_matrix::Option_list::is_of_boundary_type,
                "Method not available for chain columns as a base element should not be empty.");

  for (auto* cell : column_) {
    if constexpr (Master_matrix::Option_list::has_row_access) ra_opt::unlink(cell);
    cellPool_->destroy(cell);
  }

  column_.clear();
}

template <class Master_matrix>
inline void Naive_vector_column<Master_matrix>::clear(id_index rowIndex) 
{
  static_assert(!Master_matrix::isNonBasic || Master_matrix::Option_list::is_of_boundary_type,
                "Method not available for chain columns.");

  auto it = column_.begin();
  while (it != column_.end() && (*it)->get_row_index() != rowIndex) ++it;
  if (it != column_.end()) {
    _delete_cell(*it);
    column_.erase(it);
  }
}

template <class Master_matrix>
inline typename Naive_vector_column<Master_matrix>::id_index
Naive_vector_column<Master_matrix>::get_pivot() const 
{
  static_assert(Master_matrix::isNonBasic,
                "Method not available for base columns.");  // could technically be, but is the notion usefull then?

  if constexpr (Master_matrix::Option_list::is_of_boundary_type) {
    return column_.empty() ? -1 : column_.back()->get_row_index();
  } else {
    return chain_opt::get_pivot();
  }
}

template <class Master_matrix>
inline typename Naive_vector_column<Master_matrix>::Field_element_type
Naive_vector_column<Master_matrix>::get_pivot_value() const 
{
  static_assert(Master_matrix::isNonBasic,
                "Method not available for base columns.");  // could technically be, but is the notion usefull then?

  if constexpr (Master_matrix::Option_list::is_z2) {
    return 1;
  } else {
    if constexpr (Master_matrix::Option_list::is_of_boundary_type) {
      return column_.empty() ? Field_element_type() : column_.back()->get_element();
    } else {
      if (chain_opt::get_pivot() == static_cast<id_index>(-1)) return Field_element_type();
      for (const Cell* cell : column_) {
        if (cell->get_row_index() == chain_opt::get_pivot()) return cell->get_element();
      }
      return Field_element_type();  // should never happen if chain column is used properly
    }
  }
}

template <class Master_matrix>
inline typename Naive_vector_column<Master_matrix>::iterator
Naive_vector_column<Master_matrix>::begin() noexcept 
{
  return column_.begin();
}

template <class Master_matrix>
inline typename Naive_vector_column<Master_matrix>::const_iterator
Naive_vector_column<Master_matrix>::begin() const noexcept 
{
  return column_.begin();
}

template <class Master_matrix>
inline typename Naive_vector_column<Master_matrix>::iterator
Naive_vector_column<Master_matrix>::end() noexcept 
{
  return column_.end();
}

template <class Master_matrix>
inline typename Naive_vector_column<Master_matrix>::const_iterator
Naive_vector_column<Master_matrix>::end() const noexcept 
{
  return column_.end();
}

template <class Master_matrix>
inline typename Naive_vector_column<Master_matrix>::reverse_iterator
Naive_vector_column<Master_matrix>::rbegin() noexcept 
{
  return column_.rbegin();
}

template <class Master_matrix>
inline typename Naive_vector_column<Master_matrix>::const_reverse_iterator
Naive_vector_column<Master_matrix>::rbegin() const noexcept 
{
  return column_.rbegin();
}

template <class Master_matrix>
inline typename Naive_vector_column<Master_matrix>::reverse_iterator
Naive_vector_column<Master_matrix>::rend() noexcept 
{
  return column_.rend();
}

template <class Master_matrix>
inline typename Naive_vector_column<Master_matrix>::const_reverse_iterator
Naive_vector_column<Master_matrix>::rend() const noexcept 
{
  return column_.rend();
}

template <class Master_matrix>
template <class Cell_range>
inline Naive_vector_column<Master_matrix>&
Naive_vector_column<Master_matrix>::operator+=(const Cell_range& column) 
{
  static_assert((!Master_matrix::isNonBasic || std::is_same_v<Cell_range, Naive_vector_column>),
                "For boundary columns, the range has to be a column of same type to help ensure the validity of the "
                "base element.");  // could be removed, if we give the responsability to the user.
  static_assert((!Master_matrix::isNonBasic || Master_matrix::Option_list::is_of_boundary_type),
                "For chain columns, the given column cannot be constant.");

  _add(column);

  return *this;
}

template <class Master_matrix>
inline Naive_vector_column<Master_matrix>&
Naive_vector_column<Master_matrix>::operator+=(Naive_vector_column& column) 
{
  if constexpr (Master_matrix::isNonBasic && !Master_matrix::Option_list::is_of_boundary_type) {
    // assumes that the addition never zeros out this column.
    if (_add(column)) {
      chain_opt::swap_pivots(column);
      dim_opt::swap_dimension(column);
    }
  } else {
    _add(column);
  }

  return *this;
}

template <class Master_matrix>
inline Naive_vector_column<Master_matrix>&
Naive_vector_column<Master_matrix>::operator*=(unsigned int v) 
{
  if constexpr (Master_matrix::Option_list::is_z2) {
    if (v % 2 == 0) {
      if constexpr (Master_matrix::isNonBasic && !Master_matrix::Option_list::is_of_boundary_type) {
        throw std::invalid_argument("A chain column should not be multiplied by 0.");
      } else {
        clear();
      }
    }
  } else {
    Field_element_type val = operators_->get_value(v);

    if (val == Field_operators::get_additive_identity()) {
      if constexpr (Master_matrix::isNonBasic && !Master_matrix::Option_list::is_of_boundary_type) {
        throw std::invalid_argument("A chain column should not be multiplied by 0.");
      } else {
        clear();
      }
      return *this;
    }

    if (val == Field_operators::get_multiplicative_identity()) return *this;

    for (Cell* cell : column_) {
      operators_->multiply_inplace(cell->get_element(), val);
      if constexpr (Master_matrix::Option_list::has_row_access) ra_opt::update_cell(*cell);
    }
  }

  return *this;
}

template <class Master_matrix>
template <class Cell_range>
inline Naive_vector_column<Master_matrix>&
Naive_vector_column<Master_matrix>::multiply_target_and_add(const Field_element_type& val,
                                                                       const Cell_range& column) 
{
  static_assert((!Master_matrix::isNonBasic || std::is_same_v<Cell_range, Naive_vector_column>),
                "For boundary columns, the range has to be a column of same type to help ensure the validity of the "
                "base element.");  // could be removed, if we give the responsability to the user.
  static_assert((!Master_matrix::isNonBasic || Master_matrix::Option_list::is_of_boundary_type),
                "For chain columns, the given column cannot be constant.");

  if constexpr (Master_matrix::Option_list::is_z2) {
    if (val) {
      _add(column);
    } else {
      clear();
      _add(column);
    }
  } else {
    _multiply_target_and_add(val, column);
  }

  return *this;
}

template <class Master_matrix>
inline Naive_vector_column<Master_matrix>&
Naive_vector_column<Master_matrix>::multiply_target_and_add(const Field_element_type& val,
                                                                       Naive_vector_column& column) 
{
  if constexpr (Master_matrix::isNonBasic && !Master_matrix::Option_list::is_of_boundary_type) {
    // assumes that the addition never zeros out this column.
    if constexpr (Master_matrix::Option_list::is_z2) {
      if (val) {
        if (_add(column)) {
          chain_opt::swap_pivots(column);
          dim_opt::swap_dimension(column);
        }
      } else {
        throw std::invalid_argument("A chain column should not be multiplied by 0.");
      }
    } else {
      if (_multiply_target_and_add(val, column)) {
        chain_opt::swap_pivots(column);
        dim_opt::swap_dimension(column);
      }
    }
  } else {
    if constexpr (Master_matrix::Option_list::is_z2) {
      if (val) {
        _add(column);
      } else {
        clear();
        _add(column);
      }
    } else {
      _multiply_target_and_add(val, column);
    }
  }

  return *this;
}

template <class Master_matrix>
template <class Cell_range>
inline Naive_vector_column<Master_matrix>&
Naive_vector_column<Master_matrix>::multiply_source_and_add(const Cell_range& column,
                                                                       const Field_element_type& val) 
{
  static_assert((!Master_matrix::isNonBasic || std::is_same_v<Cell_range, Naive_vector_column>),
                "For boundary columns, the range has to be a column of same type to help ensure the validity of the "
                "base element.");  // could be removed, if we give the responsability to the user.
  static_assert((!Master_matrix::isNonBasic || Master_matrix::Option_list::is_of_boundary_type),
                "For chain columns, the given column cannot be constant.");

  if constexpr (Master_matrix::Option_list::is_z2) {
    if (val) {
      _add(column);
    }
  } else {
    _multiply_source_and_add(column, val);
  }

  return *this;
}

template <class Master_matrix>
inline Naive_vector_column<Master_matrix>&
Naive_vector_column<Master_matrix>::multiply_source_and_add(Naive_vector_column& column,
                                                                       const Field_element_type& val) 
{
  if constexpr (Master_matrix::isNonBasic && !Master_matrix::Option_list::is_of_boundary_type) {
    // assumes that the addition never zeros out this column.
    if constexpr (Master_matrix::Option_list::is_z2) {
      if (val) {
        if (_add(column)) {
          chain_opt::swap_pivots(column);
          dim_opt::swap_dimension(column);
        }
      }
    } else {
      if (_multiply_source_and_add(column, val)) {
        chain_opt::swap_pivots(column);
        dim_opt::swap_dimension(column);
      }
    }
  } else {
    if constexpr (Master_matrix::Option_list::is_z2) {
      if (val) {
        _add(column);
      }
    } else {
      _multiply_source_and_add(column, val);
    }
  }

  return *this;
}

template <class Master_matrix>
inline Naive_vector_column<Master_matrix>&
Naive_vector_column<Master_matrix>::operator=(const Naive_vector_column& other) 
{
  static_assert(!Master_matrix::Option_list::has_row_access, "= assignement not enabled with row access option.");

  dim_opt::operator=(other);
  chain_opt::operator=(other);

  auto tmpPool = cellPool_;
  cellPool_ = other.cellPool_;

  while (column_.size() > other.column_.size()) {
    if (column_.back() == nullptr) {
      if constexpr (Master_matrix::Option_list::has_row_access) ra_opt::unlink(column_.back());
      tmpPool->destroy(column_.back());
    }
    column_.pop_back();
  }

  column_.resize(other.column_.size(), nullptr);
  index i = 0;
  for (const Cell* cell : other.column_) {
    if (column_[i] != nullptr) {
      if constexpr (Master_matrix::Option_list::has_row_access) ra_opt::unlink(column_[i]);
      tmpPool->destroy(column_[i]);
    }
    if constexpr (Master_matrix::Option_list::is_z2) {
      _update_cell(cell->get_row_index(), i++);
    } else {
      _update_cell(cell->get_element(), cell->get_row_index(), i++);
    }
  }

  operators_ = other.operators_;

  return *this;
}

template <class Master_matrix>
inline void Naive_vector_column<Master_matrix>::_delete_cell(Cell* cell) 
{
  if constexpr (Master_matrix::Option_list::has_row_access) ra_opt::unlink(cell);
  cellPool_->destroy(cell);
}

template <class Master_matrix>
inline typename Naive_vector_column<Master_matrix>::Cell* Naive_vector_column<Master_matrix>::_insert_cell(
    const Field_element_type& value, id_index rowIndex, Column_type& column)
{
  if constexpr (Master_matrix::Option_list::has_row_access) {
    Cell* newCell = cellPool_->construct(ra_opt::columnIndex_, rowIndex);
    newCell->set_element(value);
    column.push_back(newCell);
    ra_opt::insert_cell(rowIndex, newCell);
    return newCell;
  } else {
    Cell* newCell = cellPool_->construct(rowIndex);
    column.push_back(newCell);
    newCell->set_element(value);
    return newCell;
  }
}

template <class Master_matrix>
inline void Naive_vector_column<Master_matrix>::_insert_cell(id_index rowIndex, Column_type& column) 
{
  if constexpr (Master_matrix::Option_list::has_row_access) {
    Cell* newCell = cellPool_->construct(ra_opt::columnIndex_, rowIndex);
    column.push_back(newCell);
    ra_opt::insert_cell(rowIndex, newCell);
  } else {
    column.push_back(cellPool_->construct(rowIndex));
  }
}

template <class Master_matrix>
inline void Naive_vector_column<Master_matrix>::_update_cell(const Field_element_type& value,
                                                                               id_index rowIndex, 
                                                                               index position) 
{
  if constexpr (Master_matrix::Option_list::has_row_access) {
    Cell* newCell = cellPool_->construct(ra_opt::columnIndex_, rowIndex);
    newCell->set_element(value);
    column_[position] = newCell;
    ra_opt::insert_cell(rowIndex, newCell);
  } else {
    column_[position] = cellPool_->construct(rowIndex);
    column_[position]->set_element(value);
  }
}

template <class Master_matrix>
inline void Naive_vector_column<Master_matrix>::_update_cell(id_index rowIndex, index position) 
{
  if constexpr (Master_matrix::Option_list::has_row_access) {
    Cell* newCell = cellPool_->construct(ra_opt::columnIndex_, rowIndex);
    column_[position] = newCell;
    ra_opt::insert_cell(rowIndex, newCell);
  } else {
    column_[position] = cellPool_->construct(rowIndex);
  }
}

template <class Master_matrix>
template <class Cell_range>
inline bool Naive_vector_column<Master_matrix>::_add(const Cell_range& column) 
{
  if (column.begin() == column.end()) return false;
  if (column_.empty()) {  // chain should never enter here.
    column_.resize(column.size());
    index i = 0;
    for (const Cell& cell : column) {
      if constexpr (Master_matrix::Option_list::is_z2) {
        _update_cell(cell.get_row_index(), i++);
      } else {
        _update_cell(cell.get_element(), cell.get_row_index(), i++);
      }
    }
    return true;
  }

  auto itTarget = column_.begin();
  auto itSource = column.begin();
  bool pivotIsZeroed = false;
  Column_type newColumn;
  newColumn.reserve(column_.size() + column.size());  // safe upper bound

  while (itTarget != column_.end() && itSource != column.end()) {
    Cell* cellTarget = *itTarget;
    const Cell& cellSource = *itSource;
    if (cellTarget->get_row_index() < cellSource.get_row_index()) {
      newColumn.push_back(cellTarget);
      ++itTarget;
    } else if (cellTarget->get_row_index() > cellSource.get_row_index()) {
      if constexpr (Master_matrix::Option_list::is_z2) {
        _insert_cell(cellSource.get_row_index(), newColumn);
      } else {
        _insert_cell(cellSource.get_element(), cellSource.get_row_index(), newColumn);
      }
      ++itSource;
    } else {
      if constexpr (Master_matrix::Option_list::is_z2) {
        if constexpr (Master_matrix::isNonBasic && !Master_matrix::Option_list::is_of_boundary_type) {
          if (cellTarget->get_row_index() == chain_opt::get_pivot()) pivotIsZeroed = true;
        }
        _delete_cell(cellTarget);
      } else {
        operators_->add_inplace(cellTarget->get_element(), cellSource.get_element());
        if (cellTarget->get_element() == Field_operators::get_additive_identity()) {
          if constexpr (Master_matrix::isNonBasic && !Master_matrix::Option_list::is_of_boundary_type) {
            if (cellTarget->get_row_index() == chain_opt::get_pivot()) pivotIsZeroed = true;
          }
          _delete_cell(cellTarget);
        } else {
          newColumn.push_back(cellTarget);
          if constexpr (Master_matrix::Option_list::has_row_access) ra_opt::update_cell(**itTarget);
        }
      }
      ++itTarget;
      ++itSource;
    }
  }

  while (itSource != column.end()) {
    if constexpr (Master_matrix::Option_list::is_z2) {
      _insert_cell(itSource->get_row_index(), newColumn);
    } else {
      _insert_cell(itSource->get_element(), itSource->get_row_index(), newColumn);
    }
    ++itSource;
  }

  while (itTarget != column_.end()) {
    newColumn.push_back(*itTarget);
    itTarget++;
  }

  column_.swap(newColumn);

  return pivotIsZeroed;
}

template <class Master_matrix>
template <class Cell_range>
inline bool Naive_vector_column<Master_matrix>::_multiply_target_and_add(const Field_element_type& val,
                                                                                    const Cell_range& column) 
{
  bool pivotIsZeroed = false;

  if (val == 0u) {
    if constexpr (Master_matrix::isNonBasic && !Master_matrix::Option_list::is_of_boundary_type) {
      throw std::invalid_argument("A chain column should not be multiplied by 0.");
      // this would not only mess up the base, but also the pivots stored.
    } else {
      clear();
    }
  }
  if (column_.empty()) {  // chain should never enter here.
    column_.resize(column.size());
    index i = 0;
    for (const Cell& cell : column) {
      if constexpr (Master_matrix::Option_list::is_z2) {
        _update_cell(cell.get_row_index(), i++);
      } else {
        _update_cell(cell.get_element(), cell.get_row_index(), i++);
      }
    }
    return true;
  }

  Column_type newColumn;
  newColumn.reserve(column_.size() + column.size());  // safe upper bound

  auto itTarget = column_.begin();
  auto itSource = column.begin();
  while (itTarget != column_.end() && itSource != column.end()) {
    Cell* cellTarget = *itTarget;
    const Cell& cellSource = *itSource;
    if (cellTarget->get_row_index() < cellSource.get_row_index()) {
      operators_->multiply_inplace(cellTarget->get_element(), val);
      if constexpr (Master_matrix::Option_list::has_row_access) ra_opt::update_cell(**itTarget);
      newColumn.push_back(cellTarget);
      ++itTarget;
    } else if (cellTarget->get_row_index() > cellSource.get_row_index()) {
      _insert_cell(cellSource.get_element(), cellSource.get_row_index(), newColumn);
      ++itSource;
    } else {
      operators_->multiply_and_add_inplace_front(cellTarget->get_element(), val, cellSource.get_element());
      if (cellTarget->get_element() == Field_operators::get_additive_identity()) {
        if constexpr (Master_matrix::isNonBasic && !Master_matrix::Option_list::is_of_boundary_type) {
          if (cellTarget->get_row_index() == chain_opt::get_pivot()) pivotIsZeroed = true;
        }
        _delete_cell(cellTarget);
      } else {
        if constexpr (Master_matrix::Option_list::has_row_access) ra_opt::update_cell(**itTarget);
        newColumn.push_back(cellTarget);
      }
      ++itTarget;
      ++itSource;
    }
  }

  while (itTarget != column_.end()) {
    operators_->multiply_inplace((*itTarget)->get_element(), val);
    if constexpr (Master_matrix::Option_list::has_row_access) ra_opt::update_cell(**itTarget);
    newColumn.push_back(*itTarget);
    itTarget++;
  }

  while (itSource != column.end()) {
    _insert_cell(itSource->get_element(), itSource->get_row_index(), newColumn);
    ++itSource;
  }

  column_.swap(newColumn);

  return pivotIsZeroed;
}

template <class Master_matrix>
template <class Cell_range>
inline bool Naive_vector_column<Master_matrix>::_multiply_source_and_add(const Cell_range& column,
                                                                                    const Field_element_type& val) 
{
  if (val == 0u || column.begin() == column.end()) {
    return false;
  }

  bool pivotIsZeroed = false;
  Column_type newColumn;
  newColumn.reserve(column_.size() + column.size());  // safe upper bound

  auto itTarget = column_.begin();
  auto itSource = column.begin();
  while (itTarget != column_.end() && itSource != column.end()) {
    Cell* cellTarget = *itTarget;
    const Cell& cellSource = *itSource;
    if (cellTarget->get_row_index() < cellSource.get_row_index()) {
      newColumn.push_back(cellTarget);
      ++itTarget;
    } else if (cellTarget->get_row_index() > cellSource.get_row_index()) {
      Cell* newCell = _insert_cell(cellSource.get_element(), cellSource.get_row_index(), newColumn);
      operators_->multiply_inplace(newCell->get_element(), val);
      ++itSource;
    } else {
      operators_->multiply_and_add_inplace_back(cellSource.get_element(), val, cellTarget->get_element());
      if (cellTarget->get_element() == Field_operators::get_additive_identity()) {
        if constexpr (Master_matrix::isNonBasic && !Master_matrix::Option_list::is_of_boundary_type) {
          if (cellTarget->get_row_index() == chain_opt::get_pivot()) pivotIsZeroed = true;
        }
        _delete_cell(cellTarget);
      } else {
        if constexpr (Master_matrix::Option_list::has_row_access) ra_opt::update_cell(**itTarget);
        newColumn.push_back(cellTarget);
      }
      ++itTarget;
      ++itSource;
    }
  }

  while (itSource != column.end()) {
    Cell* newCell = _insert_cell(itSource->get_element(), itSource->get_row_index(), newColumn);
    operators_->multiply_inplace(newCell->get_element(), val);
    ++itSource;
  }

  while (itTarget != column_.end()) {
    newColumn.push_back(*itTarget);
    ++itTarget;
  }

  column_.swap(newColumn);

  return pivotIsZeroed;
}

}  // namespace persistence_matrix
}  // namespace Gudhi

/**
 * @ingroup persistence_matrix
 *
 * @brief Hash method for @ref Gudhi::persistence_matrix::Naive_vector_column.
 * 
 * @tparam Master_matrix Template parameter of @ref Gudhi::persistence_matrix::Naive_vector_column.
 * @tparam Cell_constructor Template parameter of @ref Gudhi::persistence_matrix::Naive_vector_column.
 */
template <class Master_matrix>
struct std::hash<Gudhi::persistence_matrix::Naive_vector_column<Master_matrix> > 
{
  size_t operator()(
      const Gudhi::persistence_matrix::Naive_vector_column<Master_matrix>& column) const {
    std::size_t seed = 0;
    for (const auto& cell : column) {
      seed ^= std::hash<unsigned int>()(cell.get_row_index() * static_cast<unsigned int>(cell.get_element())) +
              0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};

#endif  // PM_NAIVE_VECTOR_COLUMN_H
