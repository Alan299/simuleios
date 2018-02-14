/*-------------euler.cpp------------------------------------------------------//
*
* Purpose: To implement a basic version of euler methods for random things
*
*-----------------------------------------------------------------------------*/

#include <iostream>
#include <vector>
#include "../visualization/cairo/vec.h"
#include "../visualization/cairo/cairo_vis.h"

// Simple struct to hold a moving particle
struct Particle{
    vec pos, vel, acc;
};

// Function to initialize the scene
std::vector<frame> init_scene(){
    vec res = {600, 400};
    int fps = 30;
    color bg_clr = {0,0,0,1};

    return init_layers(3, res, fps, bg_clr);
}

// Function to map coordinates
vec map_coordinates(vec pos, vec max, vec res){
    vec double_pos = {(pos.x / max.x) + 0.5*max.x,
                      (pos.y / max.y) + 0.5*max.y,
                      0};
    return {res.x * double_pos.x / max.x,
            res.y * double_pos.y / max.y, 0};
    
}

vec find_acc(Particle &planet, Particle &ball){
    vec distance = planet.pos - ball.pos;
    vec unit_vector = {distance.x/length(distance), 
                       distance.y/length(distance),
                       distance.z/length(distance)};
    double diff = length(distance);
    double acc_mag = 1/(diff*diff);
    return acc_mag*distance;
}

// We will be returning the new position of the ball after moving forward 1 dt
void euler_ball(Particle &ball, double dt){
    ball.vel += ball.acc*dt;

    ball.pos += ball.vel*dt;
}

void visualize_euler_ball(){

    Particle planet;

    planet.acc = {0,0,0};
    planet.vel = {0,0,0};
    planet.pos = {0,0,0};

    std::vector<Particle> balls(5);
    for (Particle &ball : balls){
        ball.acc = {0,0,0};
        ball.vel = {0,1,0};
        ball.pos = {5,0,0};
    }

    balls[1].vel = {1,0,0};
    balls[1].pos = {0,5,0};

    balls[2].vel = {0.5,-0.5,0};
    balls[2].pos = {4,4,0};

    balls[3].vel = {1.5,0,0};
    balls[3].pos = {0,5,0};

    balls[4].vel = {-0.5,0.5,0};
    balls[4].pos = {-4,-4,0};

    double threshold = 0.1;
    double dt = 0.1;

    std::vector<frame> layers = init_scene();
    color white = {1,1,1,1};

    vec max = {5, 5, 0};
    vec res = {layers[0].res_x, layers[0].res_y, 0};
    vec pos = map_coordinates(planet.pos, max, res);
    grow_circle(layers[1], 1, pos, 50, white);

    for (Particle &ball : balls){
        pos = map_coordinates(ball.pos, max, res);
        grow_circle(layers[1], 1, 100, 200, pos, 10, white);
    }

    //while (distance(planet.pos, ball.pos) > threshold){
    for(int i = 0; i < 800; ++i){
        for (Particle &ball : balls){
            ball.acc = find_acc(planet, ball);
            euler_ball(ball, dt);
            vec mapped_coord = map_coordinates(ball.pos, max, res);
            draw_filled_circle(layers[1], mapped_coord, 10, 200 + i, white);
            print(ball.pos);
        }
        std::cout << '\n';
    }

    draw_layers(layers);
}

int main(){
    visualize_euler_ball();
}