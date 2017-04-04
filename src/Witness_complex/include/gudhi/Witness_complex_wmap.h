/*    This file is part of the Gudhi Library. The Gudhi library
 *    (Geometric Understanding in Higher Dimensions) is a generic C++
 *    library for computational topology.
 *
 *    Author(s):       Siargey Kachanovich
 *
 *    Copyright (C) 2017  INRIA (France)
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WITNESS_COMPLEX_WMAP_H_
#define WITNESS_COMPLEX_WMAP_H_

#include <gudhi/Witness_complex.h>
#include <gudhi/Kd_tree_search.h>
#include <gudhi/Active_witness/Active_witness.h>
#include <gudhi/Active_witness/Witness_for_simplex.h>
#include <gudhi/Active_witness/Sib_vertex_pair.h>
#include <gudhi/Witness_complex/all_faces_in.h>
#include <gudhi/Witness_complex/check_if_neighbors.h>
// #include <gudhi/Witness_complex/Simplex.h>
#include <gudhi/Simplex_tree/Vertex_subtree_iterator.h>
#include <gudhi/Simplex_tree/Fixed_dimension_iterator.h>

#include <gudhi/Simplex_tree.h>

#include <utility>
#include <vector>
#include <list>
#include <map>
#include <limits>

namespace Gudhi {
  
namespace witness_complex {

/**
 * \private
 * \class Witness_complex
 * \brief Constructs (weak) witness complex for a given table of nearest landmarks with respect to witnesses.
 * \ingroup witness_complex
 *
 * \tparam Nearest_landmark_table_ needs to be a range of a range of pairs of nearest landmarks and distances.
 *         The range of pairs must admit a member type 'iterator'. The dereference type 
 *         of the pair range iterator needs to be 'std::pair<std::size_t, double>'.
*/
template< class Nearest_landmark_table_ >
class Witness_complex_wmap : public Witness_complex<Nearest_landmark_table_> {
private:
  typedef typename Nearest_landmark_table_::value_type               Nearest_landmark_range;
  typedef std::size_t                                                Witness_id;
  typedef std::size_t                                                Landmark_id;
  typedef std::pair<Landmark_id, double>                             Id_distance_pair;
  typedef Active_witness<Id_distance_pair, Nearest_landmark_range>   ActiveWitness;
  typedef std::list< ActiveWitness >                                 ActiveWitnessList;
  typedef std::vector< Landmark_id >                                 typeVectorVertex;
  typedef std::vector<Nearest_landmark_range>                        Nearest_landmark_table_internal;

  typedef Landmark_id Vertex_handle;

 protected:
  Nearest_landmark_table_internal              nearest_landmark_table_;
  
 public:
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /* @name Constructor
   */

  //@{

  Witness_complex_wmap()
  {
  }
  
  /**
   *  \brief Initializes member variables before constructing simplicial complex.
   *  \details Records nearest landmark table.
   *  @param[in] nearest_landmark_table needs to be a range of a range of pairs of nearest landmarks and distances.
 *         The range of pairs must admit a member type 'iterator'. The dereference type 
 *         of the pair range iterator needs to be 'std::pair<std::size_t, double>'.
   */

