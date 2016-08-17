/*-------------geometrical.cpp------------------------------------------------//
*
*              geometrical optics
*
* Purpose: to simulate light going through a lens with a variable refractive
*          index. Not wave-like.
*
*   Notes: This file was mostly written by Gustorn. Thanks!
*          Negative refractive indices are in, but I am not confident in their
*              physical interpretation
*
*-----------------------------------------------------------------------------*/

#include <array>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include "optics_vis.h"
#include "geometrical.h"

/*----------------------------------------------------------------------------//
* MAIN
*-----------------------------------------------------------------------------*/

int main() {

    // Creating layers for drawing
    std::vector<frame> layer(3);
    for (size_t i = 0; i < layer.size(); ++i){
        layer[i].create_frame(600, 450, 10, "/tmp/image");
        layer[i].init();

        layer[i].curr_frame = 1;
    }

    // fiding circle radius
    double radius = layer[0].res_y / 3.0;

    // Creating standard black background
    create_bg(layer[0], 0, 0, 0);

    // defines output
    std::ofstream output("geometrical.dat", std::ofstream::out);

    vec dim = {2 * radius, 10};
    double max_vel = 30.0;

    // Implement other lenses and change this line to use them
    vec lens_p = {layer[0].res_x / 2.0, layer[0].res_y / 2.0};
    sphere lens = {radius, lens_p.x, lens_p.y};
    ray_array rays = light_gen(dim, lens, max_vel, 0 /*0.523598776*/,
                               (layer[0].res_y / 2.0) - radius);
    draw_lens(layer[1], 1, lens);
    propagate(rays, lens, 0.0001, max_vel, layer[1]);

    draw_layers(layer);

}

/*----------------------------------------------------------------------------//
* SUBROUTINE
*-----------------------------------------------------------------------------*/

// Refracts the given normalized vector "l", based on the normalized normal "n"
// and the given index of refraction "ior", where ior = n1 / n2
vec refract(vec l, vec n, double ior) {
    double c = dot(-n, l);
    double d = 1.0 - ior * ior * (1.0 - c * c);
/*
    std::cout << "d is: " << d << '\n';
    std::cout << "nx is: " << n.x << '\t' << "lx is: " << l.x << '\n';
    std::cout << "ny is: " << n.y << '\t' << "ly is: " << l.y << '\n';
    std::cout << "ior is: " << ior << '\t' << "c is: " << c << '\n';
*/

    if (d < 0.0) {
        return vec(0.0, 0.0);
    }

    return ior * l + (ior * c - sqrt(d)) * n;
}

vec reflect(vec l, vec n) {
    return l - (2.0 * dot(n, l)) * n;
}

template <typename T>
ray_array light_gen(vec dim, const T& lens, double max_vel, double angle,
                    double offset) {
    ray_array rays;
    vec velocity = vec(cos(angle), sin(angle)) * max_vel;

    // Create rays
    for (size_t i = 0; i < rays.size(); i++) {
        rays[i].p = vec(0.0, offset + i * dim.x / NUM_LIGHTS);
        rays[i].v = velocity;
        rays[i].previous_index = refractive_index_at(lens, rays[i].p);
    }

    return rays;
}

template <typename T>
void propagate(ray_array& rays, const T& lens,
               double step_size, double max_vel,
               frame &anim) {

    //std::cout << "curr_frame is: " << anim.curr_frame << '\n';
    // Temporary position (where the line starts every timestep)
    vec temp_p = vec(0.0, 0.0);
    int start_frame = anim.curr_frame;
    int ray_frame = start_frame;

    // white color for fun
    color white{1,1,1};

    // move simulation every timestep
    for (auto& ray : rays){
        ray_frame = start_frame;
        for (size_t i = 0; i < TIME_RES; i++){
            //temp_p = ray.p;
            if (fabs(ray.p.x) > lens.origin.x + 5 * lens.radius + 1){
                continue;
            }
            ray.p += ray.v * step_size;

            double n1 = ray.previous_index;
            double n2 = refractive_index_at(lens, ray.p);

            // If the ray passed through a refraction index change
            if (n1 != n2) {
                vec n = normal_at(lens, ray.p);
                vec l = normalize(ray.v);
                double ior = n1 / n2;

                if (dot(-n, l) < 0.0) {
                    n = -n;
                }

                vec speed = refract(l, n, ior);

                if (is_null(speed)) {
                    speed = reflect(l, n);
                }

                // Multiply with ior * length(ray.v) to get the proper velocity
                // for the refracted vector
                if (ior > 0){
                    ray.v = normalize(speed) * ior * length(ray.v);
                }
                else{
                    ray.v = -normalize(speed) * ior * length(ray.v);
                }
            }

            ray.previous_index = n2;

            if (i % 2000 == 0 && i != 0){
                animate_line(anim, ray_frame, 1/anim.fps, 
                             temp_p, ray.p, white);
                ray_frame++;
                temp_p = ray.p;
            }
            else if(i == 0){
                temp_p = ray.p;
            }

/*
            if (i % 1000 == 0){
                output << ray.p.x <<'\t'<< ray.p.y << '\t'
                       << ray.v.x <<'\t'<< ray.v.y << '\n';
            }
*/
        }
        //output << '\n' << '\n';
    }

}

// Inside_of functions
// simple lens slab
bool inside_of(const simple& lens, vec p) {
    return p.x > lens.left && p.x < lens.right;
}

// Circle / sphere
bool inside_of(const sphere& lens, vec p) {
    double diff = distance(lens.origin, p);
    return diff < lens.radius;
}

// Find the normal
// Lens slab
vec normal_at(const simple&, vec) {
    return normalize(vec(-1.0, 0.0));
}

// Circle / sphere
// ERROR: This is defined incorrectly!
vec normal_at(const sphere& lens, vec p) {
    //return normalize(vec(-1.0, 0.0));
    return normalize(p - lens.origin);
}

// find refractive index
// Lens slab
double refractive_index_at(const simple& lens, vec p) {
    return inside_of(lens, p) ? 1.4 : 1.0;
}

// Circle / sphere
double refractive_index_at(const sphere& lens, vec p) {
    //return inside_of(lens, p) ? 1.4 : 1.0;

    double index, diff, cutoff;
    cutoff = 0.001;

    if (inside_of(lens, p)){
        double r = distance(lens.origin, p);
        diff = r;
/*
        if (fabs(r) > cutoff){
            index = (1/sqrt(r));
        }
        else{
            index = 100;
        }
*/
        // Formula for invisible lens
        if (fabs(r) > cutoff){
            double a = lens.radius;
            double q = cbrt(-(a/r) + sqrt((a * a) / (r * r) + 1.0 / 27.0));
            index = (q - 1.0 / (3.0 * q)) * (q - 1.0 / (3.0 * q));
        }
        else{
            r = cutoff;
            double a = lens.radius;
            double q = cbrt(-(a/r) + sqrt((a * a) / (r * r) + 1.0 / 27.0));
            index = (q - 1.0 / (3.0 * q)) * (q - 1.0 / (3.0 * q));
        }
        //index = 1.4;
    }
    else{
        index = 1.0;
    }

    return index;
}
