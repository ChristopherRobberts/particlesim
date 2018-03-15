#include <cmath>
#include "pthread.h"

particle_t *particles;             /* main particle array */
particle_vector_t *particle_matrix;
int nof_slices;                     /* number of slices */
int nof_particles;                  /* number of particles */
double size;
pthread_mutex_t index_lock;
pthread_mutex_t work_lock;
pthread_mutex_t barrier_lock;

pthread_cond_t index_cond;
pthread_cond_t work_cond;
pthread_cond_t barrier_cond;
bool indexation_done;
bool work_done;
int thread_counter = 0;
void Barrier() {
    pthread_mutex_lock(&barrier_lock);
    thread_counter++;
    if (thread_counter == NOF_THREADS) {
        thread_counter = 0;
        pthread_cond_broadcast(&barrier_cond);
    } else {
        pthread_cond_wait(&barrier_cond, &barrier_lock);
    }
    pthread_mutex_unlock(&barrier_lock);
}
void *Worker(void *arg) {
    int pid = (int) arg;
    int slice = nof_slices/NOF_THREADS;
    int start = pid*slice;
    int end = start + slice;

    for (int steps = 0; steps < NSTEPS; steps++) {
        pthread_mutex_lock(&index_lock);
        while(!indexation_done) {
            pthread_cond_wait(&index_cond, &index_lock);
        }
        pthread_mutex_unlock(&index_lock);

        for (int i = start; i < end; i++) {
            int comp_start = ((i > 0) ? (i - 1) : i);
            int comp_end = ((i < nof_slices - 1) ? (i + 2) : (i + 1));

            for (particle_t *curr_particle : particle_matrix[i]) {
                (*curr_particle).ax = (*curr_particle).ay = 0;

                for (int j = comp_start; j < comp_end; j++) {
                    for (particle_t *neigh_particle: particle_matrix[j]) {
                        apply_force((*curr_particle), (*neigh_particle));
                    }
                }
            }
        }
        Barrier();
        if (pid == 0) {
            indexation_done = false;
        }

        for (int i = start*10; i < end*10; i++) {
            move(particles[i]);
        }
        Barrier();
        if (pid == 0) {
            pthread_mutex_lock(&work_lock);
            work_done = true;
            pthread_cond_broadcast(&work_cond);
            pthread_mutex_unlock(&work_lock);
        }
    }
}

void init(int n) {
    nof_particles = n;
    nof_slices = (n < 10) ? 1 : n / 10;
    size = sqrt(0.0005 * n);

    particle_matrix = new particle_vector_t[nof_slices];
    particles = new particle_t[nof_particles];
    init_particles(nof_particles, particles);
    pthread_cond_init(&barrier_cond, NULL);
    pthread_cond_init(&index_cond, NULL);
    pthread_cond_init(&work_cond, NULL);

    pthread_mutex_init(&work_lock, NULL);
    pthread_mutex_init(&index_lock, NULL);
    pthread_mutex_init(&barrier_lock, NULL);
    indexation_done = false;
    work_done = false;

    pthread_t threads[NOF_THREADS];
    for (int i = 0; i < NOF_THREADS; i++) {
        pthread_create(&threads[i], NULL, Worker, (void *) i);
    }
}

void perform_steps(int n, bool perform_save) {
    init(n);
    for (int step = 0; step < NSTEPS; step++) {
        pthread_mutex_lock(&index_lock);
        for (int i = 0; i < nof_slices; i++) {
            particle_matrix[i].clear();
        }

        for (int i = 0; i < nof_particles; i++) {
            particle_matrix[(int) (particles[i].x * floor(nof_slices / size))].emplace_back(&particles[i]);
        }

        indexation_done = true;
        pthread_cond_broadcast(&index_cond);
        pthread_mutex_unlock(&index_lock);
        pthread_mutex_lock(&work_lock);
        while(!work_done) {
            pthread_cond_wait(&work_cond, &work_lock);
        }
        pthread_mutex_unlock(&work_lock);
        if (perform_save) {
            save(particles);
        }
    }
}