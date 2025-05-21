/* File:     nbody_basic.c (modified for OpenMP optimization)
 * Purpose:  Implement a 2-dimensional n-body solver that uses the 
 *           straightforward n^2 algorithm. This version uses OpenMP
 *           to parallelize computations within each timestep.
 *
 * Compile:  gcc -g -Wall -fopenmp -o nbody_omp nbody_omp_optimized_with_comments.c -lm // 修改点：添加-fopenmp，修改文件名
 *           (Note: -fopenmp flag is added for OpenMP)
 *           If COMPUTE_ENERGY is defined, the program will print 
 *              total potential energy, total kinetic energy and total
 *              energy of the system at each time step.
 *           To turn off output except for timing results, define NO_OUTPUT
 *           To get verbose output, define DEBUG
 *           Needs timer.h
 * Run:      ./nbody_omp <number of threads> <number of particles> <number of timesteps>  // 修改点：运行命令中增加了线程数参数
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
 *    for each timestep t { // 此循环保持串行执行
 *       for each particle i (in parallel) // 内部的粒子计算并行化
 *          compute f(i), the force on i
 *       for each particle i (in parallel) // 内部的粒子计算并行化
 *          update position and velocity of i using F = ma
 *       if (output step) Output new positions and velocities (sequentially) // 此部分串行执行
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
#define NO_OUTPUT // 根据你之前的文件，这里定义了NO_OUTPUT

const double G = 6.673e-11;  /* Gravitational constant. */
                             /* Units are m^3/(kg*s^2)  */

typedef double vect_t[DIM];  /* Vector type for position, etc. */

struct particle_s {
   double m;  /* Mass     */
   vect_t s;  /* Position */
   vect_t v;  /* Velocity */
};

void Usage(char* prog_name); 
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
   double t;                   /* Current Time               */ // 警告：此变量在 NO_OUTPUT 定义时未被使用
   struct particle_s* curr;    /* Current state of system    */
   vect_t* forces;             /* Forces on each particle    */
   char g_i;                   /*_G_en or _i_nput init conds */
#  ifdef COMPUTE_ENERGY
   double kinetic_energy, potential_energy;
#  endif
   double start, finish;       /* For timings                */

   int num_threads; // 定义线程数
 
   // 修改点：从命令行获取线程数，并进行参数数量检查
   if (argc < 7) { // 期望参数: 程序名, 线程数, 粒子数, 时间步数, 时间步长, 输出频率, g|i
      fprintf(stderr, "Usage: %s <number of threads> <number of particles> <number of timesteps>\n", argv[0]);
      fprintf(stderr, "   <size of timestep> <output frequency> <g|i>\n");
      exit(0);
   }
   num_threads = atoi(argv[1]); // 第一个参数是线程数
   omp_set_num_threads(num_threads); // 设置OpenMP线程数
   
   // 修改点：调整传递给 Get_args 的参数，跳过已经被处理的线程数参数
   // 将(argc-1)和(argv+1)传递给Get_args，因为它现在处理的是程序名之后的参数
   Get_args(argc-1, argv+1, &n, &n_steps, &delta_t, &output_freq, &g_i);

   curr = malloc(n*sizeof(struct particle_s));
   forces = malloc(n*sizeof(vect_t));
   // 建议的修改点：添加内存分配失败的检查
   if (curr == NULL || forces == NULL) {
        fprintf(stderr, "Failed to allocate memory for particles or forces.\n");
        exit(1);
   }

   if (g_i == 'i')
      Get_init_cond(curr, n);
   else
      Gen_init_cond(curr, n);

   GET_TIME(start); // 计时开始点，实验要求不可修改
#  ifdef COMPUTE_ENERGY
   Compute_energy(curr, n, &kinetic_energy, &potential_energy);
   printf("   PE = %e, KE = %e, Total Energy = %e\n",
         potential_energy, kinetic_energy, kinetic_energy+potential_energy);
#  endif
#  ifndef NO_OUTPUT
   Output_state(0, curr, n);
