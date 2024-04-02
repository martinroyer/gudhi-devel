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
 * @file base_matrix_with_column_compression.h
 * @author Hannah Schreiber
 * @brief Contains the @ref Base_matrix_with_column_compression class.
 */

#ifndef PM_BASE_MATRIX_COMPRESSION_H
#define PM_BASE_MATRIX_COMPRESSION_H

#include <iostream>   //print() only
#include <vector>
#include <set>
#include <utility>    //std::swap, std::move & std::exchange

#include <boost/intrusive/set.hpp>
#include <boost/pending/disjoint_sets.hpp>

#include <gudhi/Simple_object_pool.h>

namespace Gudhi {
namespace persistence_matrix {

/**
 * @brief A base matrix (see @ref Base_matrix), but with column compression. That is, all identical columns in
 * the matrix are compressed together as the same column. For matrices with a lot of redundant columns, this will
 * save a lot of space. Also, any addition made onto a column will be performed at the same time on all other
 * identical columns, which is an advantage for the cohomology algorithm for example.
 * 
 * @tparam Master_matrix An instanciation of @ref Matrix from which all types and options are deduced.
 */
template <class Master_matrix>
class Base_matrix_with_column_compression : protected Master_matrix::Matrix_row_access_option 
{
 public:
  using index = typename Master_matrix::index;                        /**< Container index type. */
  using dimension_type = typename Master_matrix::dimension_type;      /**< Dimension value type. */
  /**
   * @brief Field operators class. Necessary only if @ref PersistenceMatrixOptions::is_z2 is false.
   */
  using Field_operators = typename Master_matrix::Field_operators;
  using Field_element_type = typename Master_matrix::element_type;    /**< Field element value type. */
  using Row_type = typename Master_matrix::Row_type;                  /**< Row type,
                                                                           only necessary with row access option. */
  using Cell_constructor = typename Master_matrix::Cell_constructor;  /**< Factory of @ref Cell classes. */

  /**
   * @brief Type for columns. Only one for each "column class" is explicitely constructed.
   */
  class Column_type
      : public Master_matrix::Column_type,
        public boost::intrusive::set_base_hook<boost::intrusive::link_mode<boost::intrusive::normal_link> > 
  {
   public:
    using Base = typename Master_matrix::Column_type;

    // Column_type(Field_operators* operators = nullptr, Cell_constructor* cellConstructor = nullptr)
    //     : Base(operators, cellConstructor) {}
    // template <class Container_type>
    // Column_type(const Container_type& nonZeroRowIndices, Field_operators* operators, 
    //             Cell_constructor* cellConstructor)
    //     : Base(nonZeroRowIndices, operators, cellConstructor) {}
    // template <class Container_type, class Row_container_type>
    // Column_type(index columnIndex, const Container_type& nonZeroRowIndices, Row_container_type& rowContainer,
    //             Field_operators* operators, Cell_constructor* cellConstructor)
    //     : Base(columnIndex, nonZeroRowIndices, rowContainer, operators, cellConstructor) {}
    // template <class Container_type>
    // Column_type(const Container_type& nonZeroRowIndices, dimension_type dimension, Field_operators* operators,
    //             Cell_constructor* cellConstructor)
    //     : Base(nonZeroRowIndices, dimension, operators, cellConstructor) {}
    // template <class Container_type, class Row_container_type>
    // Column_type(index columnIndex, const Container_type& nonZeroRowIndices, dimension_type dimension,
    //             Row_container_type& rowContainer, Field_operators* operators, Cell_constructor* cellConstructor)
    //     : Base(columnIndex, nonZeroRowIndices, dimension, rowContainer, operators, cellConstructor) {}
    // Column_type(const Column_type& column, Field_operators* operators = nullptr,
    //             Cell_constructor* cellConstructor = nullptr)
    //     : Base(static_cast<const Base&>(column), operators, cellConstructor) {}
    // template <class Row_container_type>
    // Column_type(const Column_type& column, index columnIndex, Row_container_type& rowContainer,
    //             Field_operators* operators = nullptr, Cell_constructor* cellConstructor = nullptr)
    //     : Base(static_cast<const Base&>(column), columnIndex, rowContainer, operators, cellConstructor) {}
    // Column_type(Column_type&& column) noexcept : Base(std::move(static_cast<Base&>(column))) {}

    template <class... U>
    Column_type(U&&... u) : Base(std::forward<U>(u)...) {}

    index get_rep() const { return rep_; }
    void set_rep(const index& rep) { rep_ = rep; }

    struct Hasher {
      size_t operator()(const Column_type& c) const { return std::hash<Base>()(static_cast<Base>(c)); }
    };

   private:
    index rep_; /**< Index in the union-find of the root of the set representing this column class. */
  };

