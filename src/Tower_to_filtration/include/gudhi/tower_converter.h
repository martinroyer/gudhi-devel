/*    This file is part of the Gudhi Library. The Gudhi library
 *    (Geometric Understanding in Higher Dimensions) is a generic C++
 *    library for computational topology.
 *
 *    Author(s):       Hannah Schreiber
 *
 *    Copyright (C) 2018  INRIA Sophia Antipolis-Méditerranée (France)
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

#ifndef TOWER_CONVERTER_H
#define TOWER_CONVERTER_H

/** @file tower_converter.h
 * @brief Contains @ref Gudhi::tower_to_filtration::Tower_converter<ComplexStructure> class.
 */

#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace Gudhi {
namespace tower_to_filtration {

template<class ComplexStructure>
/**
 * @brief Takes the elementary operations of a tower in order and convert them into an equivalent filtration.
 */
class Tower_converter
{
public:
    using vertex = double;
    using index = double;
    using simplex_base = std::vector<vertex>;

    /**
     * @brief Enumeration of the types of operations.
     */
    enum operationType : int {
        INCLUSION,      /**< Elementary inclusion. */
        CONTRACTION,    /**< Elementary contraction. */
        COMMENT         /**< Comment or similar to be ignored (e.g. useful when reading a file). */
    };
    /**
     * @brief Enumeration for the streaming format.
     */
    enum streamingType : int {
        FACES,      /**< Simplices will be represented by the identifiers of their facets in the output. */
        VERTICES    /**< Simplices will be represented by the identifiers of their vertices in the output. */
    };

    /**
     * @brief Constructor without parameters.
     *
     * Initializes the members. The output option is set as "no output".
     */
    Tower_converter();
    /**
     * @brief Full constructor.
     *
     * Initializes the members. The output stream will be redirected to @p outputFileName with the output format @p type.
     *
     * @param outputFileName File name for the output.
     * @param type Output format. By default it is #VERTICES. See Enumeration #streamingType.
     */
    Tower_converter(std::string outputFileName, streamingType type = VERTICES);
    ~Tower_converter();

    /**
     * @brief Add an elementary insertion as the next tower operation and convert it into the output stream.
     * @param simplex simplex to be inserted, represented as a vector of its vertex identifiers in increasing order.
     * @param timestamp time value or filtration value which will be associated to the operation in the filtration. Has to be equal or higher to the precedent ones.
     * @return true if the insertion is successful, false otherwise.
     */
    bool add_insertion(std::vector<double> *simplex, double timestamp);
    /**
     * @brief Add an elementary insertion as the next tower operation and convert it into the output stream.
     * @param simplex simplex to be inserted, represented as a vector of its vertex identifiers in increasing order.
     * @param timestamp time value or filtration value which will be associated to the operation in the filtration. Has to be equal or higher to the precedent ones.
     * @param simplexBoundary pointer to an (empty) vector, where the identifiers of the boundary of the inserted simplex will be stored.
     * @param simplexInsertionNumber pointer to an identifier, which will be replaced by the one of the inserted simplex.
     * @return true if the insertion is successful, false otherwise.
     */
    bool add_insertion(std::vector<double> *simplex, double timestamp, std::vector<index> *simplexBoundary, index *simplexInsertionNumber);
    /**
     * @brief Add an elementary contraction as the next tower operation and convert it into a equivalent sequence of insertions into the output stream.
     * @param v identifier of the contracted vertex which disapears.
     * @param u identifier of the contracted vertex which remains.
     * @param timestamp time value or filtration value which will be associated to the operation in the filtration. Has to be equal or higher to the precedent ones.
     * @return The identifier of the first new simplex in the equivalent insertion ; the remaining new simplices will take the identifiers which follows continuously.
     */
    index add_contraction(double v, double u, double timestamp);
    /**
     * @brief Add an elementary contraction as the next tower operation and convert it into a equivalent sequence of insertions into the output stream.
     * @param v identifier of the contracted vertex which disapears.
     * @param u identifier of the contracted vertex which remains.
     * @param timestamp time value or filtration value which will be associated to the operation in the filtration. Has to be equal or higher to the precedent ones.
     * @param addedBoundaries pointer to an (empty) vector, where the boundaries of the inserted simplices will be stored.
     * @param removedIndices pointer to an (empty) vector, where the identifiers of the simplices which become inactive will be stored.
     * @return The identifier of the first new simplex in the equivalent insertion ; the remaining new simplices will take the identifiers which follows continuously.
     */
    index add_contraction(double v, double u, double timestamp, std::vector<std::vector<index>*> *addedBoundaries, std::vector<index> *removedIndices);

