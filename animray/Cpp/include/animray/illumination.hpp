/*
    Copyright 2014, Kirit Saelensminde.
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


#ifndef ANIMRAY_ILLUMINATION_HPP
#define ANIMRAY_ILLUMINATION_HPP
#pragma once


#include <animray/ray.hpp>


namespace animray {


    /// Like a ray, but carries with it an illumination model
    template< typename R, typename C >
    class beam {
        R ray;
    public:
        /// The co-ordinate system precision type
        typedef typename R::value_type value_type;
        /// The ray type for specifying the direction of the beam
        typedef R ray_type;
        /// The colour model for the beam
        typedef C color_type;

        /// Construct a beam
        beam(const ray_type &ray)
        : ray(ray) {
        }
    };

    /// A scene featuring a light and a model
    template< typename G, typename L, typename A, typename B >
    class scene {
    public:
        /// The geometry type
        typedef G geometry_type;
        /// The light type
        typedef L light_type;
        /// The ambient light colour model type
        typedef A ambient_light_type;
        /// The beam type
        typedef B beam_type;

        /// Construct an empty scene
        scene() {}

        /// Store the geometry
        fostlib::accessors<geometry_type, fostlib::lvalue> geometry;
        /// Store the ambient light
        fostlib::accessors<ambient_light_type> ambient;
        /// Store the light
        fostlib::accessors<light_type, fostlib::lvalue> light;

        /// Given a position on the camera film, calculate the colour it should be
        template< typename C, typename S >
        typename beam_type::color_type operator() (const C &camera, S x, S y) const {
            typename beam_type::ray_type r(camera(x, y));
            fostlib::nullable<typename beam_type::ray_type>
                intersection(geometry().intersection(r));
            if ( !intersection.isnull() ) {
                return ambient() + light()(intersection.value(), geometry());
            } else {
                return typename beam_type::color_type(0);
            }
        }
    };


}


#endif // ANIMRAY_ILLUMINATION_HPP