  /**
   * @brief Constructs an empty matrix.
   * 
   * @param operators Pointer to the field operators.
   * @param cellConstructor Pointer to the cell factory.
   */
  Base_matrix_with_column_compression(Field_operators* operators, Cell_constructor* cellConstructor);
  /**
   * @brief Constructs a matrix from the given ordered columns. The columns are inserted in the given order.
   * If no identical column already existed, a copy of the column is stored. If an identical one existed, no new
   * column is constructed and the relationship between the two is registered in an union-find structure.
   * 
   * @tparam Container_type Range type for @ref Matrix::cell_rep_type ranges.
   * Assumed to have a begin(), end() and size() method.
   * @param columns A vector of @ref Matrix::cell_rep_type ranges to construct the columns from.
   * The content of the ranges are assumed to be sorted by increasing ID value.
   * @param operators Pointer to the field operators.
   * @param cellConstructor Pointer to the cell factory.
   */
  template <class Container_type>
  Base_matrix_with_column_compression(const std::vector<Container_type>& columns, 
                                      Field_operators* operators,
                                      Cell_constructor* cellConstructor);
  /**
   * @brief Constructs a new empty matrix and reserves space for the given number of columns.
   * 
   * @param numberOfColumns Number of columns to reserve space for.
   * @param operators Pointer to the field operators.
   * @param cellConstructor Pointer to the cell factory.
   */
  Base_matrix_with_column_compression(unsigned int numberOfColumns, 
                                      Field_operators* operators,
                                      Cell_constructor* cellConstructor);
  /**
   * @brief Copy constructor. If @p operators or @p cellConstructor is not a null pointer, its value is kept
   * instead of the one in the copied matrix.
   * 
   * @param matrixToCopy Matrix to copy.
   * @param operators Pointer to the field operators.
   * If null pointer, the pointer in @p matrixToCopy is choosen instead.
   * @param cellConstructor Pointer to the cell factory.
   * If null pointer, the pointer in @p matrixToCopy is choosen instead.
   */
  Base_matrix_with_column_compression(const Base_matrix_with_column_compression& matrixToCopy,
                                      Field_operators* operators = nullptr,
                                      Cell_constructor* cellConstructor = nullptr);
  /**
   * @brief Move constructor.
   * 
   * @param other Matrix to move.
   */
  Base_matrix_with_column_compression(Base_matrix_with_column_compression&& other) noexcept;
  /**
   * @brief Destructor.
   */
  ~Base_matrix_with_column_compression();

  /**
   * @brief Inserts a new ordered column at the end of the matrix by copying the given range of @ref cell_rep_type.
   * The content of the range is assumed to be sorted by increasing ID value. 
   * 
   * @tparam Container_type Range of @ref Matrix::cell_rep_type. Assumed to have a begin(), end() and size() method.
   * @param column Range of @ref Matrix::cell_rep_type from which the column has to be constructed. Assumed to be
   * ordered by increasing ID value. 
   */
  template <class Container_type>
  void insert_column(const Container_type& column);
  /**
   * @brief Same as @ref insert_column, only for interface purposes. The given dimension is ignored and not stored.
   * 
   * @tparam Boundary_type Range of @ref Matrix::cell_rep_type. Assumed to have a begin(), end() and size() method.
   * @param boundary Range of @ref Matrix::cell_rep_type from which the column has to be constructed. Assumed to be
   * ordered by increasing ID value. 
   * @param dim Ignored.
   */
  template <class Boundary_type>
  void insert_boundary(const Boundary_type& boundary, dimension_type dim = -1);
  /**
   * @brief Returns the column at the given MatIdx index.
   * The type of the column depends on the choosen options, see @ref PersistenceMatrixOptions::column_type.
   *
   * Remark: the method it-self is not const, because of the path compression optimization of the union-find structure,
   * when a column is looked up. 
   * 
   * @param columnIndex MatIdx index of the column to return.
   * @return Const reference to the column.
   */
  const Column_type& get_column(index columnIndex);
  /**
   * @brief Only available if @ref has_row_access is true.
   * Returns the row at the given row index (see [TODO: description]) of the compressed matrix.
   * The type of the row depends on the choosen options, see @ref PersistenceMatrixOptions::has_intrusive_rows.
   * Note that the row will be from the compressed matrix, that is, the one with only unique columns.
   * 
   * @param rowIndex IDIdx row index of the row to return, see [TODO: description].
   * @return Const reference to the row.
   */
  const Row_type& get_row(index rowIndex) const;
  /**
   * @brief If @ref PersistenceMatrixOptions::has_row_access and @ref PersistenceMatrixOptions::has_removable_rows
   * are true: assumes that the row is empty and removes it. Otherwise, does nothing.
   *
   * @warning The removed rows are always assumed to be empty. If it is not the case, the deleted row cells are not
   * removed from their columns. And in the case of intrusive rows, this will generate a segmentation fault when 
   * the column cells are destroyed later. The row access is just meant as a "read only" access to the rows and the
   * `erase_row` method just as a way to specify that a row is empty and can therefore be removed from dictionnaries.
   * This allows to avoid testing the emptiness of a row at each column cell removal, what can be quite frequent. 
   * 
   * @param rowIndex IDIdx row index of the empty row, see [TODO: description].
   */
  void erase_row(index rowIndex);

