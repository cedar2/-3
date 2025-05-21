/* File:     nbody_basic.c (modified for OpenMP optimization)
 * Purpose:  Implement a 2-dimensional n-body solver that uses the 
 *           straightforward n^2 algorithm. This version uses OpenMP
 *           to parallelize computations within each timestep.
 *
 * Compile:  gcc -g -Wall -fopenmp -o nbody_omp nbody_omp_optimized.c -lm
 *           (Note: -fopenmp flag is added for OpenMP)
 *           If COMPUTE_ENERGY is defined, the program will print 
 *              total potential energy, total kinetic energy and total
 *              energy of the system at each time step.
 *           To turn off output except for timing results, define NO_OUTPUT
 *           To get verbose output, define DEBUG
 *           Needs timer.h
 * Run:      ./nbody_omp <number of threads> <number of particles> <number of timesteps>  
 *              <size of timestep> <output frequency> <g|i>
 *              'g': generate initial conditions using a random number
 *                   generator
 *              'i': read initial conditions from stdin
 *           A timestep of 0.01 seems to work reasonably well for
 *           the automatically generated data.
 *
 * Input:    If 'g' is specified on the command line, none.  
 *           If 'i', mass, initial position and initial velocity of 
 *              each particle
 * Output:   If the output frequency is k, then position and velocity of 
 *              each particle at every kth timestep
 *
 * Algorithm: Slightly modified version of algorithm in James Demmel, 
 *    "CS 267, Applications of Parallel Computers:  Hierarchical 
 *    Methods for the N-Body Problem",
 *    www.cs.berkeley.edu/~demmel/cs267_Spr09, April 20, 2009.
 *
 *    for each timestep t { // This loop remains sequential
 *       for each particle i (in parallel)
 *          compute f(i), the force on i
 *       for each particle i (in parallel)
 *          update position and velocity of i using F = ma
 *       if (output step) Output new positions and velocities (sequentially)
 *    }
 *
 * Force:    The force on particle i due to particle k is given by
 *
 *    -G m_i m_k (s_i - s_k)/|s_i - s_k|^3
 *
 * Here, m_j is the mass of particle j, s_j is its position vector
 * (at time t), and G is the gravitational constant (see below).  
 *
 * Integration:  We use Euler's method:
 *
 *    v_i(t+1) = v_i(t) + h v'_i(t)
 *    s_i(t+1) = s_i(t) + h v_i(t)
 *
 * Here, v_i(u) is the velocity of the ith particle at time u and
 * s_i(u) is its position.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "timer.h"
#include <omp.h>   //使用openmp

#define DIM 2  /* Two-dimensional system */
#define X 0    /* x-coordinate subscript */
#define Y 1    /* y-coordinate subscript */
#define COMPUTE_ENERGY
#define NO_OUTPUT // As per your original file

const double G = 6.673e-11;  /* Gravitational constant. */
                             /* Units are m^3/(kg*s^2)  */

typedef double vect_t[DIM];  /* Vector type for position, etc. */

struct particle_s {
   double m;  /* Mass     */
   vect_t s;  /* Position */
   vect_t v;  /* Velocity */
};

void Usage(char* prog_name); // Original Usage function for 5 args
void Get_args(int argc, char* argv[], int* n_p, int* n_steps_p, 
      double* delta_t_p, int* output_freq_p, char* g_i_p);
void Get_init_cond(struct particle_s curr[], int n);
void Gen_init_cond(struct particle_s curr[], int n);
void Output_state(double time, struct particle_s curr[], int n);
void Compute_force(int part, vect_t forces[], struct particle_s curr[], 
      int n);
void Update_part(int part, vect_t forces[], struct particle_s curr[], 
      int n, double delta_t);
void Compute_energy(struct particle_s curr[], int n, double* kin_en_p,
      double* pot_en_p);

