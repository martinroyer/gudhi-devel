/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2022 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

#include <iostream>
#include <random>
#include <vector>
#include <utility>

#include "gudhi/matrix.h"
#include "gudhi/options.h"
#include "gudhi/Z2_field.h"
#include "gudhi/Zp_field.h"

using Gudhi::persistence_matrix::Z2_field_element;
using Gudhi::persistence_matrix::Zp_field_element;
using Gudhi::persistence_matrix::Matrix;
using Gudhi::persistence_matrix::Representative_cycles_options;
using Gudhi::persistence_matrix::Default_options;
using Gudhi::persistence_matrix::Zigzag_options;
using Gudhi::persistence_matrix::Multi_persistence_options;
using Gudhi::persistence_matrix::Cohomology_persistence_options;
using Gudhi::persistence_matrix::Column_types;

using boundary_type = std::vector<unsigned int>;
template<class Field_type = Zp_field_element<5> >
using field_boundary_type = std::vector<std::pair<unsigned int,Field_type> >;

template<class Matrix_type>
void test_comp_zp(Matrix_type m)
{
	std::vector<field_boundary_type<> > ob;
	field_boundary_type<> fb;

	m.insert_boundary(fb);
	m.get_column(0);
	m.get_max_dimension();
	m.get_number_of_columns();
	m.get_column_dimension(0);
	m.add_to(0, 1);
	m.is_zero_cell(0, 0);
	m.is_zero_column(0);
	m.get_pivot(0);
	swap(m, m);
	m.print();
	m = Matrix_type(ob);
}

template<class Matrix_type>
void test_comp_z2(Matrix_type m)
{
	std::vector<boundary_type> ob;
	boundary_type fb;

	m.insert_boundary(fb);
	m.get_column(0);
	m.get_max_dimension();
	m.get_number_of_columns();
	m.get_column_dimension(0);
	m.add_to(0, 1);
	m.is_zero_cell(0, 0);
	m.is_zero_column(0);
	m.get_pivot(0);
	swap(m, m);
	m.print();
	m = Matrix_type(ob);
}

