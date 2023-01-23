/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2022 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

#ifndef C_VECTOR_COLUMN_H
#define C_VECTOR_COLUMN_H

#include <iostream>
#include <vector>
#include <unordered_set>

#include "../../utilities/utilities.h"
#include "../vector_column.h"

namespace Gudhi {
namespace persistence_matrix {

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
class Vector_chain_column : public Vector_column<Field_element_type,Cell_type,Row_access_option>
{
private:
	using Base = Vector_column<Field_element_type,Cell_type,Row_access_option>;

public:
	using Cell = typename Base::Cell;
	using Column_type = typename Base::Column_type;
	using iterator = typename Base::iterator;
	using const_iterator = typename Base::const_iterator;

	Vector_chain_column(Dictionnary_type& pivotToColumnIndex);
	template<class Chain_type>
	Vector_chain_column(const Chain_type& chain, dimension_type dimension, Dictionnary_type& pivotToColumnIndex);
	template<class Row_container_type>
	Vector_chain_column(index columnIndex, Row_container_type &rowContainer, Dictionnary_type& pivotToColumnIndex);
	template<class Chain_type, class Row_container_type>
	Vector_chain_column(index columnIndex, const Chain_type& chain, dimension_type dimension, Row_container_type &rowContainer, Dictionnary_type& pivotToColumnIndex);
	Vector_chain_column(const Vector_chain_column& column);
	Vector_chain_column(const Vector_chain_column& column, index columnIndex);
	Vector_chain_column(Vector_chain_column&& column) noexcept;

	int get_pivot() const;
	Field_element_type get_pivot_value();
	index get_paired_chain_index() const;
	bool is_paired() const;
	void assign_paired_chain(index other_col);
	void unassign_paired_chain();

	Vector_chain_column& operator+=(Vector_chain_column &column);
	friend Vector_chain_column operator+(Vector_chain_column column1, Vector_chain_column &column2){
		column1 += column2;
		return column1;
	}
	friend Vector_chain_column operator*(Vector_chain_column column, unsigned int const& v){
		column *= v;
		return column;
	}
	friend Vector_chain_column operator*(unsigned int const& v, Vector_chain_column column){
		column *= v;
		return column;
	}

	Vector_chain_column& operator=(Vector_chain_column other);

	friend void swap(Vector_chain_column& col1, Vector_chain_column& col2){
		swap(static_cast<Vector_column<Field_element_type,Cell_type,Row_access_option>&>(col1),
			 static_cast<Vector_column<Field_element_type,Cell_type,Row_access_option>&>(col2));
		std::swap(col1.pivotToColumnIndex_, col2.pivotToColumnIndex_);
		std::swap(col1.pivot_, col2.pivot_);
		std::swap(col1.pairedColumn_, col2.pairedColumn_);
	}

private:
	Dictionnary_type* pivotToColumnIndex_;
	int pivot_;		//simplex index associated to the chain
	int pairedColumn_;
};

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::Vector_chain_column(Dictionnary_type& pivotToColumnIndex)
	: Base(),
	  pivotToColumnIndex_(&pivotToColumnIndex),
	  pivot_(-1),
	  pairedColumn_(-1)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
template<class Chain_type>
inline Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::Vector_chain_column(
		const Chain_type& chain, dimension_type dimension, Dictionnary_type& pivotToColumnIndex)
	: Base(chain, dimension),
	  pivotToColumnIndex_(&pivotToColumnIndex),
	  pivot_(chain.empty() ? -1 : chain.rbegin()->first),
	  pairedColumn_(-1)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
template<class Row_container_type>
inline Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::Vector_chain_column(
		index columnIndex, Row_container_type &rowContainer, Dictionnary_type &pivotToColumnIndex)
	: Base(columnIndex, rowContainer),
	  pivotToColumnIndex_(&pivotToColumnIndex),
	  pivot_(-1),
	  pairedColumn_(-1)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
template<class Chain_type, class Row_container_type>
inline Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::Vector_chain_column(
		index columnIndex, const Chain_type& chain, dimension_type dimension, Row_container_type &rowContainer, Dictionnary_type &pivotToColumnIndex)
	: Base(columnIndex, chain, dimension, rowContainer),
	  pivotToColumnIndex_(&pivotToColumnIndex),
	  pivot_(chain.empty() ? -1 : chain.rbegin()->first),
	  pairedColumn_(-1)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::Vector_chain_column(
		const Vector_chain_column& column)
	: Base(static_cast<const Base&>(column)),
	  pivotToColumnIndex_(column.pivotToColumnIndex_),
	  pivot_(column.pivot_),
	  pairedColumn_(column.pairedColumn_)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::Vector_chain_column(
		const Vector_chain_column& column, index columnIndex)
	: Base(static_cast<const Base&>(column), columnIndex),
	  pivotToColumnIndex_(column.pivotToColumnIndex_),
	  pivot_(column.pivot_),
	  pairedColumn_(column.pairedColumn_)
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::Vector_chain_column(
		Vector_chain_column&& column) noexcept
	: Base(std::move(static_cast<Base&&>(column))),
	  pivotToColumnIndex_(std::move(column.pivotToColumnIndex_)),
	  pivot_(std::exchange(column.pivot_, -1)),
	  pairedColumn_(std::exchange(column.pairedColumn_, 0))
{}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline int Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::get_pivot() const
{
	return pivot_;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Field_element_type Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::get_pivot_value()
{
	if (pivot_ == -1) return Field_element_type();

	for (const Cell* cell : Base::column_){
		if (cell->get_row_index() == pivot_) return cell->get_element();
	}

	return Field_element_type();	//should never happen if chain column is used properly
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline index Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::get_paired_chain_index() const
{
	return pairedColumn_;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline bool Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::is_paired() const
{
	return pairedColumn_ != -1;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline void Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::assign_paired_chain(index other_col)
{
	pairedColumn_ = other_col;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline void Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::unassign_paired_chain()
{
	pairedColumn_ = -1;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option> &
Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::operator+=(Vector_chain_column &column)
{
	Base::operator+=(column);

	//assumes that the addition never zeros out this column. If the use of those columns changes at some point, we should think about it.
	if (!Base::is_non_zero(pivot_)){
		std::swap(pivotToColumnIndex_->at(pivot_),
				  pivotToColumnIndex_->at(column.get_pivot()));
		std::swap(pivot_, column.pivot_);
	}

	return *this;
}

template<class Dictionnary_type, class Field_element_type, class Cell_type, class Row_access_option>
inline Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option> &
Vector_chain_column<Dictionnary_type,Field_element_type,Cell_type,Row_access_option>::operator=(Vector_chain_column other)
{
	Base::operator=(static_cast<Base&>(other));
	std::swap(pivotToColumnIndex_, other.pivotToColumnIndex_);
	std::swap(pivot_, other.pivot_);
	std::swap(pairedColumn_, other.pairedColumn_);
	return *this;
}

} //namespace persistence_matrix
} //namespace Gudhi

#endif // C_VECTOR_COLUMN_H
