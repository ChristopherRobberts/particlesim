#ifndef __CS267_COMMON_H__
#define __CS267_COMMON_H__

#include <iostream>

//
// particle data structure
//

typedef struct
{
    double x;
    double y;
    double vx;
    double vy;
    double ax;
    double ay;
} particle_t;

void init_particles( int n, particle_t *p );
void apply_force( particle_t &particle, particle_t &neighbor );
void move( particle_t &p );
void copy_particle( particle_t *src, particle_t *dst);
void print_particle( particle_t );

//
//  parameters
//

const int NSTEPS = 1000;
const int DEF_SPEED = 1;

void init_parameters(int n, int s, char * save, bool vis, int r_size);

//
//  timing routines
//
double read_timer( );

//
//  I/O routines
//
void save(particle_t *p);
void save_file();
std::string working_dir();

//
//  argument processing routines
//
int find_option( int argc, char **argv, const char *option );
int read_int( int argc, char **argv, const char *option, int default_value );
char *read_string( int argc, char **argv, const char *option, char *default_value );

//
// visualizer
//

void run_visualizer();

#endif
