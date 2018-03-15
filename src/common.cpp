#include "common.h"
#include <string.h>
#include <sys/time.h>
#include <cmath>
#include "windows.h"
#include <random>

//
//  tuned constants
//
#define density 0.0005
#define mass    0.01
#define cutoff  0.01
#define min_r   (cutoff/100)
#define dt      0.0005

FILE * c_fsave;
std::string c_save_name;

double c_size;
double c_requested_size;
double c_size_coef;

int c_nof_particles;

double c_speed;

bool c_visualize = false;

void init_parameters(int n, int s, char * save, bool vis, int r_size) {
    c_size = sqrt( density * n );
    c_requested_size = (r_size == 0) ? c_size : r_size;
    c_nof_particles = n;
    c_size_coef =  c_requested_size / c_size;
    c_visualize = vis;

    c_speed = s*c_size_coef;

    if (save != NULL) {
        printf("hej");
        c_fsave = fopen(save, "w");
        c_save_name = std::string(save);

    }
}

//
//  timer
//
double read_timer()
{
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

//
//  I/O routines
//
void save(particle_t *p)
{
    static bool first = true;

    if( first )
    {
        fprintf( c_fsave, "%d %g\n", c_nof_particles, c_requested_size );
        first = false;
    }
    for( int i = 0; i < c_nof_particles; i++ )
        fprintf( c_fsave, "%g %g\n", p[i].x * c_size_coef, p[i].y * c_size_coef);
}
void save_file() {
    if( c_fsave ) {
        fclose(c_fsave);

        if (c_visualize) {
            run_visualizer();
        }
    }
}
std::string working_dir() {
    char buf[256];
    GetCurrentDirectoryA(256, buf);
    return std::string(buf) + "\\";
}

//
//  command line option processing
//
int find_option( int argc, char **argv, const char *option )
{
    for( int i = 1; i < argc; i++ )
        if( strcmp( argv[i], option ) == 0 )
            return i;
    return -1;
}

int read_int( int argc, char **argv, const char *option, int default_value )
{
    int iplace = find_option( argc, argv, option );
    if( iplace >= 0 && iplace < argc-1 )
        return atoi( argv[iplace+1] );
    return default_value;
}

char *read_string( int argc, char **argv, const char *option, char *default_value )
{
    int iplace = find_option( argc, argv, option );
    if( iplace >= 0 && iplace < argc-1 )
        return argv[iplace+1];
    return default_value;
}

//
// Start visualizer
//

void run_visualizer() {
    /* build path to program and savefile */
    std::string path = working_dir();
    std::string arg = path + c_save_name;
    path.replace(path.length()-18, 18, "util\\visualize.exe");

    ShellExecute(NULL, "open", path.c_str(), arg.c_str(), NULL, SW_SHOWDEFAULT);
}

//
// particle code;
//



//
// copies particle from src to dst
//
void copy_particle(particle_t * src, particle_t * dst) {

    dst->x = src->x;
    dst->y = src->y;

    dst->ax = src->ax;
    dst->ay = src->ay;

    dst->vx = src->vx;
    dst->vy = src->vy;
}

void print_particle(particle_t particle) {
    printf("x: %f, y: %f, ax: %f, ay: %f, vx: %f, vy: %f\n", particle.x,  particle.y,  particle.ax,  particle.ay,  particle.vx,  particle.vy);
}

//
//  Initialize the particle positions and velocities
//
void init_particles( int n, particle_t *p )
{

    /* random init code */
    std::random_device device;
    std::mt19937 rnd(device());
    std::uniform_real_distribution<double> dist(1.0, 100.0);

    int sx = (int)ceil(sqrt((double)n));
    int sy = (n+sx-1)/sx;

    int *shuffle = (int*)malloc( n * sizeof(int) );
    for( int i = 0; i < n; i++ )
        shuffle[i] = i;

    for( int i = 0; i < n; i++ )
    {
        //
        //  make sure particles are not spatially sorted
        //
        int j = (int) dist(rnd)%(n-i);
        int k = shuffle[j];
        shuffle[j] = shuffle[n-i-1];

        //
        //  distribute particles evenly to ensure proper spacing
        //
        p[i].x = c_size*(1.+(k%sx))/(1+sx);
        p[i].y = c_size*(1.+(k/sx))/(1+sy);

        //
        //  assign random velocities within a bound
        //
        p[i].vx = dist(rnd)*2-1;
        p[i].vy = dist(rnd)*2-1;
    }
    free( shuffle );
}

//
//  interact two particles
//
void apply_force( particle_t &particle, particle_t &neighbor )
{
    double dx = neighbor.x - particle.x;
    double dy = neighbor.y - particle.y;
    double r2 = dx * dx + dy * dy;

    if( r2 > cutoff*cutoff )
        return;
    r2 = fmax( r2, min_r*min_r );
    double r = sqrt( r2 );

    //
    //  very simple short-range repulsive force
    //
    double coef = ( 1 - cutoff / r ) / r2 / mass;
    particle.ax += coef * dx;
    particle.ay += coef * dy;

}

//
//  integrate the ODE
//
void move( particle_t &p )
{

    //  slightly simplified Velocity Verlet integration
    //  conserves energy better than explicit Euler method
    //
    p.vx += p.ax * dt;
    p.vy += p.ay * dt;
    p.x  += (p.vx * dt)/c_speed;
    p.y  += (p.vy * dt)/c_speed;

    //
    //  bounce from walls
    //
    while( p.x < 0 || p.x > c_size )
    {
        p.x  = p.x < 0 ? -p.x : 2*c_size-p.x;
        p.vx = -p.vx;
    }
    while( p.y < 0 || p.y > c_size )
    {
        p.y  = p.y < 0 ? -p.y : 2*c_size-p.y;
        p.vy = -p.vy;
    }
}