  Witness_complex_wmap(Nearest_landmark_table_ const & nearest_landmark_table)
    : nearest_landmark_table_(std::begin(nearest_landmark_table), std::end(nearest_landmark_table))
  {
  }

    
  /** \brief Outputs the (weak) witness complex of relaxation 'max_alpha_square'
   *         in a simplicial complex data structure.
   *  \details The function returns true if the construction is successful and false otherwise.
   *  @param[out] complex Simplicial complex data structure compatible which is a model of
   *              SimplicialComplexForWitness concept.
   *  @param[in] max_alpha_square Maximal squared relaxation parameter.
   *  @param[in] limit_dimension Represents the maximal dimension of the simplicial complex
   *         (default value = no limit).
   */
  template < typename SimplicialComplexForWitness >
  bool create_complex(SimplicialComplexForWitness& complex,
                      double  max_alpha_square,
                      std::size_t limit_dimension = std::numeric_limits<std::size_t>::max())    
  {
    typedef typename SimplicialComplexForWitness::Vertex_handle Vertex_handle;
    typedef Sib_vertex_pair<SimplicialComplexForWitness, Vertex_handle> Simplex_key;

    typedef Witness_for_simplex<typename ActiveWitness::iterator, ActiveWitnessList> Witnessed_simplex;
    typedef std::list<Witnessed_simplex> Witnessed_simplex_list;
    typedef std::map<Simplex_key, Witnessed_simplex_list> Simplex_witness_list_map;
    
    if (complex.num_vertices() > 0) {
      std::cerr << "Witness complex cannot create complex - complex is not empty.\n";
      return false;
    }
    if (max_alpha_square < 0) {
      std::cerr << "Witness complex cannot create complex - squared relaxation parameter must be non-negative.\n";
      return false;
    }
    if (limit_dimension < 0) {
      std::cerr << "Witness complex cannot create complex - limit dimension must be non-negative.\n";
      return false;
    }
    
    ActiveWitnessList active_witnesses;
    for (auto w: nearest_landmark_table_)
      active_witnesses.push_back(ActiveWitness(w));

    Simplex_witness_list_map* prev_dim_map = new Simplex_witness_list_map();
    fill_vertices(max_alpha_square, complex, active_witnesses, prev_dim_map);
    
    //std::cout << "Size of the vertex map: " << prev_dim_map->size() << std::endl;
    // //----------------------- TEST
    // for (auto vw_pair: *prev_dim_map) {
    //   std::cout << "*";
    //   for (auto v: complex.simplex_vertex_range(vw_pair.first.simplex_handle()))
    //     std::cout << v << " ";
    //   std::cout << "\n[";
    //   for (auto w: vw_pair.second) {
    //     std::cout << "(" << &(*w.witness_)
    //               << ", " << w.last_it_->first
    //               << ", " << w.limit_distance_
    //               << ")";
    //   }
    //   std::cout << "]\n";
    // }
    // //---------------------------

    Simplex_witness_list_map* dim1_map = new Simplex_witness_list_map();
    fill_edges(max_alpha_square, complex, active_witnesses, prev_dim_map, dim1_map);
    delete prev_dim_map;
    prev_dim_map = dim1_map;

    // //----------------------- TEST
    // for (auto vw_pair: *prev_dim_map) {
    //   std::cout << "*";
    //   for (auto v: complex.simplex_vertex_range(vw_pair.first.simplex_handle()))
    //     std::cout << v << " ";
    //   std::cout << "\n[";
    //   for (auto w: vw_pair.second) {
    //     std::cout << "(" << &(*w.witness_)
    //               << ", " << w.last_it_->first
    //               << ", " << w.limit_distance_
    //               << ")";        
    //   }
    //   std::cout << "]\n";
    // }

    Landmark_id k = 2; /* current dimension in iterative construction */

    /* test */
    // typedef Gudhi::Simplex_tree_vertex_subtree_iterator<SimplicialComplexForWitness> Vertex_subtree_iterator;
    // typedef boost::iterator_range<Vertex_subtree_iterator> Vertex_subtree_range;
    // for (auto v0: complex.complex_vertex_range()) {
    //   std::cout << "*" << v0 << "\n";
    //   for (auto sh: Vertex_subtree_range(Vertex_subtree_iterator(&complex, v0, 1), Vertex_subtree_iterator())) {
    //     for (auto v: complex.simplex_vertex_range(sh))
    //       std::cout << v << " ";
    //     std::cout << std::endl;
    //   }
    // }
    
    while (!active_witnesses.empty() && k <= limit_dimension) {
      Simplex_witness_list_map* curr_dim_map = new Simplex_witness_list_map();
      fill_simplices(max_alpha_square, k, complex, active_witnesses, prev_dim_map, curr_dim_map);

      delete prev_dim_map;
      prev_dim_map = curr_dim_map;
      k++;
    }
    delete prev_dim_map;
    complex.set_dimension(k-1);
    return true;
  }

  //@}

 private:

