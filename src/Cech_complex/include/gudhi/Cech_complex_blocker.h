/*    This file is part of the Gudhi Library - https://gudhi.inria.fr/ - which is released under MIT.
 *    See file LICENSE or go to https://gudhi.inria.fr/licensing/ for full license details.
 *    Author(s):       Vincent Rouvreau
 *
 *    Copyright (C) 2018 Inria
 *
 *    Modification(s):
 *      - YYYY/MM Author: Description of the modification
 */

#ifndef CECH_COMPLEX_BLOCKER_H_
#define CECH_COMPLEX_BLOCKER_H_

#include <CGAL/NT_converter.h> // for casting from FT to Filtration_value
#include <CGAL/Lazy_exact_nt.h> // for CGAL::exact

#include <iostream>
#include <vector>
#include <set>
#include <cmath>  // for std::sqrt

namespace Gudhi {

namespace cech_complex {

/** \internal
 * \class Cech_blocker
 * \brief Čech complex blocker.
 *
 * \ingroup cech_complex
 *
 * \details
 * Čech blocker is an oracle constructed from a Cech_complex and a simplicial complex.
 *
 * \tparam SimplicialComplexForCech furnishes `Simplex_handle` and `Filtration_value` type definition,
 * `simplex_vertex_range(Simplex_handle sh)`and `assign_filtration(Simplex_handle sh, Filtration_value filt)` methods.
 *
 * \tparam Cech_complex is required by the blocker.
 *
 * \tparam Kernel CGAL kernel: either Epick_d or Epeck_d.
 */
template <typename SimplicialComplexForCech, typename Cech_complex, typename Kernel>
class Cech_blocker {

 public:

  using Point_d = typename Kernel::Point_d;
  // Numeric type of coordinates in the kernel
  using FT = typename Kernel::FT;
  // Sphere is a pair of point and squared radius.
  using Sphere = typename std::pair<Point_d, FT>;

 private:

  using Simplex_handle = typename SimplicialComplexForCech::Simplex_handle;
  using Filtration_value = typename SimplicialComplexForCech::Filtration_value;

  template<class PointIterator>
  Sphere get_sphere(PointIterator begin, PointIterator end) const {
    Point_d c = kernel_.construct_circumcenter_d_object()(begin, end);
    FT r = kernel_.squared_distance_d_object()(c, *begin);
    return std::make_pair(std::move(c), std::move(r));
  }

 public:

  /** \internal \brief Čech complex blocker operator() - the oracle - assigns the filtration value from the simplex
   * radius and returns if the simplex expansion must be blocked.
   *  \param[in] sh The Simplex_handle.
   *  \return true if the simplex radius is greater than the Cech_complex max_radius*/
  bool operator()(Simplex_handle sh) {
    using Point_cloud =  std::vector<Point_d>;
    CGAL::NT_converter<FT, Filtration_value> cast_to_fv;
    Filtration_value radius = 0;
    bool is_min_enclos_ball = false;

    // for each face of simplex sh, test outsider point is indeed inside enclosing ball, if yes, take it and exit loop, otherwise, new sphere is circumsphere of all vertices
    for (auto face : sc_ptr_->boundary_simplex_range(sh)) {
        // Find which vertex of sh is missing in face. We rely on the fact that simplex_vertex_range is sorted.
        auto longlist = sc_ptr_->simplex_vertex_range(sh);
        auto shortlist = sc_ptr_->simplex_vertex_range(face);

        auto longiter = std::begin(longlist);
        auto shortiter = std::begin(shortlist);
        auto enditer = std::end(shortlist);
        while(shortiter != enditer && *longiter == *shortiter) { ++longiter; ++shortiter; }
        auto extra = *longiter; // Vertex_handle

        Sphere sph;
        auto k = sc_ptr_->key(face);
        if(k != sc_ptr_->null_key()) {
            sph = cc_ptr_->get_cache().at(k);
        }
        else {
            Point_cloud face_points;
            for (auto vertex : sc_ptr_->simplex_vertex_range(face)) {
                face_points.push_back(cc_ptr_->get_point(vertex));
#ifdef DEBUG_TRACES
                std::clog << "#(" << vertex << ")#";
#endif  // DEBUG_TRACES
            }
            sph = get_sphere(face_points.cbegin(), face_points.cend());
            // Put edge sphere in cache
            sc_ptr_->assign_key(face, cc_ptr_->get_cache().size());
            cc_ptr_->get_cache().push_back(sph);
            // Clear face_points
            face_points.clear();
        }
        // Check if the minimal enclosing ball of current face contains the extra point
        if (kernel_.squared_distance_d_object()(sph.first, cc_ptr_->get_point(extra)) <= sph.second) {
#ifdef DEBUG_TRACES
            std::clog << "center: " << sph.first << ", radius: " <<  radius << std::endl;
#endif  // DEBUG_TRACES
            is_min_enclos_ball = true;
#if CGAL_VERSION_NR >= 1050000000
            if(exact_) CGAL::exact(sph.second);
#endif
            radius = std::sqrt(cast_to_fv(sph.second));
            sc_ptr_->assign_key(sh, cc_ptr_->get_cache().size());
            cc_ptr_->get_cache().push_back(sph);
            break;
        }
    }
    // Spheres of each face don't contain the whole simplex
    if(!is_min_enclos_ball) {
        Point_cloud points;
        for (auto vertex : sc_ptr_->simplex_vertex_range(sh)) {
            points.push_back(cc_ptr_->get_point(vertex));
        }
        Sphere sph = get_sphere(points.cbegin(), points.cend());
#if CGAL_VERSION_NR >= 1050000000
        if(exact_) CGAL::exact(sph.second);
#endif
        radius = std::sqrt(cast_to_fv(sph.second));

        sc_ptr_->assign_key(sh, cc_ptr_->get_cache().size());
        cc_ptr_->get_cache().push_back(std::move(sph));
    }

#ifdef DEBUG_TRACES
    if (radius > cc_ptr_->max_radius()) std::clog << "radius > max_radius => expansion is blocked\n";
#endif  // DEBUG_TRACES
    sc_ptr_->assign_filtration(sh, radius);
    return (radius > cc_ptr_->max_radius());
  }

  /** \internal \brief Čech complex blocker constructor. */
  Cech_blocker(SimplicialComplexForCech* sc_ptr, Cech_complex* cc_ptr, const bool exact) : sc_ptr_(sc_ptr), cc_ptr_(cc_ptr), exact_(exact) {}

 private:
  SimplicialComplexForCech* sc_ptr_;
  Cech_complex* cc_ptr_;
  Kernel kernel_;
  const bool exact_;
};

}  // namespace cech_complex

}  // namespace Gudhi

#endif  // CECH_COMPLEX_BLOCKER_H_
