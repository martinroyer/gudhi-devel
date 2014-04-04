#include <iostream>
#include <fstream>
#include <boost/graph/adjacency_list.hpp>
#include "graph_simplicial_complex.h"

/**
 * \brief Read a set of points to turn it
 * into a vector< vector<double> > by filling points
 *
 * File format: 1 point per line
 * X11 X12 ... X1d 
 * X21 X22 ... X2d
 * etc
 */
inline void
read_points ( std::string file_name
            , std::vector< std::vector< double > > & points)
{  
  std::ifstream in_file (file_name.c_str(),std::ios::in);
  if(!in_file.is_open()) {
    std::cerr << "Unable to open file " << file_name << std::endl;
    return;}

  std::string line;
  double x;
  while( getline ( in_file , line ) )
  {
    std::vector< double > point;
    std::istringstream iss( line );
    while(iss >> x) { point.push_back(x); }
    points.push_back(point);
  }
  in_file.close();
}

/**
 * \brief Read a graph from a file.
 *
 * File format: 1 simplex per line
 * Dim1 X11 X12 ... X1d Fil1 
 * Dim2 X21 X22 ... X2d Fil2
 * etc
 *
 * The vertices must be labeled from 0 to n-1.
 * Every simplex must appear exactly once.
 * Simplices of dimension more than 1 are ignored.
 */
typedef int                                       Vertex_handle;
typedef double                                    Filtration_value;
typedef boost::adjacency_list < boost::vecS, boost::vecS, boost::undirectedS
                              , boost::property < vertex_filtration_t, Filtration_value >
                              , boost::property < edge_filtration_t, Filtration_value > 
                              >                   Graph_t;
typedef std::pair< Vertex_handle, Vertex_handle > Edge_t;
inline Graph_t
read_graph ( std::string file_name )
{
  std::ifstream in_ (file_name.c_str(),std::ios::in);
  if(!in_.is_open()) { std::cerr << "Unable to open file " << file_name << std::endl; }

  std::vector< Edge_t >                       edges;
  std::vector< Filtration_value >             edges_fil;
  std::map< Vertex_handle, Filtration_value > vertices;
  
  std::string      line;
  int              dim;
  Vertex_handle    u,v,max_h = -1;
  Filtration_value fil;
  while( getline ( in_ , line ) )
  {
    std::istringstream iss( line );
    while(iss >> dim) {
      switch ( dim ) {
        case 0 : {
          iss >> u; iss >> fil; 
          vertices[u] = fil;
          if(max_h < u) { max_h = u; }
          break;
        }
        case 1 : {
          iss >> u; iss >> v; iss >> fil;
          edges.push_back(Edge_t(u,v));
          edges_fil.push_back(fil);
          break;
        }
        default: {break;} 
      } 
    }
  }
  in_.close();
  if(max_h+1 != vertices.size()) 
    { std::cerr << "Error: vertices must be labeled from 0 to n-1 \n"; }

  Graph_t skel_graph(edges.begin(),edges.end(),edges_fil.begin(),vertices.size());
  auto vertex_prop = boost::get(vertex_filtration_t(),skel_graph);

  boost::graph_traits<Graph_t>::vertex_iterator vi, vi_end;
  auto v_it = vertices.begin();
  for (tie(vi, vi_end) = boost::vertices(skel_graph); vi != vi_end; ++vi,++v_it)
  { boost::put(vertex_prop, *vi, v_it->second); }

  return skel_graph;
}

/**
 * \brief Read a face from a file.
 *
 * File format: 1 simplex per line
 * Dim1 X11 X12 ... X1d Fil1 
 * Dim2 X21 X22 ... X2d Fil2
 * etc
 *
 * The vertices must be labeled from 0 to n-1.
 * Every simplex must appear exactly once.
 * Simplices of dimension more than 1 are ignored.
 */
template< typename Vertex_handle
        , typename Filtration_value >
bool read_simplex ( std::istream                 & in_                     
                  , std::vector< Vertex_handle > & simplex
                  , Filtration_value             & fil )
{
  int dim;
  in_ >> dim;
  Vertex_handle v;
  for(int i=0; i<dim+1; ++i) 
  {
    if(!(in_ >> v)) { return false; } 
    simplex.push_back(v); 
  }
  return true;
}