/*--------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
   int n;                      /* Number of particles        */
   int n_steps;                /* Number of timesteps        */
   int step;                   /* Current step               */
   int part;                   /* Current particle           */
   int output_freq;            /* Frequency of output        */
   double delta_t;             /* Size of timestep           */
   double t;                   /* Current Time               */
   struct particle_s* curr;    /* Current state of system    */
   vect_t* forces;             /* Forces on each particle    */
   char g_i;                   /*_G_en or _i_nput init conds */
#  ifdef COMPUTE_ENERGY
   double kinetic_energy, potential_energy;
#  endif
   double start, finish;       /* For timings                */

   int num_threads; //定义线程数
 
   if (argc < 7) { // Expects: prog_name, num_threads, n, n_steps, delta_t, output_freq, g_i
      fprintf(stderr, "Usage: %s <number of threads> <number of particles> <number of timesteps>\n", argv[0]);
      fprintf(stderr, "   <size of timestep> <output frequency> <g|i>\n");
      exit(0);
   }
   num_threads = atoi(argv[1]);
   omp_set_num_threads(num_threads);
   
   // Pass the remaining arguments to Get_args
   Get_args(argc-1, argv+1, &n, &n_steps, &delta_t, &output_freq, &g_i);

   curr = malloc(n*sizeof(struct particle_s));
   forces = malloc(n*sizeof(vect_t));
   if (curr == NULL || forces == NULL) {
        fprintf(stderr, "Failed to allocate memory for particles or forces.\n");
        exit(1);
   }

   if (g_i == 'i')
      Get_init_cond(curr, n);
   else
      Gen_init_cond(curr, n);

   GET_TIME(start); // Timing starts here, DO NOT MODIFY
#  ifdef COMPUTE_ENERGY
   Compute_energy(curr, n, &kinetic_energy, &potential_energy);
   // This initial print is fine as it's before the main loop
   printf("   PE = %e, KE = %e, Total Energy = %e\n",
         potential_energy, kinetic_energy, kinetic_energy+potential_energy);
#  endif
#  ifndef NO_OUTPUT
   Output_state(0, curr, n);
#  endif

   // Main time stepping loop - THIS LOOP IS SEQUENTIAL
   for (step = 1; step <= n_steps; step++) {
      t = step*delta_t;
//    memset(forces, 0, n*sizeof(vect_t)); // Not strictly needed as Compute_force initializes forces[part]

      // Parallelize the force computation loop
#     pragma omp parallel for private(part)
      for (part = 0; part < n; part++)
         Compute_force(part, forces, curr, n);

      // Implicit barrier here: all threads synchronize before proceeding.

      // Parallelize the particle update loop
#     pragma omp parallel for private(part)
      for (part = 0; part < n; part++)
         Update_part(part, forces, curr, n, delta_t);
      
      // Implicit barrier here.

#     ifdef COMPUTE_ENERGY
      if (step % output_freq == 0) {
        // This block is now executed by a single thread (the one iterating the 'step' loop)
        // No #pragma omp critical needed here anymore.
        Compute_energy(curr, n, &kinetic_energy, &potential_energy);
        printf(" istep = %d, PE = %e, KE = %e, Total Energy = %e\n",
              step, potential_energy, kinetic_energy, kinetic_energy+potential_energy);
	  }
#     endif
#     ifndef NO_OUTPUT // NO_OUTPUT is defined in your original code
      if (step % output_freq == 0) {
        // No #pragma omp critical needed here anymore.
         Output_state(t, curr, n);
      }
#     endif
   } // End of sequential step loop
   
   GET_TIME(finish); // Timing ends here, DO NOT MODIFY
   printf("Elapsed time = %e seconds\n", finish-start);

   free(curr);
   free(forces);
   return 0;
}  /* main */


/*---------------------------------------------------------------------
 * Function: Usage
 * Purpose:  Print instructions for command-line and exit
 * In arg:   
 *    prog_name:  the name of the program as typed on the command-line
 * Note:      This Usage function is for the original 5 arguments to Get_args.
 *            The main function now has a new Usage message for 6 program arguments.
 */