  /**
   * @brief Returns the current number of columns in the matrix, counting also the redundant columns.
   * 
   * @return The number of columns.
   */
  index get_number_of_columns() const;

  /**
   * @brief Adds column represented by @p sourceColumn onto the column at @p targetColumnIndex in the matrix.
   *
   * The representatives of redundant columns are summed together, which means that
   * all column compressed together with the target column are affected by the change, not only the target.
   * 
   * @tparam Cell_range_or_column_index Either a range of @ref Cell with a begin() and end() method,
   * or any integer type.
   * @param sourceColumn Either a cell range or the MatIdx index of the column to add.
   * @param targetColumnIndex MatIdx index of the target column.
   */
  template <class Cell_range_or_column_index>
  void add_to(const Cell_range_or_column_index& sourceColumn, index targetColumnIndex);
  /**
   * @brief Multiplies the target column with the coefficiant and then adds the source column to it.
   * That is: targetColumn = (targetColumn * coefficient) + sourceColumn.
   *
   * The representatives of redundant columns are summed together, which means that
   * all column compressed together with the target column are affected by the change, not only the target.
   * 
   * @tparam Cell_range_or_column_index Either a range of @ref Cell with a begin() and end() method,
   * or any integer type.
   * @param sourceColumn Either a cell range or the MatIdx index of the column to add.
   * @param coefficient Value to multiply.
   * @param targetColumnIndex MatIdx index of the target column.
   */
  template <class Cell_range_or_column_index>
  void multiply_target_and_add_to(const Cell_range_or_column_index& sourceColumn, 
                                  const Field_element_type& coefficient,
                                  index targetColumnIndex);
  /**
   * @brief Multiplies the source column with the coefficiant before adding it to the target column.
   * That is: targetColumn += (coefficient * sourceColumn). The source column will **not** be modified.
   *
   * The representatives of redundant columns are summed together, which means that
   * all column compressed together with the target column are affected by the change, not only the target.
   * 
   * @tparam Cell_range_or_column_index Either a range of @ref Cell with a begin() and end() method,
   * or any integer type.
   * @param coefficient Value to multiply.
   * @param sourceColumn Either a cell range or the MatIdx index of the column to add.
   * @param targetColumnIndex MatIdx index of the target column.
   */
  template <class Cell_range_or_column_index>
  void multiply_source_and_add_to(const Field_element_type& coefficient, 
                                  const Cell_range_or_column_index& sourceColumn,
                                  index targetColumnIndex);

  /**
   * @brief Indicates if the cell at given coordinates has value zero.
   * 
   * @param columnIndex MatIdx index of the column of the cell.
   * @param rowIndex Row index of the row of the cell.
   * @return true If the cell has value zero.
   * @return false Otherwise.
   */
  bool is_zero_cell(index columnIndex, index rowIndex);
  /**
   * @brief Indicates if the column at given index has value zero.
   * 
   * @param columnIndex MatIdx index of the column.
   * @return true If the column has value zero.
   * @return false Otherwise.
   */
  bool is_zero_column(index columnIndex);

