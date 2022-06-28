/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2022 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

#ifndef Z2_UNORDEREDSETCOLUMN_H
#define Z2_UNORDEREDSETCOLUMN_H

#include <iostream>
#include <list>
#include <unordered_set>
#include <algorithm>

#include "../utilities.h"

namespace Gudhi {
namespace persistence_matrix {

class Z2_unordered_set_column
{
public:
	Z2_unordered_set_column();
	Z2_unordered_set_column(boundary_type& boundary);
	Z2_unordered_set_column(Z2_unordered_set_column& column);
	Z2_unordered_set_column(Z2_unordered_set_column&& column) noexcept;

	void get_content(boundary_type& container);
	bool contains(unsigned int value) const;
	bool is_empty();
	dimension_type get_dimension() const;
	int get_pivot();
	void clear();
	void clear(unsigned int value);
	void reorder(std::vector<index>& valueMap);
	void add(Z2_unordered_set_column& column);

	Z2_unordered_set_column& operator=(Z2_unordered_set_column other);

	friend void swap(Z2_unordered_set_column& col1, Z2_unordered_set_column& col2);

private:
	int dim_;
	std::unordered_set<unsigned int> column_;
	bool pivotChanged_;
	int pivot_;
};

inline Z2_unordered_set_column::Z2_unordered_set_column()
	: dim_(0), pivotChanged_(false), pivot_(-1)
{}

inline Z2_unordered_set_column::Z2_unordered_set_column(boundary_type &boundary)
	: dim_(boundary.size() == 0 ? 0 : boundary.size() - 1),
	  column_(boundary.begin(), boundary.end()),
	  pivotChanged_(false),
	  pivot_(boundary.size() == 0 ? -1 : *std::max_element(boundary.begin(), boundary.end()))
{}

inline Z2_unordered_set_column::Z2_unordered_set_column(Z2_unordered_set_column &column)
	: dim_(column.dim_),
	  column_(column.column_),
	  pivotChanged_(column.pivotChanged_),
	  pivot_(column.pivot_)
{}

inline Z2_unordered_set_column::Z2_unordered_set_column(Z2_unordered_set_column &&column) noexcept
	: dim_(std::exchange(column.dim_, 0)),
	  column_(std::move(column.column_)),
	  pivotChanged_(std::exchange(column.pivotChanged_, 0)),
	  pivot_(std::exchange(column.pivot_, 0))
{}

inline void Z2_unordered_set_column::get_content(boundary_type &container)
{
	std::copy(column_.begin(), column_.end(), std::back_inserter(container));
	std::sort(container.begin(), container.end());
}

inline bool Z2_unordered_set_column::contains(unsigned int value) const
{
	return column_.find(value) != column_.end();
}

inline bool Z2_unordered_set_column::is_empty()
{
	return column_.empty();
}

inline dimension_type Z2_unordered_set_column::get_dimension() const
{
	return dim_;
}

inline int Z2_unordered_set_column::get_pivot()
{
	if (pivotChanged_){
		pivot_ = column_.size() == 0 ?
					-1
				  : *std::max_element(column_.begin(), column_.end());
		pivotChanged_ = false;
	}

	return pivot_;
}

inline void Z2_unordered_set_column::clear()
{
	column_.clear();
	pivot_ = -1;
	pivotChanged_ = false;
}

inline void Z2_unordered_set_column::clear(unsigned int value)
{
	column_.erase(value);
	if (static_cast<int>(value) == pivot_) pivotChanged_ = true;
}

inline void Z2_unordered_set_column::reorder(std::vector<index> &valueMap)
{
	std::unordered_set<unsigned int> newSet;
	for (const unsigned int& v : column_) newSet.insert(valueMap.at(v));
	column_.swap(newSet);
	pivotChanged_ = true;
}

inline void Z2_unordered_set_column::add(Z2_unordered_set_column &column)
{
	for (const unsigned int& v : column.column_){
		if (column_.find(v) != column_.end()){
			column_.erase(v);
			if (static_cast<int>(v) == pivot_) pivotChanged_ = true;
		} else {
			column_.insert(v);
			if (static_cast<int>(v) > pivot_){
				pivot_ = v;
				pivotChanged_ = false;
			}
		}
	}
}

inline Z2_unordered_set_column &Z2_unordered_set_column::operator=(Z2_unordered_set_column other)
{
	std::swap(dim_, other.dim_);
	std::swap(column_, other.column_);
	std::swap(pivotChanged_, other.pivotChanged_);
	std::swap(pivot_, other.pivot_);
	return *this;
}

inline void swap(Z2_unordered_set_column& col1, Z2_unordered_set_column& col2)
{
	std::swap(col1.dim_, col2.dim_);
	col1.column_.swap(col2.column_);
	std::swap(col1.pivotChanged_, col2.pivotChanged_);
	std::swap(col1.pivot_, col2.pivot_);
}

} //namespace persistence_matrix
} //namespace Gudhi

#endif // Z2_UNORDEREDSETCOLUMN_H