    /**
     * @brief Returns the current size of the filtration.
     * @return The current size of the filtration.
     */
    double get_filtration_size() const;
    /**
     * @brief Returns the maximal size of the complex until now.
     * @return The maximal size of the complex until now.
     */
    double get_tower_width() const;
    /**
     * @brief Prints various information about the filtration in the terminal.
     *
     * Those are: filtration size, maximal size of a complex, maximal dimension of a simplex, tower width.
     */
    void print_filtration_data();

private:
    ComplexStructure *complex_;                     /**< Current complex. */
    std::unordered_map<double, vertex> *vertices_;  /**< Current vertices in the complex. Keeps the coherence between vertex identifiers outside and inside the class. */
    std::ofstream *outputStream_;                   /**< Output file. */
    streamingType streamingType_;                   /**< Output format. See Enumeration `streamingType`. */
    double filtrationSize_;                         /**< Current filtration size. */
    double towerWidth_;                             /**< Current tower width. */

    /**
     * @brief Make the union of a set of simplices and a vertex.
     * @param v vertex identifier to unify.
     * @param simplices vector of simplices to unify with `v`. The simplices will be replaced by the resulting simplices.
     */
    void get_union(vertex v, std::vector<simplex_base*> *simplices);
    /**
     * @brief Writes the simplex as an insertion in the output.
     * @param simplex simplex to be inserted.
     * @param timestamp filtration value of the insertion.
     */
    void stream_simplex(simplex_base *simplex, double timestamp);
};

template<class ComplexStructure>
Tower_converter<ComplexStructure>::Tower_converter() : streamingType_(VERTICES), filtrationSize_(0), towerWidth_(0)
{
    outputStream_ = NULL;
    vertices_ = new std::unordered_map<double, vertex>();
    complex_ = new ComplexStructure();
}

template<class ComplexStructure>
Tower_converter<ComplexStructure>::Tower_converter(std::string outputFileName, streamingType type) : streamingType_(type), filtrationSize_(0), towerWidth_(0)
{
    outputStream_ = new std::ofstream(outputFileName);
    vertices_ = new std::unordered_map<double, vertex>();
    complex_ = new ComplexStructure();
}

template<class ComplexStructure>
Tower_converter<ComplexStructure>::~Tower_converter()
{
    if (outputStream_ != NULL) delete outputStream_;
    delete vertices_;
    delete complex_;
}

template<class ComplexStructure>
inline bool Tower_converter<ComplexStructure>::add_insertion(std::vector<double> *simplex, double timestamp)
{
    return add_insertion(simplex, timestamp, NULL, NULL);
}

template<class ComplexStructure>
bool Tower_converter<ComplexStructure>::add_insertion(std::vector<double> *simplex, double timestamp, std::vector<index> *simplexBoundary, index *simplexInsertionNumber)
{
    simplex_base transSimplex;

    if (simplex->size() == 1){
        vertices_->emplace(simplex->at(0), simplex->at(0));
        transSimplex.push_back(simplex->at(0));
    } else {
        for (simplex_base::size_type i = 0; i < simplex->size(); i++){
            transSimplex.push_back(vertices_->at(simplex->at(i)));
        }
        std::sort(transSimplex.begin(), transSimplex.end());
    }

    if (complex_->insert_simplex(&transSimplex)) {
        stream_simplex(&transSimplex, timestamp);
        if (complex_->get_size() > towerWidth_) towerWidth_ = complex_->get_size();
        if (simplexBoundary != NULL) *simplexInsertionNumber = complex_->get_boundary(&transSimplex, simplexBoundary);
        return true;
    }
    return false;
}

template<class ComplexStructure>
inline typename Tower_converter<ComplexStructure>::index Tower_converter<ComplexStructure>::add_contraction(double v, double u, double timestamp)
{
    return add_contraction(v, u, timestamp, NULL, NULL);
}

template<class ComplexStructure>
typename Tower_converter<ComplexStructure>::index Tower_converter<ComplexStructure>::add_contraction(double v, double u, double timestamp,
                                                                          std::vector<std::vector<index>*> *addedBoundaries, std::vector<index> *removedIndices)
{
    std::vector<simplex_base*> closedStar;
    vertex tv = vertices_->at(v), tu = vertices_->at(u);
    vertex dis = complex_->get_smallest_closed_star(tv, tu, &closedStar);
    simplex_base vdis(1, dis);
    index first = -1;

    vertices_->erase(v);
    if (dis == tu){
        vertices_->at(u) = tv;
        get_union(tv, &closedStar);
    } else {
        get_union(tu, &closedStar);
    }

    for (auto it = closedStar.begin(); it != closedStar.end(); it++){
        if (complex_->insert_simplex(*it)) {
            stream_simplex(*it, timestamp);
            if (addedBoundaries != NULL){
                std::vector<index> *boundary = new std::vector<index>();
                if (first == -1) first = complex_->get_max_index();
                complex_->get_boundary(*it, boundary);
                addedBoundaries->push_back(boundary);
            }
        }
        delete *it;
    }
    complex_->remove_simplex(&vdis, removedIndices);

    if (complex_->get_size() > towerWidth_) towerWidth_ = complex_->get_size();

    return first;
}

template<class ComplexStructure>
inline double Tower_converter<ComplexStructure>::get_filtration_size() const
{
    return filtrationSize_;
}

template<class ComplexStructure>
inline double Tower_converter<ComplexStructure>::get_tower_width() const
{
    return towerWidth_;
}

template<class ComplexStructure>
inline void Tower_converter<ComplexStructure>::print_filtration_data()
{
    std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1) << "Filtration Size: " << filtrationSize_ << "\n";
    std::cout << "Max Size: " << complex_->get_max_size() << "\n";
    std::cout << "Max Dimension: " << complex_->get_max_dimension() << "\n";
    std::cout << "Tower Width: " << towerWidth_ << "\n";
}

