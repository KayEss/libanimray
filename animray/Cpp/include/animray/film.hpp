/*
    Copyright 1995-2010, Kirit Saelensminde.
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


#ifndef ANIMRAY_FILM_HPP
#define ANIMRAY_FILM_HPP
#pragma once


#include <animray/extents2d.hpp>


namespace animray {


    /// A film represents a raster of pixel data
    template< typename C, typename E = uint16_t >
    class film : boost::noncopyable {
        typedef std::vector< C > row_type;
        typedef std::vector< row_type > rows_type;
        rows_type rows;
        public:
            /// The colour type
            typedef C color_type;
            /// The type of the extents co-ordinates
            typedef E extents_value_type;
            /// The extents type
            typedef extents2d< extents_value_type > extents_type;
            /// The extents size type
            typedef typename extents_type::size_type size_type;

            /// Construct an empty targa of the given size
            film( size_type width, size_type height, const C &colour = C() )
            : rows( height, row_type(width, colour)), width( width ), height( height ) {
            }

            /// The width of the image
            fostlib::accessors< const size_type > width;
            /// The height of the image
            fostlib::accessors< const size_type > height;
            /// Return the extents of the image
            extents_type size() const {
                return extents_type( 0, 0, width() - 1, height() - 1 );
            }

            /// Return a mutable row
            row_type &operator [] ( size_type r ) {
                return rows[r];
            }
            /// Return a non-mutable row
            const row_type &operator [] ( size_type r ) const {
                return rows[r];
            }

            /// Iterate the given function across the image and allow it to mutate the image
            void transform(
                boost::function3<
                    color_type, size_type, size_type, const color_type &
                > fn
            ) {
                for_each( fn, size() );
            }
            /// Iterate the given function across the image and allow it to mutate the image
            void transform(
                boost::function3<
                    color_type, size_type, size_type, const color_type &
                > fn, const extents_type &area
            ) {
                for ( size_type r = area.lower_left().y(); r <= area.top_right().y(); ++r )
                    for ( size_type c = area.lower_left().x(); c < area.top_right().x(); ++c )
                        rows[r][c] = fn( c, r, rows[r][c] );
            }
            /// Iterate the function across the image rows/columns
            void for_each(
                boost::function1<
                    void, const color_type &
                > fn
            ) const {
                for ( size_type r = 0; r < height(); ++r )
                    for ( size_type c = 0; c < width(); ++c )
                        fn( rows[r][c] );
            }
    };


}


#endif // ANIMRAY_FILM_HPP