  /* \brief Fills the map "vertex -> witnesses for vertices"
   * It is necessary to go through the aw list two times,
   * because of the use of Simplex_handle as key in the map.
   * With every insertion of a vertex, the previous Simplex_handles
   * are invalidated, therefore it is not possible to build 
   * Simplex_tree and the map at the same time.
   */
  template < typename SimplicialComplexForWitness,
             typename ActiveWitnessList,
             typename SimplexWitnessMap >
  void fill_vertices(const double alpha2,
                     SimplicialComplexForWitness& complex,
                     ActiveWitnessList& aw_list,
                     SimplexWitnessMap* sw_map) const
  {
    typedef typename SimplicialComplexForWitness::Simplex_handle Simplex_handle;
    typedef typename SimplicialComplexForWitness::Vertex_handle Vertex_handle;
    typedef typename SimplicialComplexForWitness::Siblings Siblings;
    typedef Sib_vertex_pair<SimplicialComplexForWitness, Vertex_handle> Simplex_key;
    typedef std::vector<Vertex_handle> Vertex_vector;
    typedef typename SimplexWitnessMap::mapped_type Simplex_witness_list;
    typedef typename Simplex_witness_list::value_type WitnessForSimplex;

    for (auto aw_it = aw_list.begin(); aw_it != aw_list.end(); ++aw_it) {
      typename ActiveWitness::iterator l_it = aw_it->begin();
      typename ActiveWitness::iterator end = aw_it->end();
      double filtration_value = 0;
      double norelax_dist2 = std::numeric_limits<double>::infinity();
      for (; l_it != end && l_it->second - alpha2 <= norelax_dist2; ++l_it) {
        if (l_it->second > norelax_dist2)
          filtration_value = l_it->second - norelax_dist2;
        else
          norelax_dist2 = l_it->second;
        complex.insert_simplex(Vertex_vector(1, l_it->first), filtration_value);
      }
    }
    for (auto aw_it = aw_list.begin(); aw_it != aw_list.end(); ++aw_it) {
      typename ActiveWitness::iterator l_it = aw_it->begin();
      typename ActiveWitness::iterator end = aw_it->end();
      double norelax_dist2 = std::numeric_limits<double>::infinity();
      for (; l_it != end && l_it->second - alpha2 <= norelax_dist2; ++l_it) {
        Simplex_handle sh = complex.find(Vertex_vector(1, l_it->first));
        Siblings* sib = complex.self_siblings(sh);
        Vertex_handle v = sh->first;
        Simplex_key sk(sib,v);
        (*sw_map)[sk].emplace_back(WitnessForSimplex(l_it, aw_it, norelax_dist2)); 
        aw_it->increase();
        if (l_it->second < norelax_dist2)
          norelax_dist2 = l_it->second;
      }
      // std::cout << aw_it->counter() << " "; 
    }
    std::cout << "0-dim active witness list size = " << aw_list.size() << "\n";
    // std::cout << "\n\n";
  }

  /* \brief Fills the map "edges -> witnesses for edges"
   */
  template < typename SimplicialComplexForWitness,
             typename ActiveWitnessList,
             typename SimplexWitnessMap >
  void fill_edges(const double alpha2,
                  SimplicialComplexForWitness& complex,
                  ActiveWitnessList& aw_list,
                  SimplexWitnessMap* dim0_map,
                  SimplexWitnessMap* dim1_map) const
  {
    typedef typename SimplicialComplexForWitness::Simplex_handle Simplex_handle;
    typedef typename SimplicialComplexForWitness::Vertex_handle Vertex_handle;
    typedef typename SimplicialComplexForWitness::Siblings Siblings;
    typedef Sib_vertex_pair<SimplicialComplexForWitness, Vertex_handle> Simplex_key;
    typedef std::vector<Vertex_handle> Vertex_vector;
    typedef typename SimplexWitnessMap::mapped_type Simplex_witness_list;
    typedef typename Simplex_witness_list::value_type WitnessForSimplex;

    for (auto vw_pair: *dim0_map) {
      for (auto w: vw_pair.second) {
        typename ActiveWitness::iterator l_it = w.last_it_;
        typename ActiveWitness::iterator end = w.witness_->end();
        if (l_it != end)
          l_it++;
        double filtration_value = 0;
        double norelax_dist2 = w.limit_distance_;
        for (; l_it != end && l_it->second - alpha2 <= norelax_dist2; ++l_it) {
          if (l_it->second > norelax_dist2)
            filtration_value = l_it->second - norelax_dist2;
          else
            norelax_dist2 = l_it->second;
          auto sv_range = complex.simplex_vertex_range(vw_pair.first.simplex_handle());
          Vertex_vector vertices(sv_range.begin(), sv_range.end());
          vertices.push_back(l_it->first);
          complex.insert_simplex(vertices, filtration_value);
        }
      }
    }
    for (auto vw_pair: *dim0_map) {
      for (auto w: vw_pair.second) {
        typename ActiveWitness::iterator l_it = w.last_it_;
        typename ActiveWitness::iterator end = w.witness_->end();
        if (l_it != end)
          l_it++;
        double norelax_dist2 = w.limit_distance_;
        for (; l_it != end && l_it->second - alpha2 <= norelax_dist2; ++l_it) {
          auto sv_range = complex.simplex_vertex_range(vw_pair.first.simplex_handle());
          Vertex_vector vertices(sv_range.begin(), sv_range.end());
          vertices.push_back(l_it->first);
          Simplex_handle sh = complex.find(vertices);
          Siblings* sib = complex.self_siblings(sh);
          Vertex_handle v = sh->first;
          Simplex_key sk(sib,v);
          (*dim1_map)[sk].emplace_back(WitnessForSimplex(l_it, w.witness_, norelax_dist2));
          w.witness_->increase();
          if (l_it->second < norelax_dist2)
            norelax_dist2 = l_it->second;
        }
        w.witness_->decrease();
        if (w.witness_->counter() == 0)
          aw_list.erase(w.witness_);
      }
    }
    // for (auto aw: aw_list)
    //   std::cout << aw.counter() << " "; 
    std::cout << "1-dim active witness list size = " << aw_list.size() << "\n";
  }