void Usage(char* prog_name) {
   // prog_name here will be argv[1] from main's perspective if Get_args calls it
   // or the original program name if Get_args uses its own argv[0].
   // Let's assume it's for the arguments Get_args processes.
   fprintf(stderr, "usage: %s <number of particles> <number of timesteps>\n",
         prog_name); // prog_name would be the "number of particles" if called as Usage(argv[0]) from Get_args
   fprintf(stderr, "   <size of timestep> <output frequency>\n");
   fprintf(stderr, "   <g|i>\n");
   fprintf(stderr, "   'g': program should generate init conds\n");
   fprintf(stderr, "   'i': program should get init conds from stdin\n");
    
   exit(0);
}  /* Usage */


/*---------------------------------------------------------------------
 * Function:  Get_args
 * Purpose:   Get command line args (particles, steps, delta_t, freq, g_i)
 * In args:
 *    argc:            number of command line args (should be 6: prog_name + 5 data args)
 *    argv:            command line args
 * Out args:
 *    n_p:             pointer to n, the number of particles
 *    n_steps_p:       pointer to n_steps, the number of timesteps
 *    delta_t_p:       pointer to delta_t, the size of each timestep
 *    output_freq_p:   pointer to output_freq, which is the number of
 *                     timesteps between steps whose output is printed
 *    g_i_p:           pointer to char which is 'g' if the init conds
 *                     should be generated by the program and 'i' if
 *                     they should be read from stdin
 */
void Get_args(int argc, char* argv[], int* n_p, int* n_steps_p, 
      double* delta_t_p, int* output_freq_p, char* g_i_p) {
   // After main processes num_threads, argc passed here is (original_argc - 1)
   // argv passed here is (original_argv + 1)
   // So, argv[0] for Get_args is the original argv[1] (num_threads),
   // argv[1] for Get_args is the original argv[2] (number of particles), etc.
   // It expects 5 arguments *after* what it considers its own "program name" (which is actually num_threads).
   // So, it expects argc to be 1 (for "program name") + 5 = 6.
   if (argc != 6) Usage(argv[0]); // argv[0] here is effectively the num_threads string.
                                   // This might be confusing for the Usage message.
                                   // A dedicated Usage for the main program's full arg list is better.
                                   // The main function already has its own Usage print.
   *n_p = strtol(argv[1], NULL, 10);             // original argv[2]
   *n_steps_p = strtol(argv[2], NULL, 10);       // original argv[3]
   *delta_t_p = strtod(argv[3], NULL);           // original argv[4]
   *output_freq_p = strtol(argv[4], NULL, 10);   // original argv[5]
   *g_i_p = argv[5][0];                          // original argv[6]

   if (*n_p <= 0 || *n_steps_p < 0 || *delta_t_p <= 0) Usage(argv[0]); // Pass "num_threads" string to Usage
   if (*g_i_p != 'g' && *g_i_p != 'i') Usage(argv[0]); // Pass "num_threads" string to Usage

#  ifdef DEBUG
   printf("n = %d\n", *n_p);
   printf("n_steps = %d\n", *n_steps_p);
   printf("delta_t = %e\n", *delta_t_p);
   printf("output_freq = %d\n", *output_freq_p);
   printf("g_i = %c\n", *g_i_p);
#  endif
}  /* Get_args */


/*---------------------------------------------------------------------
 * Function:  Get_init_cond
 * Purpose:   Read in initial conditions:  mass, position and velocity
 *            for each particle
 * In args:  
 *    n:      number of particles
 * Out args:
 *    curr:   array of n structs, each struct stores the mass (scalar),
 *            position (vector), and velocity (vector) of a particle
 */
