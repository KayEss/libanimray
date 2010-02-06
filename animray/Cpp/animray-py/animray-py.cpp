/*
    Copyright 2010, Kirit Saelensminde.
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


#include <fost/python>

#include <animray/film.hpp>


BOOST_PYTHON_MODULE( _animray ) {
    using namespace boost::python;
    fostlib::python_string_registration();
    fostlib::python_json_registration();

    class_<
        animray::film< uint8_t >,
        std::auto_ptr< animray::film< uint8_t > >,
        boost::noncopyable
    >(
        "film_gray8",
        init< std::size_t, std::size_t, uint8_t >()
    )
        .add_property("width",
            fostlib::accessors_getter<
                animray::film< uint8_t >,
                const animray::film< uint8_t >::size_type,
                &animray::film< uint8_t >::width
            >
        )
    ;
}