  /* \brief Fills the map "k-simplex -> witnesses for edges"
   */
  template < typename SimplicialComplexForWitness,
             typename ActiveWitnessList,
             typename SimplexWitnessMap >
  void fill_simplices(double alpha2,
                      std::size_t k,
                      SimplicialComplexForWitness& complex,
                      ActiveWitnessList& aw_list,
                      SimplexWitnessMap* prev_dim_map,
                      SimplexWitnessMap* curr_dim_map)
  {
    typedef typename SimplicialComplexForWitness::Vertex_handle Vertex_handle;
    typedef typename SimplicialComplexForWitness::Simplex_handle Simplex_handle;
    typedef typename SimplicialComplexForWitness::Siblings Siblings;
    typedef Sib_vertex_pair<SimplicialComplexForWitness, Vertex_handle> Simplex_key;
    typedef std::vector<Vertex_handle> Vertex_vector;
    typedef typename SimplexWitnessMap::mapped_type Simplex_witness_list;
    typedef typename Simplex_witness_list::value_type WitnessForSimplex;

    //#if defined(DEBUG_TRACES)
    //bool verbose = true;
    //#else
    //bool verbose = false;
    //#endif

    // confirmation of cofaces by witnesses
    for (auto vw_pair: *prev_dim_map) {
      for (auto w: vw_pair.second) {
        typename ActiveWitness::iterator l_it = w.last_it_;
        typename ActiveWitness::iterator end = w.witness_->end();
        if (l_it != end)
          l_it++;
        double norelax_dist2 = w.limit_distance_;
        for (; l_it != end && l_it->second - alpha2 <= norelax_dist2; ++l_it) {
          auto sv_range = complex.simplex_vertex_range(vw_pair.first.simplex_handle());
          Vertex_vector vertices(sv_range.begin(), sv_range.end());
          vertices.push_back(l_it->first);
          double filtration_value = 0;
          // if norelax_dist is infinite, relaxation is 0.
          if (l_it->second > norelax_dist2) 
            filtration_value = l_it->second - norelax_dist2;
          if (all_faces_in(vertices, &filtration_value, complex)) {
            auto sh_bool = complex.insert_simplex(vertices, filtration_value);            
            Simplex_handle sh = sh_bool.first;
            if (sh == complex.null_simplex())
              sh = complex.find(vertices);
            Siblings* sib = complex.self_siblings(sh);
            Vertex_handle v = sh->first;
            Simplex_key sk(sib,v);
            (*curr_dim_map)[sk].emplace_back(WitnessForSimplex(l_it, w.witness_, norelax_dist2));
            w.witness_->increase();
          }          
          if (l_it->second < norelax_dist2)
            norelax_dist2 = l_it->second;
        }
        w.witness_->decrease();
         // if (w.witness_->counter() == 0)
        //   aw_list.erase(w.witness_);
      }
    }
    auto aw_it = aw_list.begin();
    while (aw_it != aw_list.end()) {
      // std::cout << aw_it->counter() << " ";
      if (aw_it->counter() == 0)
        aw_list.erase(aw_it++);
      else
        aw_it++;
    }
    // std::cout << "\n";
    std::cout << k << "-dim active witness list size = " << aw_list.size() << "\n";
    // remove_non_witnessed_simplices(complex, curr_dim_map);
  }
  
};
  
}  // namespace witness_complex

}  // namespace Gudhi

#endif  // WITNESS_COMPLEX_WMAP_H_