void Get_init_cond(struct particle_s curr[], int n) {
   int part;

   printf("For each particle, enter (in order):\n");
   printf("   its mass, its x-coord, its y-coord, ");
   printf("its x-velocity, its y-velocity\n");
   for (part = 0; part < n; part++) {
      if (scanf("%lf", &curr[part].m) != 1) {fprintf(stderr, "Error reading mass for particle %d\n", part); exit(1);}
      if (scanf("%lf", &curr[part].s[X])!= 1) {fprintf(stderr, "Error reading s[X] for particle %d\n", part); exit(1);}
      if (scanf("%lf", &curr[part].s[Y])!= 1) {fprintf(stderr, "Error reading s[Y] for particle %d\n", part); exit(1);}
      if (scanf("%lf", &curr[part].v[X])!= 1) {fprintf(stderr, "Error reading v[X] for particle %d\n", part); exit(1);}
      if (scanf("%lf", &curr[part].v[Y])!= 1) {fprintf(stderr, "Error reading v[Y] for particle %d\n", part); exit(1);}
   }
}  /* Get_init_cond */

/*---------------------------------------------------------------------
 * Function:  Gen_init_cond
 * Purpose:   Generate initial conditions:  mass, position and velocity
 *            for each particle
 * In args:  
 *    n:      number of particles
 * Out args:
 *    curr:   array of n structs, each struct stores the mass (scalar),
 *            position (vector), and velocity (vector) of a particle
 *
 * Note:      The initial conditions place all particles at
 *            equal intervals on the nonnegative x-axis with 
 *            identical masses, and identical initial speeds
 *            parallel to the y-axis.  However, some of the
 *            velocities are in the positive y-direction and
 *            some are negative.
 */
void Gen_init_cond(struct particle_s curr[], int n) {
   int part;
   double mass = 5.0e24;
   double gap = 1.0e5;
   double speed = 3.0e4;

   srandom(1); // Fixed seed for reproducible results
   for (part = 0; part < n; part++) {
      curr[part].m = mass;
      curr[part].s[X] = part*gap;
      curr[part].s[Y] = 0.0;
      curr[part].v[X] = 0.0;
      if (part % 2 == 0) // Changed from random to make it deterministic based on part index
         curr[part].v[Y] = speed;
      else
         curr[part].v[Y] = -speed;
   }
}  /* Gen_init_cond */


/*---------------------------------------------------------------------
 * Function:  Output_state
 * Purpose:   Print the current state of the system
 * In args:
 *    curr:   array with n elements, curr[i] stores the state (mass,
 *            position and velocity) of the ith particle
 *    n:      number of particles
 */
void Output_state(double time, struct particle_s curr[], int n) {
   int part;
   // Assuming NO_OUTPUT is defined, this function's body might not be compiled
   // or its calls might be optimized out.
   // If it were called, printf itself is not thread-safe for complex formatting
   // from multiple threads simultaneously without a critical section.
   // But now it's called from a sequential part of the step loop.
   printf("%.2f\n", time);
   for (part = 0; part < n; part++) {
//    printf("%.3f ", curr[part].m);
      printf("%3d %10.3e ", part, curr[part].s[X]);
      printf("  %10.3e ", curr[part].s[Y]);
      printf("  %10.3e ", curr[part].v[X]);
      printf("  %10.3e\n", curr[part].v[Y]);
   }
   printf("\n");
}  /* Output_state */


/*---------------------------------------------------------------------
 * Function:  Compute_force
 * Purpose:   Compute the total force on particle part.  
 * In args:   
 *    part:   the particle on which we're computing the total force
 *    curr:   current state of the system:  curr[i] stores the mass,
 *            position and velocity of the ith particle
 *    n:      number of particles
 * Out arg:
 *    forces: force[i] stores the total force on the ith particle
 */