#  endif

   // 修改点：移除了原先在 step 循环外部的 #pragma omp parallel for
   // 主时间步进循环 - 此循环必须保持串行执行
   for (step = 1; step <= n_steps; step++) {
      t = step*delta_t; // 变量 t 的赋值

      // 修改点：内部的粒子计算循环并行化
      // 并行计算每个粒子受到的力
      // private(part) 确保每个线程有自己的 part 变量副本
      // forces 数组被多个线程写入，但每个线程写入 forces[part]，是不同的内存位置，因此是安全的。
      // curr 数组主要被读取。
#     pragma omp parallel for private(part) 
      for (part = 0; part < n; part++) {
         // Compute_force 内部会初始化 forces[part]
         Compute_force(part, forces, curr, n);
      }

      // 此处有一个隐式的屏障 (barrier): 所有线程在进入下一个并行区域前会同步，
      // 确保所有力的计算都已完成。

      // 修改点：内部的粒子更新循环并行化
      // 并行更新每个粒子的位置和速度
      // private(part) 确保每个线程有自己的 part 变量副本
      // curr[part] 被写入，forces[part] 被读取。
#     pragma omp parallel for private(part)
      for (part = 0; part < n; part++) {
         Update_part(part, forces, curr, n, delta_t);
      }
      
      // 此处有一个隐式的屏障 (barrier): 所有线程同步。

#     ifdef COMPUTE_ENERGY
      if (step % output_freq == 0) {
        // 修改点：移除了此处的 #pragma omp critical
        // 因为外层的 step 循环是串行的，这部分代码由单个线程执行，不再需要临界区保护。
        Compute_energy(curr, n, &kinetic_energy, &potential_energy);
        printf(" istep = %d, PE = %e, KE = %e, Total Energy = %e\n",
              step, potential_energy, kinetic_energy, kinetic_energy+potential_energy);
	  }
#     endif
#     ifndef NO_OUTPUT // 如果 NO_OUTPUT 未定义 (在你的文件中是定义的)
      if (step % output_freq == 0) {
        // 修改点：移除了此处的 #pragma omp critical
        // 原因同上。
         Output_state(t, curr, n); // 变量 t 在此处被使用
      }
#     endif
   } // 结束串行的 step 循环
   
   GET_TIME(finish); // 计时结束点，实验要求不可修改
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
 * 注意: 此 Usage 函数是为 Get_args 设计的，它期望5个参数。
 *       main 函数中有一个新的 Usage 打印，用于处理程序完整的6个参数。
 */