  /**
   * @brief Resets the matrix to an empty matrix.
   * 
   * @param operators Pointer to the field operators.
   * @param cellConstructor Pointer to the cell factory.
   */
  void reset(Field_operators* operators, Cell_constructor* cellConstructor) {
    columnToRep_.clear_and_dispose(delete_disposer());
    columnClasses_ = boost::disjoint_sets_with_storage<>();
    repToColumn_.clear();
    nextColumnIndex_ = 0;
    operators_ = operators;
    cellPool_ = cellConstructor;
  }

  // void set_operators(Field_operators* operators) {
  //   operators_ = operators;
  //   for (auto& col : columnToRep_) {
  //     col.set_operators(operators);
  //   }
  // }

  /**
   * @brief Assign operator.
   */
  Base_matrix_with_column_compression& operator=(const Base_matrix_with_column_compression& other);
  /**
   * @brief Swap operator.
   */
  friend void swap(Base_matrix_with_column_compression& matrix1, Base_matrix_with_column_compression& matrix2) {
    matrix1.columnToRep_.swap(matrix2.columnToRep_);
    swap(matrix1.columnClasses_, matrix2.columnClasses_);
    matrix1.repToColumn_.swap(matrix2.repToColumn_);  // be careful when columnPool_ becomes not static
    std::swap(matrix1.nextColumnIndex_, matrix2.nextColumnIndex_);
    std::swap(matrix1.operators_, matrix2.operators_);
    std::swap(matrix1.cellPool_, matrix2.cellPool_);

    if constexpr (Master_matrix::Option_list::has_row_access) {
      swap(static_cast<typename Master_matrix::Matrix_row_access_option&>(matrix1),
           static_cast<typename Master_matrix::Matrix_row_access_option&>(matrix2));
      // for (auto& col : matrix1.columnToRep_) {
      //   col.set_rows(&matrix1.rows_);
      // }
      // for (auto& col : matrix2.columnToRep_) {
      //   col.set_rows(&matrix2.rows_);
      // }
    }
  }

  void print();  // for debug

 private:
  /**
   * @brief The disposer object function for boost intrusive container
   */
  struct delete_disposer {
    void operator()(Column_type* delete_this) { columnPool_.destroy(delete_this); }
  };

  using ra_opt = typename Master_matrix::Matrix_row_access_option;
  using cell_rep_type =
      typename std::conditional<Master_matrix::Option_list::is_z2, 
                                index, 
                                std::pair<index, Field_element_type>
                               >::type;
  using tmp_column_type = typename std::conditional<
      Master_matrix::Option_list::is_z2, 
      std::set<index>,
      std::set<std::pair<index, Field_element_type>, typename Master_matrix::CellPairComparator>
    >::type;
  using col_dict_type = boost::intrusive::set<Column_type, boost::intrusive::constant_time_size<false> >;

  col_dict_type columnToRep_;                         /**< Map from a column to the index of its representative. */
  boost::disjoint_sets_with_storage<> columnClasses_; /**< Union-find structure,
                                                           where two columns in the same set are identical. */
  std::vector<Column_type*> repToColumn_;             /**< Map from the representative index to
                                                           the representative Column. */
  index nextColumnIndex_;                             /**< Next unused column index. */
  Field_operators* operators_;                        /**< Field operators, can be nullptr if
                                                           @ref PersistenceMatrixOptions::is_z2 is true. */
  Cell_constructor* cellPool_;                        /**< Cell factory. */
  /**
   * @brief Column factory.
   * @warning As the member is static, they can eventually be problems if the matrix is duplicated in several threads.
   * If this become necessary, the static should be removed in the future.
   */
  inline static Simple_object_pool<Column_type> columnPool_;
  inline static const Column_type empty_column_;      /**< Representative for empty columns. */