template<class ComplexStructure>
void Tower_converter<ComplexStructure>::get_union(vertex v, std::vector<simplex_base*> *simplices)
{
    for (auto itSimplices = simplices->begin(); itSimplices != simplices->end(); itSimplices++){
        auto itVertices = (*itSimplices)->begin();
        while (itVertices != (*itSimplices)->end() && *itVertices < v) itVertices++;
        if ((itVertices != (*itSimplices)->end() && *itVertices != v) || itVertices == (*itSimplices)->end()){
            (*itSimplices)->insert(itVertices, v);
        }
    }
}

template<class ComplexStructure>
void Tower_converter<ComplexStructure>::stream_simplex(simplex_base *simplex, double timestamp)
{
    filtrationSize_++;
    if (outputStream_ == NULL) return;
    simplex_base::size_type size = simplex->size();
    *outputStream_ << std::setprecision(std::numeric_limits<double>::digits10 + 1) << (size - 1) << " ";
    if (streamingType_ == FACES){
        if (size > 1){
            std::vector<index> boundary;
            complex_->get_boundary(simplex, &boundary);
            for (std::vector<index>::size_type i = 0; i < size; i++){
                *outputStream_ << boundary.at(i) << " ";
            }
        }
    } else {
        for (simplex_base::size_type i = 0; i < size; i++){
            *outputStream_ << simplex->at(i) << " ";
        }
    }
    *outputStream_ << timestamp << "\n";
}

}
}

#endif // TOWER_CONVERTER_H