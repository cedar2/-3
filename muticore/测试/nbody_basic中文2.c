/* File:     nbody_basic.c (Re-optimized with OpenMP)
 * Purpose:  Implement a 2-dimensional n-body solver that uses the 
 *           straightforward n^2 algorithm.  This version directly
 *           computes all the forces.
 *
 * Compile:  gcc -g -Wall -fopenmp -o nbody_omp_reoptimized nbody_omp_reoptimized.c -lm
 * Run:      ./nbody_omp_reoptimized <number of threads> <number of particles> <number of timesteps>  
 *              <size of timestep> <output frequency> <g|i>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "timer.h"
// 优化：包含 OpenMP 头文件以使用 OpenMP 功能
#include <omp.h>

#define DIM 2  /* Two-dimensional system */
#define X 0    /* x-coordinate subscript */
#define Y 1    /* y-coordinate subscript */
#define COMPUTE_ENERGY
#define NO_OUTPUT

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
   double t;                   /* Current Time               */
   struct particle_s* curr;    /* Current state of system    */
   vect_t* forces;             /* Forces on each particle    */
   char g_i;                   /*_G_en or _i_nput init conds */
#  ifdef COMPUTE_ENERGY
   double kinetic_energy, potential_energy;
#  endif
   double start, finish;       /* For timings                */
   // 优化：定义线程数变量
   int num_threads;

   // 优化：检查命令行参数数量，期望比原来多一个线程数参数
   if (argc != 7) { // 程序名 + 线程数 + 5 个原有参数
      fprintf(stderr, "Usage: %s <number of threads> <number of particles> <number of timesteps>\n", argv[0]);
      fprintf(stderr, "   <size of timestep> <output frequency> <g|i>\n");
      exit(0);
   }
   // 优化：从命令行参数获取线程数
   num_threads = strtol(argv[1], NULL, 10);
   if (num_threads <= 0) {
       fprintf(stderr, "Error: Number of threads must be positive.\n");
       exit(0);
   }
   // 优化：设置 OpenMP 使用的线程数
   omp_set_num_threads(num_threads);

   // 优化：调用 Get_args 时，传递调整后的 argc 和 argv，跳过已处理的线程数参数
   // argc 变为原来的 argc-1，argv 指向原来的 argv[1]
   Get_args(argc-1, argv+1, &n, &n_steps, &delta_t, &output_freq, &g_i);
   
   curr = malloc(n*sizeof(struct particle_s));
   forces = malloc(n*sizeof(vect_t));
   // 优化：增加内存分配失败的检查
   if (curr == NULL || forces == NULL) {
      fprintf(stderr, "Error: Failed to allocate memory for particles or forces.\n");
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
   // 主时间步进循环，此循环保持串行，因为时间步之间有依赖关系
   for (step = 1; step <= n_steps; step++) {
      t = step*delta_t;
//    memset(forces, 0, n*sizeof(vect_t)); // 此行可以省略，因为 Compute_force 会初始化 forces[part]

      // 优化：使用 OpenMP并行化计算所有粒子受力的循环
      // private(part) 子句确保每个线程拥有自己的 part 循环变量副本
      // forces 数组的访问是安全的，因为每个线程操作 forces[part]，part 对于不同线程是不同的
      // curr 数组在此循环中主要被读取
#     pragma omp parallel for private(part)
      for (part = 0; part < n; part++)
         Compute_force(part, forces, curr, n);

      // OpenMP 在并行的 for 循环结束后会有一个隐式的屏障 (barrier)，
      // 确保所有线程都完成了力的计算，才继续执行后续代码。

      // 优化：使用 OpenMP并行化更新所有粒子位置和速度的循环
      // private(part) 子句确保每个线程拥有自己的 part 循环变量副本
      // curr[part] 的不同元素由不同线程更新，forces[part] 被读取
#     pragma omp parallel for private(part)
      for (part = 0; part < n; part++)
         Update_part(part, forces, curr, n, delta_t);

      // OpenMP 在并行的 for 循环结束后会有一个隐式的屏障。

#     ifdef COMPUTE_ENERGY
      if (step % output_freq == 0)
	  {
        // 此处的能量计算和打印在串行的时间步循环内执行，由主线程完成，
        // 因此不需要 OpenMP 的临界区保护。
        Compute_energy(curr, n, &kinetic_energy, &potential_energy);
        printf(" istep = %d, PE = %e, KE = %e, Total Energy = %e\n",
              step, potential_energy, kinetic_energy, kinetic_energy+potential_energy);
	  }
#     endif
#     ifndef NO_OUTPUT // NO_OUTPUT 在此文件中被定义，所以此块代码通常不编译
      if (step % output_freq == 0)
         // 此处的输出也在串行的时间步循环内执行，不需要临界区保护。
         Output_state(t, curr, n);
#     endif
   }
   
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
 */
void Usage(char* prog_name) {
   // 注意：当 Get_args 调用此 Usage 时，prog_name 可能是线程数字符串。
   // main 函数中已经有了更准确的 Usage 打印。
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
 * Purpose:   Get command line args (不包括线程数，线程数已在 main 中处理)
 * In args:
 *    argc:            number of command line args (从 main 传入时，是原 argc-1)
 *    argv:            command line args (从 main 传入时，是原 argv+1)
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
   // 优化：argc 现在期望是 6 (因为 main 传过来的是 argc-1, argv+1,
   // argv[0] 对 Get_args 来说是原来的线程数参数，之后跟5个数据参数)
   if (argc != 6) Usage(argv[0]); // argv[0] 是线程数字符串，Usage 消息可能不完美
   
   // 参数索引相对于新的 argv
   *n_p = strtol(argv[1], NULL, 10);         // 对应原始命令行参数的第3个 (粒子数)
   *n_steps_p = strtol(argv[2], NULL, 10);   // 对应原始命令行参数的第4个 (时间步数)
   *delta_t_p = strtod(argv[3], NULL);       // 对应原始命令行参数的第5个 (时间步长)
   *output_freq_p = strtol(argv[4], NULL, 10); // 对应原始命令行参数的第6个 (输出频率)
   *g_i_p = argv[5][0];                      // 对应原始命令行参数的第7个 (g/i)

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
 * Purpose:   Read in initial conditions
 */
void Get_init_cond(struct particle_s curr[], int n) {
   int part;

   printf("For each particle, enter (in order):\n");
   printf("   its mass, its x-coord, its y-coord, ");
   printf("its x-velocity, its y-velocity\n");
   for (part = 0; part < n; part++) {
      // 优化：为 scanf 添加返回值检查，确保输入成功
      if (scanf("%lf", &curr[part].m) != 1) { fprintf(stderr, "Error reading mass for particle %d\n", part); exit(1); }
      if (scanf("%lf", &curr[part].s[X]) != 1) { fprintf(stderr, "Error reading s[X] for particle %d\n", part); exit(1); }
      if (scanf("%lf", &curr[part].s[Y]) != 1) { fprintf(stderr, "Error reading s[Y] for particle %d\n", part); exit(1); }
      if (scanf("%lf", &curr[part].v[X]) != 1) { fprintf(stderr, "Error reading v[X] for particle %d\n", part); exit(1); }
      if (scanf("%lf", &curr[part].v[Y]) != 1) { fprintf(stderr, "Error reading v[Y] for particle %d\n", part); exit(1); }
   }
}  /* Get_init_cond */

/*---------------------------------------------------------------------
 * Function:  Gen_init_cond
 * Purpose:   Generate initial conditions
 */
void Gen_init_cond(struct particle_s curr[], int n) {
   int part;
   double mass = 5.0e24;
   double gap = 1.0e5;
   double speed = 3.0e4;

   srandom(1); // 使用固定种子以保证初始条件的可复现性，便于调试和比较
   for (part = 0; part < n; part++) {
      curr[part].m = mass;
      curr[part].s[X] = part*gap;
      curr[part].s[Y] = 0.0;
      curr[part].v[X] = 0.0;
      // 优化：将随机生成改为确定性生成，便于比较不同运行的输出
      // if (random()/((double) RAND_MAX) >= 0.5) // 原随机方式
      if (part % 2 == 0) // 修改为确定性方式
         curr[part].v[Y] = speed;
      else
         curr[part].v[Y] = -speed;
   }
}  /* Gen_init_cond */


/*---------------------------------------------------------------------
 * Function:  Output_state
 * Purpose:   Print the current state of the system
 */
void Output_state(double time, struct particle_s curr[], int n) {
   int part;
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
 */
void Compute_force(int part, vect_t forces[], struct particle_s curr[], 
      int n) {
   int k;
   double mg; 
   vect_t f_part_k;
   double len, len_3, fact;

#  ifdef DEBUG
   // 此处的 DEBUG 打印的是 part 粒子在进入此函数时（可能来自上一步迭代）的受力，
   // 而非本次计算的力。
   // printf("Current total force on particle %d = (%.3e, %.3e)\n",
   //       part, forces[part][X], forces[part][Y]);
#  endif
   // 初始化当前粒子 part 的总受力为零
   forces[part][X] = forces[part][Y] = 0.0;
   for (k = 0; k < n; k++) {
      if (k != part) {
         // 计算从粒子 part 指向粒子 k 的位移矢量 (s_k - s_part)
         // 这是吸引力的方向
         f_part_k[X] = curr[k].s[X] - curr[part].s[X]; // 之前版本为 s_part - s_k, 配合 mg 带负号。
         f_part_k[Y] = curr[k].s[Y] - curr[part].s[Y]; // 此处统一为 (s_k - s_part)

         len = sqrt(f_part_k[X]*f_part_k[X] + f_part_k[Y]*f_part_k[Y]);
         
         // 优化：处理粒子完全重合的情况，避免除以零
         if (len == 0.0) {
#           ifdef DEBUG
            // 警告：如果定义了 DEBUG，这里会从多个线程并发打印，可能导致输出混乱。
            // 最好在 DEBUG 模式下，此部分由临界区保护，或者只在串行调试时启用。
            // printf("Warning: Particles %d and %d are at the same position.\n", part, k);
#           endif
            continue; // 跳过此粒子对的力计算
         }
         len_3 = len*len*len;

         // 原始代码: mg = -G*curr[part].m*curr[k].m; fact = mg/len_3; f_part_k *= fact;
         // f_part_k 的方向是 s_part - s_k。最终力是 -G*m1*m2*(s_part-s_k)/len^3 = G*m1*m2*(s_k-s_part)/len^3
         // 当前修改：f_part_k 的方向是 s_k - s_part。
         // 我们需要吸引力，所以力应该与 (s_k - s_part) 同向。
         // 公式 F_on_part_by_k = G * m_part * m_k * (s_k - s_part) / |s_k - s_part|^3
         
         mg = G*curr[part].m*curr[k].m; // G*m_part*m_k
         fact = mg/len_3;
         
         // f_part_k 已经是 (s_k - s_part) 方向的矢量，直接乘以 fact
         // forces[part] 累加的是作用在 part 上的力
         forces[part][X] += fact * f_part_k[X];
         forces[part][Y] += fact * f_part_k[Y];
   #     ifdef DEBUG
         // printf("Force on particle %d due to particle %d = (%.3e, %.3e)\n",
         //       part, k, f_part_k[X]*fact, f_part_k[Y]*fact); // 打印实际施加的力
   #     endif
      }
   }
}  /* Compute_force */


/*---------------------------------------------------------------------
 * Function:  Update_part
 * Purpose:   Update the velocity and position for particle part
 */
void Update_part(int part, vect_t forces[], struct particle_s curr[], 
      int n, double delta_t) {
   double fact_accel = delta_t/curr[part].m; // F/m * delta_t

#  ifdef DEBUG
   // printf("Before update of %d:\n", part);
   // printf("   Position  = (%.3e, %.3e)\n", curr[part].s[X], curr[part].s[Y]);
   // printf("   Velocity  = (%.3e, %.3e)\n", curr[part].v[X], curr[part].v[Y]);
   // printf("   Net force = (%.3e, %.3e)\n", forces[part][X], forces[part][Y]);
#  endif
   // 欧拉法更新位置：s_new = s_old + v_old * delta_t
   curr[part].s[X] += delta_t * curr[part].v[X];
   curr[part].s[Y] += delta_t * curr[part].v[Y];
   // 欧拉法更新速度：v_new = v_old + (F_old / m) * delta_t
   curr[part].v[X] += fact_accel * forces[part][X];
   curr[part].v[Y] += fact_accel * forces[part][Y];
#  ifdef DEBUG
   // printf("Position of %d = (%.3e, %.3e), Velocity = (%.3e,%.3e)\n",
   //       part, curr[part].s[X], curr[part].s[Y],
   //             curr[part].v[X], curr[part].v[Y]);
#  endif
}  /* Update_part */


/*---------------------------------------------------------------------
 * Function:  Compute_energy
 * Purpose:   Compute the kinetic and potential energy in the system
 */
void Compute_energy(struct particle_s curr[], int n, double* kin_en_p,
      double* pot_en_p) {
   int i, j;
   vect_t diff;
   double pe = 0.0, ke = 0.0;
   double dist, speed_sqr;

   // 计算动能: KE = sum(0.5 * m_i * v_i^2)
   for (i = 0; i < n; i++) {
      speed_sqr = curr[i].v[X]*curr[i].v[X] + curr[i].v[Y]*curr[i].v[Y];
      ke += curr[i].m*speed_sqr;
   }
   ke *= 0.5;

   // 计算势能: PE = sum_{i<j} (-G * m_i * m_j / |s_i - s_j|)
   for (i = 0; i < n-1; i++) {
      for (j = i+1; j < n; j++) {
         diff[X] = curr[i].s[X] - curr[j].s[X];
         diff[Y] = curr[i].s[Y] - curr[j].s[Y];
         dist = sqrt(diff[X]*diff[X] + diff[Y]*diff[Y]);
         // 优化：处理粒子完全重合的情况，避免除以零
         if (dist > 0) { // 只有当距离大于0时才计算势能贡献
            pe += -G*curr[i].m*curr[j].m/dist;
         }
      }
   }

   *kin_en_p = ke;
   *pot_en_p = pe;
}  /* Compute_energy */