void Usage(char* prog_name) {
   fprintf(stderr, "usage: %s <number of particles> <number of timesteps>\n",
         prog_name); 
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
 *    argc:            number of command line args (应为6: "程序名" + 5个数据参数)
 *                     注意：从main调用时，这里的argc是原始argc-1，argv[0]是原始的线程数参数
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
   // 当从 main 调用时:
   // argc 是 原始argc - 1
   // argv 是 原始argv + 1
   // 所以 Get_args 中的 argv[0] 是原始的 argv[1] (即线程数)
   // Get_args 中的 argv[1] 是原始的 argv[2] (即粒子数), 依此类推。
   // 它期望在它自己的 "程序名" (实际上是线程数字符串) 之后有5个参数。
   // 所以它期望 argc 是 1 (对于 "程序名") + 5 = 6。
   if (argc != 6) Usage(argv[0]); // 这里的 argv[0] 是线程数字符串。
                                   // 这可能会让 Usage 消息产生困惑。
                                   // main 函数中已经有了针对完整参数列表的 Usage 打印。
   *n_p = strtol(argv[1], NULL, 10);             // 对应原始 argv[2]
   *n_steps_p = strtol(argv[2], NULL, 10);       // 对应原始 argv[3]
   *delta_t_p = strtod(argv[3], NULL);           // 对应原始 argv[4]
   *output_freq_p = strtol(argv[4], NULL, 10);   // 对应原始 argv[5]
   *g_i_p = argv[5][0];                          // 对应原始 argv[6]

   if (*n_p <= 0 || *n_steps_p < 0 || *delta_t_p <= 0) Usage(argv[0]); 
   if (*g_i_p != 'g' && *g_i_p != 'i') Usage(argv[0]); 

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
      // 建议的修改点：添加 scanf 的返回值检查以确保输入成功
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
 */
void Gen_init_cond(struct particle_s curr[], int n) {
   int part;
   double mass = 5.0e24;
   double gap = 1.0e5;
   double speed = 3.0e4;

   srandom(1); // 固定种子以产生可复现的结果，便于调试
   for (part = 0; part < n; part++) {
      curr[part].m = mass;
      curr[part].s[X] = part*gap;
      curr[part].s[Y] = 0.0;
      curr[part].v[X] = 0.0;
      // 修改点：将随机条件改为确定性条件，便于比较串行和并行版本的结果
      if (part % 2 == 0) 
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
   // 假设 NO_OUTPUT 被定义，此函数体可能不会被编译，
   // 或者其调用可能被优化掉。
   // 如果它被调用，printf 本身在没有临界区保护的情况下，
   // 从多个线程同时进行复杂格式化输出不是线程安全的。
   // 但现在它从 step 循环的串行部分调用。
   printf("%.2f\n", time);
   for (part = 0; part < n; part++) {
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
   vect_t f_part_k; // 作用在粒子 `part` 上由粒子 `k` 引起的力
   double len, len_3, fact;

   // 初始化当前粒子 `part` 的受力为零
   forces[part][X] = 0.0;
   forces[part][Y] = 0.0;

   for (k = 0; k < n; k++) {
      if (k != part) {
         // 计算从粒子 `part` 指向粒子 `k` 的位移矢量 (s_k - s_part)
         // 这是吸引力的方向
         f_part_k[X] = curr[k].s[X] - curr[part].s[X]; // 修正/明确：s_k - s_i 
         f_part_k[Y] = curr[k].s[Y] - curr[part].s[Y]; // 修正/明确：s_k - s_i
         
         len = sqrt(f_part_k[X]*f_part_k[X] + f_part_k[Y]*f_part_k[Y]);
         if (len == 0.0) { // 避免粒子重叠时除以零
#           ifdef DEBUG
            printf("Warning: Particles %d and %d are at the same position.\n", part, k);
#           endif
            continue; 
         }
         len_3 = len*len*len;
         
         // 作用在粒子 `part` 上由 `k` 引起的力是 G * m_part * m_k * (s_k - s_part) / |s_k - s_part|^3
         // 你之前的实现是 (s_part - s_k) 和 -G*m_part*m_k，结果是等效的。
         // 这里统一为标准吸引力公式的形式。
         mg = G*curr[part].m*curr[k].m; // G * m_i * m_k
         fact = mg/len_3;
         
         // 将力分量加到 `part` 的总受力上
         forces[part][X] += fact * f_part_k[X];
         forces[part][Y] += fact * f_part_k[Y];
      }
   }
}  /* Compute_force */


/*---------------------------------------------------------------------
 * Function:  Update_part
 * Purpose:   Update the velocity and position for particle part
 * In args:
 *    part:    the particle we're updating
 *    forces:  forces[i] stores the total force on the ith particle
 *    curr:    In/Out: curr[i] stores the mass, position and velocity
 *    n:       number of particles (此处未使用，但保留函数签名一致性)
 *    delta_t: timestep
 */
void Update_part(int part, vect_t forces[], struct particle_s curr[], 
      int n, double delta_t) {
   double fact_a = delta_t/curr[part].m; // delta_t * (1/m) 用于加速度更新

   // 欧拉法:
   // s_new = s_old + delta_t * v_old
   // v_new = v_old + delta_t * a_old  (其中 a_old = F_old / m)

   curr[part].s[X] += delta_t * curr[part].v[X];
   curr[part].s[Y] += delta_t * curr[part].v[Y];
   
   curr[part].v[X] += fact_a * forces[part][X];
   curr[part].v[Y] += fact_a * forces[part][Y];
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

   // 动能: 0.5 * m * v^2
   for (i = 0; i < n; i++) {
      speed_sqr = curr[i].v[X]*curr[i].v[X] + curr[i].v[Y]*curr[i].v[Y];
      ke += curr[i].m*speed_sqr;
   }
   ke *= 0.5;

   // 势能: -G * m_i * m_j / |s_i - s_j|
   // 对唯一的粒子对 (i < j) 进行求和
   for (i = 0; i < n-1; i++) {
      for (j = i+1; j < n; j++) {
         diff[X] = curr[i].s[X] - curr[j].s[X];
         diff[Y] = curr[i].s[Y] - curr[j].s[Y];
         dist = sqrt(diff[X]*diff[X] + diff[Y]*diff[Y]);
         if (dist > 0) { // 避免粒子重叠时除以零
            pe += -G*curr[i].m*curr[j].m/dist;
         }
      }
   }

   *kin_en_p = ke;
   *pot_en_p = pe;
}  /* Compute_energy */