int main(int argc, char* const argv[]) {
	Zp_field_element<5> f(3);
	Zp_field_element<5> f2(7);

	std::clog << "== : " << (f == f2) << " " << (f == 3u) << " " << (f2 == 3u) << " " << (f == 7u) << "\n";

	std::clog << "+ : " << (f + f2) << " " << (f + 3u) << " " << (f2 + 3u) << " " << (7u + f) << "\n";
	std::clog << "- : " << (f - f2) << " " << (f - 3u) << " " << (f2 - 3u) << " " << (7u - f) << "\n";
	std::clog << "* : " << (f * f2) << " " << (f * 3u) << " " << (f2 * 3u) << " " << (7u * f) << "\n";

	f += f2;
	f2 += 3u;

	std::clog << "+= : " << f << " " << f2 << "\n";

	unsigned int a = 3;

	a = f;
	std::clog << "= : " << f << " " << a << "\n";

	std::vector<boundary_type> orderedBoundaries1;
	boundary_type b;
	orderedBoundaries1.emplace_back();
	orderedBoundaries1.emplace_back();
	orderedBoundaries1.emplace_back();
	orderedBoundaries1.push_back(boundary_type{0,1});
	orderedBoundaries1.push_back(boundary_type{1,2});

	std::vector<field_boundary_type<> > orderedBoundaries2;
	field_boundary_type<> fb;
	std::pair<unsigned int,Zp_field_element<5>> p;
	orderedBoundaries2.emplace_back();
	orderedBoundaries2.emplace_back();
	orderedBoundaries2.emplace_back();
	orderedBoundaries2.emplace_back(field_boundary_type<>{{0,3},{1,2}});
	orderedBoundaries2.push_back(field_boundary_type<>{{1,3},{2,2}});

	Matrix<Representative_cycles_options<Zp_field_element<5> > > m1(orderedBoundaries2);
	Matrix<Representative_cycles_options<Zp_field_element<2> > > m2(orderedBoundaries1);
	Matrix<Representative_cycles_options<Zp_field_element<5>,Column_types::LIST> > m3(orderedBoundaries2);
	Matrix<Representative_cycles_options<Zp_field_element<2>,Column_types::LIST> > m4(orderedBoundaries1);
	Matrix<Representative_cycles_options<Zp_field_element<5>,Column_types::UNORDERED_SET> > m5(orderedBoundaries2);
	Matrix<Representative_cycles_options<Zp_field_element<2>,Column_types::UNORDERED_SET> > m6(orderedBoundaries1);
	Matrix<Representative_cycles_options<Zp_field_element<5>,Column_types::VECTOR> > m7(orderedBoundaries2);
	Matrix<Representative_cycles_options<Zp_field_element<2>,Column_types::VECTOR> > m8(orderedBoundaries1);
	Matrix<Representative_cycles_options<Zp_field_element<2>,Column_types::HEAP> > m10(orderedBoundaries1);

	Matrix<Default_options<Zp_field_element<5> > > m11(orderedBoundaries2);
	Matrix<Default_options<Zp_field_element<2> > > m12(orderedBoundaries1);
	Matrix<Default_options<Zp_field_element<5>,Column_types::LIST> > m13(orderedBoundaries2);
	Matrix<Default_options<Zp_field_element<2>,Column_types::LIST> > m14(orderedBoundaries1);
	Matrix<Default_options<Zp_field_element<5>,Column_types::UNORDERED_SET> > m15(orderedBoundaries2);
	Matrix<Default_options<Zp_field_element<2>,Column_types::UNORDERED_SET> > m16(orderedBoundaries1);
	Matrix<Default_options<Zp_field_element<5>,Column_types::VECTOR> > m17(orderedBoundaries2);
	Matrix<Default_options<Zp_field_element<2>,Column_types::VECTOR> > m18(orderedBoundaries1);
	Matrix<Default_options<Zp_field_element<2>,Column_types::HEAP> > m20(orderedBoundaries1);

	Matrix<Multi_persistence_options<> > m21(orderedBoundaries1);
	Matrix<Multi_persistence_options<Column_types::LIST> > m22(orderedBoundaries1);
	Matrix<Multi_persistence_options<Column_types::UNORDERED_SET> > m23(orderedBoundaries1);
	Matrix<Multi_persistence_options<Column_types::VECTOR> > m24(orderedBoundaries1);
	Matrix<Multi_persistence_options<Column_types::HEAP> > m25(orderedBoundaries1);

	Matrix<Zigzag_options<> > m31(orderedBoundaries1);
	Matrix<Zigzag_options<Column_types::LIST> > m32(orderedBoundaries1);

	Matrix<Cohomology_persistence_options<Zp_field_element<5> > > m41(orderedBoundaries2);
	Matrix<Cohomology_persistence_options<Zp_field_element<2> > > m42(orderedBoundaries1);

	test_comp_zp(m1);
	test_comp_z2(m2);
	test_comp_zp(m3);
	test_comp_z2(m4);
	test_comp_zp(m5);
	test_comp_z2(m6);
	test_comp_zp(m7);
	test_comp_z2(m8);
	test_comp_z2(m10);

	test_comp_zp(m11);
	test_comp_z2(m12);
	test_comp_zp(m13);
	test_comp_z2(m14);
	test_comp_zp(m15);
	test_comp_z2(m16);
	test_comp_zp(m17);
	test_comp_z2(m18);
	test_comp_z2(m20);

	m11.zero_cell(0, 0);
	m11.zero_column(0);
	m12.zero_cell(0, 0);
	m12.zero_column(0);
	m13.zero_cell(0, 0);
	m13.zero_column(0);
	m14.zero_cell(0, 0);
	m14.zero_column(0);
	m15.zero_cell(0, 0);
	m15.zero_column(0);
	m16.zero_cell(0, 0);
	m16.zero_column(0);
	m17.zero_cell(0, 0);
	m17.zero_column(0);
	m18.zero_cell(0, 0);
	m18.zero_column(0);
	m20.zero_cell(0, 0);
	m20.zero_column(0);

	test_comp_z2(m21);
	test_comp_z2(m22);
	test_comp_z2(m23);
	test_comp_z2(m24);
	test_comp_z2(m25);

	m21.get_column_with_pivot(0);
	m22.get_column_with_pivot(0);
	m23.get_column_with_pivot(0);
	m24.get_column_with_pivot(0);
	m25.get_column_with_pivot(0);

	test_comp_z2(m31);
	test_comp_z2(m32);

	m31.get_row(0);
	m31.erase_last();
	m32.get_row(0);
	m32.erase_last();
	m31.get_column_with_pivot(0);
	m32.get_column_with_pivot(0);

	test_comp_zp(m41);
	test_comp_z2(m42);

	return 0;
}
