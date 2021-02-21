/**
    Copyright 2014-2021, [Kirit Saelensminde](https://kirit.com/AnimRay).

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


#include <fost/main>
#include <fost/progress-cli>
#include <fost/unicode>
#include <animray/affine.hpp>
#include <animray/camera/flat-jitter.hpp>
#include <animray/camera/pinhole.hpp>
#include <animray/cli/progress.hpp>
#include <animray/compound.hpp>
#include <animray/geometry/quadrics/sphere-unit.hpp>
#include <animray/geometry/collection.hpp>
#include <animray/intersection.hpp>
#include <animray/library/lights/block.hpp>
#include <animray/maths/angles.hpp>
#include <animray/movable.hpp>
#include <animray/scene.hpp>
#include <animray/shader.hpp>
#include <animray/surface/matte.hpp>
#include <animray/surface/gloss.hpp>
#include <animray/surface/reflective.hpp>


FSL_MAIN("animray", "AnimRay. Copyright 2010-2021 Kirit Saelensminde")
(fostlib::ostream &, fostlib::arguments &args) {
    std::size_t const threads(
            fostlib::coerce<fostlib::nullable<int>>(args.commandSwitch("t"))
                    .value_or(std::thread::hardware_concurrency()));
    std::size_t const samples(
            fostlib::coerce<int>(args.commandSwitch("ss").value_or("6")));
    std::size_t const spheres(
            fostlib::coerce<int>(args.commandSwitch("sp").value_or("10")));
    double const focal_length{fostlib::coerce<double>(
            args.commandSwitch("focal").value_or("0.05"))};

    auto const output_filename = fostlib::coerce<std::filesystem::path>(
            args[1].value_or("spheres-white.tga"));
    const int width = fostlib::coerce<int>(args[2].value_or("150"));
    const int height = fostlib::coerce<int>(args[3].value_or("100"));

    using world = double;
    const world aspect = double(width) / height;
    const world fw = width > height ? aspect * 0.024 : 0.024;
    const world fh = width > height ? 0.024 : 0.024 / aspect;

    using spheres_type =
            animray::collection<animray::unit_sphere<animray::point3d<world>>>;
    spheres_type metallic, glossy;

    std::default_random_engine generator;
    std::uniform_int_distribution<int> surface(1, 2);
    std::uniform_real_distribution<world> x_position(-20, 20),
            y_position(-20, 20);
    for (std::size_t count{}; count != spheres; ++count) {
        animray::translate<world> location(
                x_position(generator), y_position(generator), 0.0);
        animray::unit_sphere<animray::point3d<world>> s;
        s.position = location();
        switch (surface(generator)) {
        case 1: metallic.insert(s); break;
        case 2: glossy.insert(s); break;
        }
    }

    using reflective_sphere_type = animray::movable<animray::surface<
            animray::unit_sphere<animray::point3d<world>>,
            animray::reflective<float>, animray::matte<animray::rgb<float>>>>;
    using metallic_spheres_type = animray::surface<
            animray::collection<animray::unit_sphere<animray::point3d<world>>>,
            animray::reflective<animray::rgb<float>>>;
    using gloss_spheres_type = animray::surface<
            animray::collection<animray::unit_sphere<animray::point3d<world>>>,
            animray::gloss<world>, animray::matte<animray::rgb<float>>>;

    animray::compound<
            reflective_sphere_type, metallic_spheres_type, gloss_spheres_type>
            geometry;

    const world scale(200.0);
    std::get<0>(geometry.instances) =
            reflective_sphere_type{
                    animray::unit_sphere<animray::point3d<world>>{}, 0.4f,
                    animray::rgb<float>(0.3f)}(
                    animray::translate<world>(0.0, 0.0, scale + 1.0))(
                    animray::scale<world>(scale, scale, scale));
    std::get<1>(geometry.instances) = metallic_spheres_type{
            std::move(metallic), animray::rgb<float>(1, 1, 1)};
    std::get<2>(geometry.instances) = gloss_spheres_type{
            std::move(glossy), 10, animray::rgb<float>(1, 1, 1)};

    animray::scene scene{
            std::move(geometry), animray::library::lights::wide_block<world>,
            animray::rgb<float>(20, 70, 100)};

    animray::movable<
            animray::pinhole_camera<
                    animray::ray<world>, animray::flat_jitter_camera<world>>,
            animray::ray<world>>
            camera(fw, fh, width, height, focal_length);
    camera(animray::rotate_x<world>(-65_deg))(
            animray::translate<world>(0.0, -4.0, -40));

    using film_type = animray::film<animray::rgb<uint8_t>>;

    animray::cli_render<film_type>(
            output_filename, threads, width, height,
            [samples, &scene, &camera](
                    const film_type::size_type x, const film_type::size_type y) {
                animray::rgb<float> photons;
                for (std::size_t sample{}; sample != samples; ++sample) {
                    photons += scene(camera, x, y) /= samples;
                }
                const float exposure = 1.4f;
                photons /= exposure;
                return animray::rgb<uint8_t>(
                        uint8_t(photons.red() > 255 ? 255 : photons.red()),
                        uint8_t(photons.green() > 255 ? 255 : photons.green()),
                        uint8_t(photons.blue() > 255 ? 255 : photons.blue()));
            });

    return 0;
}