  void _insert_column(index columnIndex);
  void _insert_double_column(index columnIndex, typename col_dict_type::iterator& doubleIt);
};

template <class Master_matrix>
inline Base_matrix_with_column_compression<Master_matrix>::Base_matrix_with_column_compression(
    Field_operators* operators, Cell_constructor* cellConstructor)
    : ra_opt(), nextColumnIndex_(0), operators_(operators), cellPool_(cellConstructor) 
{}

template <class Master_matrix>
template <class Container_type>
inline Base_matrix_with_column_compression<Master_matrix>::Base_matrix_with_column_compression(
    const std::vector<Container_type>& columns, Field_operators* operators, Cell_constructor* cellConstructor)
    : ra_opt(columns.size()),
      columnClasses_(columns.size()),
      repToColumn_(columns.size(), nullptr),
      nextColumnIndex_(0),
      operators_(operators),
      cellPool_(cellConstructor) 
{
  for (const Container_type& c : columns) {
    insert_column(c);
  }
}

template <class Master_matrix>
inline Base_matrix_with_column_compression<Master_matrix>::Base_matrix_with_column_compression(
    unsigned int numberOfColumns, Field_operators* operators, Cell_constructor* cellConstructor)
    : ra_opt(numberOfColumns),
      columnClasses_(numberOfColumns),
      repToColumn_(numberOfColumns, nullptr),
      nextColumnIndex_(0),
      operators_(operators),
      cellPool_(cellConstructor) 
{}

template <class Master_matrix>
inline Base_matrix_with_column_compression<Master_matrix>::Base_matrix_with_column_compression(
    const Base_matrix_with_column_compression& matrixToCopy, Field_operators* operators,
    Cell_constructor* cellConstructor)
    : ra_opt(static_cast<const ra_opt&>(matrixToCopy)),
      columnClasses_(matrixToCopy.columnClasses_),
      repToColumn_(matrixToCopy.repToColumn_.size(), nullptr),
      nextColumnIndex_(0),
      operators_(operators == nullptr ? matrixToCopy.operators_ : operators),
      cellPool_(cellConstructor == nullptr ? matrixToCopy.cellPool_ : cellConstructor) 
{
  for (const Column_type* col : matrixToCopy.repToColumn_) {
    if (col != nullptr) {
      if constexpr (Master_matrix::Option_list::has_row_access) {
        repToColumn_[nextColumnIndex_] =
            columnPool_.construct(*col, col->get_column_index(), ra_opt::rows_, operators_, cellPool_);
      } else {
        repToColumn_[nextColumnIndex_] = columnPool_.construct(*col, operators_, cellPool_);
      }
      columnToRep_.insert(columnToRep_.end(), *repToColumn_[nextColumnIndex_]);
      repToColumn_[nextColumnIndex_]->set_rep(nextColumnIndex_);
    }
    nextColumnIndex_++;
  }
}

template <class Master_matrix>
inline Base_matrix_with_column_compression<Master_matrix>::Base_matrix_with_column_compression(
    Base_matrix_with_column_compression&& other) noexcept
    : ra_opt(std::move(static_cast<ra_opt&>(other))),
      columnToRep_(std::move(other.columnToRep_)),
      columnClasses_(std::move(other.columnClasses_)),
      repToColumn_(std::move(other.repToColumn_)),
      nextColumnIndex_(std::exchange(other.nextColumnIndex_, 0)),
      operators_(std::exchange(other.operators_, nullptr)),
      cellPool_(std::exchange(other.cellPool_, nullptr)) 
{
  // TODO: not sur this is necessary, as the address of rows_ should not change from the move, no?
  // if constexpr (Master_matrix::Option_list::has_row_access) {
  //   for (Column_type& col : columnToRep_) {
  //     col.set_rows(&this->rows_);
  //   }
  // }
}

template <class Master_matrix>
inline Base_matrix_with_column_compression<Master_matrix>::~Base_matrix_with_column_compression() 
{
  columnToRep_.clear_and_dispose(delete_disposer());
}

template <class Master_matrix>
template <class Container_type>
inline void Base_matrix_with_column_compression<Master_matrix>::insert_column(const Container_type& column) 
{
  insert_boundary(column);
}

template <class Master_matrix>
template <class Boundary_type>
inline void Base_matrix_with_column_compression<Master_matrix>::insert_boundary(const Boundary_type& boundary,
                                                                                dimension_type dim) 
{
  // handles a dimension which is not actually stored.
  // TODO: verify if this is not a problem for the cohomology algorithm and if yes,
  // change how Column_dimension_option is choosen.
  if (dim == -1) dim = boundary.size() == 0 ? 0 : boundary.size() - 1;

  if constexpr (Master_matrix::Option_list::has_row_access && !Master_matrix::Option_list::has_removable_rows) {
    if (boundary.begin() != boundary.end()) {
      index pivot;
      if constexpr (Master_matrix::Option_list::is_z2) {
        pivot = *std::prev(boundary.end());
      } else {
        pivot = std::prev(boundary.end())->first;
      }
      if (ra_opt::rows_->size() <= pivot) ra_opt::rows_->resize(pivot + 1);
    }
  }

  if (repToColumn_.size() == nextColumnIndex_) {
    // could perhaps be avoided, if find_set returns something special when it does not find
    columnClasses_.link(nextColumnIndex_, nextColumnIndex_);
    if constexpr (Master_matrix::Option_list::has_row_access) {
      repToColumn_.push_back(
          columnPool_.construct(nextColumnIndex_, boundary, dim, ra_opt::rows_, operators_, cellPool_));
    } else {
      repToColumn_.push_back(columnPool_.construct(boundary, dim, operators_, cellPool_));
    }
  } else {
    if constexpr (Master_matrix::Option_list::has_row_access) {
      repToColumn_[nextColumnIndex_] =
          columnPool_.construct(nextColumnIndex_, boundary, dim, ra_opt::rows_, operators_, cellPool_);
    } else {
      repToColumn_[nextColumnIndex_] = columnPool_.construct(boundary, dim, operators_, cellPool_);
    }
  }
  _insert_column(nextColumnIndex_);

  nextColumnIndex_++;
}

template <class Master_matrix>
inline const typename Base_matrix_with_column_compression<Master_matrix>::Column_type&
Base_matrix_with_column_compression<Master_matrix>::get_column(index columnIndex) 
{
  auto col = repToColumn_[columnClasses_.find_set(columnIndex)];
  if (col == nullptr) return empty_column_;
  return *col;
}

template <class Master_matrix>
inline const typename Base_matrix_with_column_compression<Master_matrix>::Row_type&
Base_matrix_with_column_compression<Master_matrix>::get_row(index rowIndex) const 
{
  static_assert(Master_matrix::Option_list::has_row_access, "Row access has to be enabled for this method.");

  return ra_opt::get_row(rowIndex);
}

template <class Master_matrix>
inline void Base_matrix_with_column_compression<Master_matrix>::erase_row(index rowIndex) 
{
  if constexpr (Master_matrix::Option_list::has_row_access && Master_matrix::Option_list::has_removable_rows) {
    ra_opt::erase_row(rowIndex);
  }
}

template <class Master_matrix>
inline typename Base_matrix_with_column_compression<Master_matrix>::index
Base_matrix_with_column_compression<Master_matrix>::get_number_of_columns() const 
{
  return nextColumnIndex_;
}

template <class Master_matrix>
template <class Cell_range_or_column_index>
inline void Base_matrix_with_column_compression<Master_matrix>::add_to(const Cell_range_or_column_index& sourceColumn,
                                                                       index targetColumnIndex) 
{
  // handle case where targetRep == sourceRep?
  index targetRep = columnClasses_.find_set(targetColumnIndex);
  Column_type& target = *repToColumn_[targetRep];
  columnToRep_.erase(target);
  if constexpr (std::is_integral_v<Cell_range_or_column_index>) {
    target += get_column(sourceColumn);
  } else {
    target += sourceColumn;
  }
  _insert_column(targetRep);
}

template <class Master_matrix>
template <class Cell_range_or_column_index>
inline void Base_matrix_with_column_compression<Master_matrix>::multiply_target_and_add_to(
    const Cell_range_or_column_index& sourceColumn, const Field_element_type& coefficient, index targetColumnIndex) 
{
  // handle case where targetRep == sourceRep?
  index targetRep = columnClasses_.find_set(targetColumnIndex);
  Column_type& target = *repToColumn_[targetRep];
  columnToRep_.erase(target);
  if constexpr (std::is_integral_v<Cell_range_or_column_index>) {
    target.multiply_and_add(coefficient, get_column(sourceColumn));
  } else {
    target.multiply_and_add(coefficient, sourceColumn);
  }
  _insert_column(targetRep);
}

template <class Master_matrix>
template <class Cell_range_or_column_index>
inline void Base_matrix_with_column_compression<Master_matrix>::multiply_source_and_add_to(
    const Field_element_type& coefficient, const Cell_range_or_column_index& sourceColumn, index targetColumnIndex) 
{
  // handle case where targetRep == sourceRep?
  index targetRep = columnClasses_.find_set(targetColumnIndex);
  Column_type& target = *repToColumn_[targetRep];
  columnToRep_.erase(target);
  if constexpr (std::is_integral_v<Cell_range_or_column_index>) {
    target.multiply_and_add(get_column(sourceColumn), coefficient);
  } else {
    target.multiply_and_add(sourceColumn, coefficient);
  }
  _insert_column(targetRep);
}

template <class Master_matrix>
inline bool Base_matrix_with_column_compression<Master_matrix>::is_zero_cell(index columnIndex, index rowIndex) 
{
  auto col = repToColumn_[columnClasses_.find_set(columnIndex)];
  if (col == nullptr) return true;
  return !col->is_non_zero(rowIndex);
}

template <class Master_matrix>
inline bool Base_matrix_with_column_compression<Master_matrix>::is_zero_column(index columnIndex) 
{
  auto col = repToColumn_[columnClasses_.find_set(columnIndex)];
  if (col == nullptr) return true;
  return col->is_empty();
}

template <class Master_matrix>
inline Base_matrix_with_column_compression<Master_matrix>&
Base_matrix_with_column_compression<Master_matrix>::operator=(const Base_matrix_with_column_compression& other) 
{
  for (auto col : repToColumn_) {
    if (col != nullptr) {
      columnPool_.destroy(col);
      col = nullptr;
    }
  }
  ra_opt::operator=(other);
  columnClasses_ = other.columnClasses_;
  columnToRep_.reserve(other.columnToRep_.size());
  repToColumn_.resize(other.repToColumn_.size(), nullptr);
  nextColumnIndex_ = 0;
  operators_ = other.operators_;
  cellPool_ = other.cellPool_;
  for (const Column_type* col : other.repToColumn_) {
    if constexpr (Master_matrix::Option_list::has_row_access) {
      repToColumn_[nextColumnIndex_] =
          columnPool_.construct(*col, col->get_column_index(), ra_opt::rows_, operators_, cellPool_);
    } else {
      repToColumn_[nextColumnIndex_] = columnPool_.construct(*col, operators_, cellPool_);
    }
    columnToRep_.insert(columnToRep_.end(), *repToColumn_[nextColumnIndex_]);
    repToColumn_[nextColumnIndex_]->set_rep(nextColumnIndex_);
    nextColumnIndex_++;
  }
  return *this;
}

template <class Master_matrix>
inline void Base_matrix_with_column_compression<Master_matrix>::print() 
{
  std::cout << "Compressed_matrix:\n";
  for (Column_type& col : columnToRep_) {
    for (auto e : col->get_content(nextColumnIndex_)) {
      if (e == 0u)
        std::cout << "- ";
      else
        std::cout << e << " ";
    }
    std::cout << "(";
    for (index i = 0; i < nextColumnIndex_; ++i) {
      if (columnClasses_.find_set(i) == col.get_rep()) std::cout << i << " ";
    }
    std::cout << ")\n";
  }
  std::cout << "\n";
  std::cout << "Row Matrix:\n";
  for (index i = 0; i < ra_opt::rows_->size(); ++i) {
    const Row_type& row = ra_opt::rows_[i];
    for (const auto& cell : row) {
      std::cout << cell.get_column_index() << " ";
    }
    std::cout << "(" << i << ")\n";
  }
  std::cout << "\n";
}

template <class Master_matrix>
inline void Base_matrix_with_column_compression<Master_matrix>::_insert_column(index columnIndex) 
{
  Column_type& col = *repToColumn_[columnIndex];

  if (col.is_empty()) {
    columnPool_.destroy(&col);  // delete curr_col;
    repToColumn_[columnIndex] = nullptr;
    return;
  }

  col.set_rep(columnIndex);
  auto res = columnToRep_.insert(col);
  if (res.first->get_rep() != columnIndex) {  //if true, then redundant column
    _insert_double_column(columnIndex, res.first);
  }
}

template <class Master_matrix>
inline void Base_matrix_with_column_compression<Master_matrix>::_insert_double_column(
    index columnIndex, typename col_dict_type::iterator& doubleIt) 
{
  index doubleRep = doubleIt->get_rep();
  columnClasses_.link(columnIndex, doubleRep);  // both should be representatives
  index newRep = columnClasses_.find_set(columnIndex);

  columnPool_.destroy(repToColumn_[columnIndex]);
  repToColumn_[columnIndex] = nullptr;

  if (newRep == columnIndex) {
    std::swap(repToColumn_[doubleRep], repToColumn_[columnIndex]);
    doubleIt->set_rep(columnIndex);
  }
}

}  // namespace persistence_matrix
}  // namespace Gudhi

#endif  // PM_BASE_MATRIX_COMPRESSION_H
