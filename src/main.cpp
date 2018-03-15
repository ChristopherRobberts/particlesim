#include <zconf.h>
#include "common.h"
#include "pthread.h"
//
//  benchmarking program
//

int main( int argc, char **argv )
{
    if( find_option( argc, argv, "-h" ) >= 0 )
    {
        printf( "Options:\n" );
        printf( "-h to see this help\n" );
        printf( "-n <int> to set the number of particles (default: 1000)\n" );
        printf( "-o <filename> to specify the output file name\n" );
        printf( "-s <int> to specify the size of the simulation area (default: 5)\n" );
        printf( "-t <int> slows the simulation down by given factor (default: 1)\n" );
        printf( "-v to run visualiser after simulation (default: off)\n" );

        return 0;
    }

    int n = read_int( argc, argv, "-n", 1000 );
    int s = read_int( argc, argv, "-t", DEF_SPEED);
    int size = read_int( argc, argv, "-s", 0);

    bool vis = (0 < find_option( argc, argv, "-v"));
    char *savename = read_string( argc, argv, "-o", NULL);

    init_parameters(n, s, savename, vis, size);


    //
    //  simulate a number of time steps
    //

    double simulation_time = read_timer();
    perform_steps(n, (savename != NULL));
    simulation_time = read_timer() - simulation_time;

    printf( "n = %d, simulation time = %g seconds\n", n, simulation_time );

    if (savename != NULL) {
        save_file();
    }
    return 0;
}

