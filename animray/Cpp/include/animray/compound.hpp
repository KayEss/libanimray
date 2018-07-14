/*
    Copyright 2014-2018, Kirit Saelensminde.
    http://www.kirit.com/AnimRay

    This file is part of AnimRay.

    AnimRay is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AnimRay is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AnimRay.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef ANIMRAY_COMPOUND_HPP
#define ANIMRAY_COMPOUND_HPP
#pragma once


#include <fost/core>
#include <animray/intersection.hpp>
#include <animray/shader.hpp>
#include <animray/emission.hpp>
#include <tuple>
#include <variant>


namespace animray {


    /**
     * # Compound
     *
     * This geometry type stores one of each of a number of different
     * geometries.
     */


    /// Stores geometry objects of different types
    template< typename O, typename... Os >
    class compound;


    /// Partial specialisation of the intersection type for compound
    template< typename O, typename... Os >
    class intersection<compound<O, Os...>> {
    public:
        /// The type of the local coordinate system
        using local_coord_type =
            typename std::common_type<
                typename O::local_coord_type,
                typename Os::local_coord_type...>::type;
        /// The type of the strike location
        using end_type = typename O::intersection_type::end_type;
        /// The type of the strike location
        using direction_type = typename O::intersection_type::direction_type;

        /// The wrapped intersection
        fostlib::accessors<
            std::variant<
                typename O::intersection_type,
                typename Os::intersection_type...
            >, fostlib::lvalue> wrapped_intersection;

        intersection() {}

        template<typename I>
        intersection(I &&i)
        : wrapped_intersection(std::move(i)) {
        }

        end_type from() const {
            return std::visit([](auto i) { return i.from(); }, wrapped_intersection());
        }

        direction_type direction() const {
            return std::visit([](auto i) { return i.direction(); }, wrapped_intersection());
        }
    };


    namespace detail {
        namespace compound_intersect {
            template<typename Pair>
            Pair collapse(Pair i1, Pair i2) {
                if ( not i1.first ) return i2;
                else if ( not i2.first ) return i1;
                else if ( i1.first.value() < i2.first.value() ) return i2;
                else return i1;
            }
            template<typename Pair, typename... Pack>
            Pair collapse(Pair i1, Pair i2, Pack... ip) {
                return collapse(collapse(i1, i2), ip...);
            }
        }
    }

    /// Stores geometry objects of different types
    template< typename O, typename... Os >
    class compound {
    public:
        /// The type we use to store the instances
        using instances_type = std::tuple<O, Os...>;
        /// The type of the local coordinate system
        using local_coord_type = typename O::local_coord_type;
        /// The intersection type
        using intersection_type = intersection<compound>;

        /// Stores the geometry
        fostlib::accessors<instances_type, fostlib::lvalue> instances;

        /// Forward the intersection check to the geometry instances.
        /// Return the closest intersection, `null` if none are found.
        template<typename R, typename E>
        fostlib::nullable<intersection_type> intersects(
            const R &by, const E epsilon
        ) const {
            using mid_type = std::pair<
                    std::optional<local_coord_type>,
                    std::optional<intersection_type>>;
            return std::apply([&by, epsilon](auto... geom) {
                const auto dot = [&by](auto i) -> mid_type
                {
                    if ( i ) return std::make_pair((i.value().from() - by.from()).dot(), i.value());
                    else return std::make_pair(std::nullopt, std::nullopt);
                };
                return detail::compound_intersect::collapse<mid_type>(
                    dot(geom.intersects(by, epsilon))...);
            }, instances()).second;
        };

        /// Calculate whether this object occludes the ray or not
        template< typename R >
        bool occludes(const R &by, const local_coord_type epsilon) const {
//             return occlusion_calculation<1 + sizeof...(Os), 0>()(*this, by, epsilon);
            return false;
        }

//         template< std::size_t left, std::size_t item >
//         struct occlusion_calculation {
//             template<typename R>
//             bool operator () (
//                 const compound &geometry, const R &by,
//                 const local_coord_type epsilon
//             ) const {
//                 return std::get<item>(geometry.instances()).occludes(by, epsilon) ||
//                     occlusion_calculation<left - 1, item + 1>()(geometry, by, epsilon);
//             }
//         };
//         template< std::size_t item >
//         struct occlusion_calculation< 1, item > {
//             template<typename R>
//             bool operator () (
//                 const compound &geometry, const R &by,
//                 const local_coord_type epsilon
//             ) const {
//                 return std::get<item>(geometry.instances()).occludes(by, epsilon);
//             }
//         };
    };


//     template<typename C, typename O, typename RI, typename RL,
//         typename G, typename... Os>
//     struct surface_interaction<C, intersection<compound<O, Os...>>, RI, RL, G> {
//         struct forwarder : public boost::static_visitor<C>{
//             const RI &observer;
//             const RL &light;
//             const C &incident;
//             const G &geometry;
//             forwarder(const RI &observer, const RL &light,
//                     const C &incident, const G &geometry)
//             : observer(observer), light(light), incident(incident),
//                     geometry(geometry) {
//             }
//
//             template<typename I>
//             C operator () (const I &inter) const {
//                 return shader(observer, light, inter, incident, geometry);
//             }
//         };
//         surface_interaction() {}
//         C operator() (
//             const RI &observer, const RL &light,
//             const intersection< compound<O, Os...> > &intersection,
//             const C &incident, const G &geometry
//         ) const {
//             return C{};
//         }
//     };


    /**
     * Returns the emission characteristics for the light that was
     * struck by the intersection.
     */
    template<typename C, typename O, typename RI,
        typename G, typename... Os>
    struct surface_emission<C, RI, intersection<compound<O, Os...>>, G> {
        surface_emission() {}
        C operator() (
            const RI &observer,
            const intersection<compound<O, Os...>> &intersection,
            const G &geometry
        ) const {
            return std::visit(
                [&observer, &geometry](const auto &inter) {
                    return emission<C>(observer, inter, geometry);
                }, intersection.wrapped_intersection());
        }
    };


}


#endif // ANIMRAY_COMPOUND_HPP