void Compute_force(int part, vect_t forces[], struct particle_s curr[], 
      int n) {
   int k;
   double mg; 
   vect_t f_part_k; // force on particle `part` due to particle `k`
   double len, len_3, fact;

   // Initialize forces for the current particle `part` to zero
   forces[part][X] = 0.0;
   forces[part][Y] = 0.0;

   for (k = 0; k < n; k++) {
      if (k != part) {
         f_part_k[X] = curr[k].s[X] - curr[part].s[X]; // Corrected: s_k - s_i
         f_part_k[Y] = curr[k].s[Y] - curr[part].s[Y]; // Corrected: s_k - s_i
         
         len = sqrt(f_part_k[X]*f_part_k[X] + f_part_k[Y]*f_part_k[Y]);
         if (len == 0.0) { // Avoid division by zero if particles overlap
#           ifdef DEBUG
            printf("Warning: Particles %d and %d are at the same position.\n", part, k);
#           endif
            continue; 
         }
         len_3 = len*len*len;
         
         // Force on particle `part` due to `k` is G * m_part * m_k * (s_k - s_part) / |s_k - s_part|^3
         mg = G*curr[part].m*curr[k].m; // G * m_i * m_k
         fact = mg/len_3;
         
         // Add force component to total force on `part`
         forces[part][X] += fact * f_part_k[X];
         forces[part][Y] += fact * f_part_k[Y];
      }
   }
#  ifdef DEBUG
   // This debug print should be outside the k loop if it's for the total force on 'part'
   // Or if it's for f_part_k, it should be inside. The original was confusingly placed.
   // Assuming it's for the total force after accumulation:
   // printf("DEBUG: Total force on particle %d after Compute_force = (%.3e, %.3e)\n",
   //       part, forces[part][X], forces[part][Y]);
#  endif
}  /* Compute_force */


/*---------------------------------------------------------------------
 * Function:  Update_part
 * Purpose:   Update the velocity and position for particle part
 * In args:
 *    part:    the particle we're updating
 *    forces:  forces[i] stores the total force on the ith particle
 *    curr:    In/Out: curr[i] stores the mass, position and velocity
 *    n:       number of particles (not used here but kept for signature)
 *    delta_t: timestep
 */
void Update_part(int part, vect_t forces[], struct particle_s curr[], 
      int n, double delta_t) {
   double fact_a = delta_t/curr[part].m; // delta_t * (1/m) for acceleration update

   // Euler's method:
   // s_new = s_old + delta_t * v_old
   // v_new = v_old + delta_t * a_old  (where a_old = F_old / m)

   curr[part].s[X] += delta_t * curr[part].v[X];
   curr[part].s[Y] += delta_t * curr[part].v[Y];
   
   curr[part].v[X] += fact_a * forces[part][X];
   curr[part].v[Y] += fact_a * forces[part][Y];

#  ifdef DEBUG
   printf("DEBUG: After update of %d:\n", part);
   printf("   Position  = (%.3e, %.3e)\n", curr[part].s[X], curr[part].s[Y]);
   printf("   Velocity  = (%.3e, %.3e)\n", curr[part].v[X], curr[part].v[Y]);
#  endif
}  /* Update_part */


/*---------------------------------------------------------------------
 * Function:  Compute_energy
 * Purpose:   Compute the kinetic and potential energy in the system
 * In args:
 *    curr:   current state of the system
 *    n:      number of particles
 * Out args:
 *    kin_en_p: pointer to kinetic energy of system
 *    pot_en_p: pointer to potential energy of system
 */
void Compute_energy(struct particle_s curr[], int n, double* kin_en_p,
      double* pot_en_p) {
   int i, j;
   vect_t diff;
   double pe = 0.0, ke = 0.0;
   double dist, speed_sqr;

   // Kinetic Energy: 0.5 * m * v^2
   for (i = 0; i < n; i++) {
      speed_sqr = curr[i].v[X]*curr[i].v[X] + curr[i].v[Y]*curr[i].v[Y];
      ke += curr[i].m*speed_sqr;
   }
   ke *= 0.5;

   // Potential Energy: -G * m_i * m_j / |s_i - s_j|
   // Sum over unique pairs (i < j)
   for (i = 0; i < n-1; i++) {
      for (j = i+1; j < n; j++) {
         diff[X] = curr[i].s[X] - curr[j].s[X];
         diff[Y] = curr[i].s[Y] - curr[j].s[Y];
         dist = sqrt(diff[X]*diff[X] + diff[Y]*diff[Y]);
         if (dist > 0) { // Avoid division by zero if particles somehow overlap
            pe += -G*curr[i].m*curr[j].m/dist;
         }
      }
   }

   *kin_en_p = ke;
   *pot_en_p = pe;
}  /* Compute_energy */
