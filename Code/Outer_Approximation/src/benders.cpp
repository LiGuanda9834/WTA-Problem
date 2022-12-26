/* --------------------------------------------------------------------------
 * File: bendersatsp.c
 * Version 20.1.0
 * --------------------------------------------------------------------------
 * Licensed Materials - Property of IBM
 * 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5655-Y21
 * Copyright IBM Corporation 1997, 2020. All Rights Reserved.
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with
 * IBM Corp.
 * --------------------------------------------------------------------------
 */

/* Example bendersatsp.c solves a flow MILP model for an 
   Asymmetric Traveling Salesman Problem (ATSP) instance 
   through Benders decomposition.

   The arc costs of an ATSP instance are read from an input file.
   The flow MILP model is decomposed into a master ILP and a worker LP.
   
   The master ILP is then solved by adding Benders' cuts via the cut callback 
   function benders_callback during the branch-and-cut process.

   The cut callback benders_callback adds to the master ILP violated Benders' cuts 
   that are found by solving the worker LP.

   The example allows the user to decide if Benders' cuts have to be separated:

   a) Only to separate integer infeasible solutions. 
   In this case, benders_callback is set as a lazy constraint cut callback 
   function, via CPXsetlazyconstraintcallbackfunc. 
   
   b) Also to separate fractional infeasible solutions. 
   In this case, benders_callback is set as a lazy constraint cut callback 
   function, via CPXsetlazyconstraintcallbackfunc, as before.
   In addition, benders_callback is also set as a user cut callback 
   function, via CPXsetusercutcallbackfunc. */


/* To run this example, command line arguments are required:
       bendersatsp {0|1} [filename]
   where 
       0         Indicates that Benders' cuts are only used as lazy constraints, 
                 to separate integer infeasible solutions.
       1         Indicates that Benders' cuts are also used as user cuts, 
                 to separate fractional infeasible solutions.

       filename  Is the name of the file containing the ATSP instance (arc costs).
                 If filename is not specified, the instance 
                 ../../../examples/data/atsp.dat is read */


/* ATSP instance defined on a directed graph G = (V, A)
   - V = {0, ..., n-1}, V0 = V \ {0}
   - A = {(i,j) : i in V, j in V, i != j }
   - forall i in V: delta+(i) = {(i,j) in A : j in V}
   - forall i in V: delta-(i) = {(j,i) in A : j in V}
   - c(i,j) = traveling cost associated with (i,j) in A
   
   Flow MILP model 

   Modeling variables:
   forall (i,j) in A: 
      x(i,j) = 1, if arc (i,j) is selected
             = 0, otherwise
   forall k in V0, forall (i,j) in A: 
      y(k,i,j) = flow of the commodity k through arc (i,j)
   
   Objective:
   minimize sum((i,j) in A) c(i,j) * x(i,j)

   Degree constraints:
   forall i in V: sum((i,j) in delta+(i)) x(i,j) = 1
   forall i in V: sum((j,i) in delta-(i)) x(j,i) = 1

   Binary constraints on arc variables:
   forall (i,j) in A: x(i,j) in {0, 1}

   Flow constraints:
   forall k in V0, forall i in V:
      sum((i,j) in delta+(i)) y(k,i,j) - sum((j,i) in delta-(i)) y(k,j,i) = q(k,i)
      where q(k,i) =  1, if i = 0
                   = -1, if k == i
                   =  0, otherwise

   Capacity constraints:
   forall k in V0, for all (i,j) in A: y(k,i,j) <= x(i,j)

   Nonnegativity of flow variables:
   forall k in V0, for all (i,j) in A: y(k,i,j) >= 0 */

                  
/* Bring in the CPLEX function declarations and the C library 
   header file stdio.h with the following single include. */

#include <ilcplex/cplex.h>

/* Bring in the declarations for the string and character functions,
   malloc, and fabs. */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <assert.h>

#define WTA_EPS (1e-6)

class WTA
{
   public:
      int n;
      int m;
      double* w;
      double** p;
      double* x;
      int* cutind;
      double* cutval;
      double rhs;
      int separate_fractional_solutions;
      // 0 for exponential form, 1 for polynominal form
      int function_form;
      int seed;
      WTA(){};
      void Init(int n_, int m_, int seed = 0)
      {
         n = n_;
         m = m_;
         w = new double[n];
         p = new double*[m];
         separate_fractional_solutions = 1;
         srand(seed);
         for(int i = 0; i < n; i++)
         {
            w[i] = rand()%75 + 25;
         }
         for(int i = 0; i < m; i++)
         {  
            p[i] = new double[n];
            for(int j = 0; j < n; j++)
               p[i][j] = rand()/RAND_MAX * 0.3 + 0.6;
         }
         x = new double[m * n + n];
         rhs = 0;
         cutind = new int[m + 1];
         cutval = new double[m + 1];
         cutval[m] = -1;
      }
      void Init(FILE* fin, int seed_)
      {
         seed = seed_;
         int status = 0;
         int f = fscanf (fin, "%d", &n);
         m = n; 
         Init(n, m, 0);
         for(int i = 0; i < n; i++)
         {
            fscanf (fin, "%lf", &w[i]);
         }
         for(int i = 0; i < n; i++)
         {
            for(int j = 0; j < m; j++)
            {
               fscanf(fin, "%lf", &(p[i][j]));
               //printf("%.2f\n", p[i][j]);
            }
         }
      }
      void Delete()
      {
         for(int i = 0; i < m; i++)
            delete[] p[i];
         delete []p;
         delete []w;
         delete []x;
         delete []cutind;
         delete []cutval;
      }
      ~WTA(){}

      double cal_fjx_(int j)
      {
         double fjx = 1;
         for (int i = 0; i < m; i++)
         {
            if( x[i * n + j] > WTA_EPS)
               fjx = fjx * (1-p[i][j]);
         }
         return fjx;
      }

      double cal_fjx_poly_(int j)
      {
         double fjx = 1;
         for (int i = 0; i < m; i++)
         {
            if( x[i * n + j] > WTA_EPS)
               fjx = fjx * (1-(p[i][j] * x[i * n + j]));
         }
         return fjx;
      }

      //Calculate the partial derivative of f_j(x) with respect to x_{ij}
      //use the polynominal form (1 - p_{ij}x_{ij}) at point x_{ij}
      double par_fj_xij_poly_(int i, int j, double fjx)
      {
         //printf("try to calculate partial value \n");
         double p_ij = p[i][j];
         double partial_value_2 = - p_ij;
         double partial_value_1 = - p_ij * fjx;
         
         if (x[i * n + j] > WTA_EPS)
            {
               partial_value_1 =  partial_value_1 / (1 - (p_ij * x[i*n+j])); 
            }
         
         
         for(int i_all = 0; i_all < m; i_all++){
            if(x[i_all * n + j] > WTA_EPS && i_all != i)
            {
               partial_value_2 = partial_value_2 * (1 - (p[i_all][j] * x[i_all *n + j]));
            }
         }
         //printf("use the fjx is %2f, not use is %2f", partial_value_1, partial_value_2);
         assert(partial_value_1 < partial_value_2 + WTA_EPS && partial_value_1 > partial_value_2 - WTA_EPS);
         
         bool par1 = 0;
         if(par1){
            return partial_value_1;
         }
         else{
            return partial_value_2;
         }
      }

      double cal_fjx_exp_(int j)
      {
         double fjx = 1;
         for (int i = 0; i < m; i++)
         {
            if( x[i * n + j] > WTA_EPS)
               fjx = fjx * pow(1-p[i][j], x[i*n+j]);
         }
         return fjx;
      }


      

      //Calculate the partial derivative of f_j(x) with respect to x_{ij}
      //use the exponential form (1 - p_{ij})^(x_{ij}) at x_{ij}
      double par_fj_xij_exp_(int i, int j, double fjx)
      {
         double partial_value = log(1 - p[i][j]) * fjx;
         return partial_value;
      }

      double read_xij_value_(int i, int j){
         return x[i * n + j];
      }

      // print the weapon that assign to the target j.
      // return true if all the variales are integer
      bool print_solution_of_targetj(int j)
      {
         int integer_num = 0;
         //printf("m = %d \n", m);
         //printf("the frac variable on target %d is :", j);
         for (int i =0; i < m; i++){
            double i_value = x[i * n + j];
            if (i_value < WTA_EPS || i_value > 1 - WTA_EPS)
            {
               integer_num++;
            }
            //printf("%4f ", i_value);
         }
         //printf("\n the number of integer variable is %d \n", integer_num);
         
         return (integer_num == m);
      }
};

/* Declaration of the data structure for the function benders_callback */

typedef struct {
   /* Parameter to decide when Benders' cuts are going to be separated:
0: only when an integer solution if found
(i.e., wherefrom == CPX_CALLBACK_MIP_CUT_FEAS )
1: even to cut-off fractional solutions, 
at the end of the cplex cut-loop
(i.e., wherefrom == CPX_CALLBACK_MIP_CUT_LAST || 
wherefrom == CPX_CALLBACK_MIP_CUT_FEAS ) */
   int separate_fractional_solutions; 
   /* Environment for the worker LP used to generate Benders' cuts */
   CPXENVptr env; 
   /* Worker LP used to generate Benders' cuts */
   CPXLPptr  lp;  
   /* number of nodes in the ATSP instance */
   int num_nodes; 
   /* Number of columns in the master ILP */
   int num_x_cols;
   /* Number of columns in the worker LP */
   int num_v_cols, num_u_cols;
   /* Data structure to 
      -- read the solution from the master ILP 
      -- update the objective function of the worker LP */
   double *x;
   int *indices;
   /* Data structure to read an unbounded direction (ray) from the worker LP */
   double *ray;
   /* Data structure to add a Benders' cut to the master ILP */
   double *cutval;
   int *cutind;

} USER_CBHANDLE;


/* Declarations for functions in this program */

static int CPXPUBLIC 
benders_callback  (CPXCENVptr env, void *cbdata, int wherefrom, 
      void *cbhandle, int *useraction_p);

static int
set_benders_callback  (CPXENVptr env, WTA* wta),
                      create_master_ILP     (CPXENVptr env, CPXLPptr lp, WTA& wta),
                      init_user_cbhandle    (USER_CBHANDLE *user_cbhandle, int num_nodes, 
                            int separate_fractional_solutions),
                      free_user_cbhandle    (USER_CBHANDLE *user_cbhandle);


static void
free_and_null  (char **ptr),
               usage          (char *progname);



   int
main (int  argc, char *argv[])
{
   int status = 0;
   int solstat;

   double objval;    
   int num_x_cols;
   double *x = NULL; 
   int i, j;
   int m = 5, n = 5;
   int seed = 1;
   int function_form = 0;

   FILE* fin = NULL;

   /* Cplex environment and master ILP */

   CPXENVptr env = NULL;
   CPXLPptr  lp = NULL;

   /* Decide when Benders' cuts are going to be separated:
0: only when a integer solution if found
(i.e., wherefrom == CPX_CALLBACK_MIP_CUT_FEAS )
1: even to cut-off fractional solutions, 
at the end of the cplex cut-loop
(i.e., wherefrom == CPX_CALLBACK_MIP_CUT_LAST || 
wherefrom == CPX_CALLBACK_MIP_CUT_FEAS ) */

   int separate_fractional_solutions; 

   /* Cut callback data structure */
   WTA wta;

   if ( argc > 6 || argc < 2) {
      usage (argv[0]);
      goto TERMINATE;
   }

   if ( (argv[1][0] != '1' && argv[1][0] != '0') || 
        argv[1][1] != '\0' ) {
      usage (argv[0]);
      goto TERMINATE;
   }
   
   if ( argc >= 3)
      function_form = atoi(argv[2]);
  
   if ( argc >= 6)
   {
      m = atoi(argv[3]);
      n = atoi(argv[4]);
      seed = atoi(argv[5]);
   }
   else
   {
      fin = fopen(argv[3], "r");
      seed = atoi(argv[4]);
   }
      


   separate_fractional_solutions = ( argv[1][0] == '0' ? 0 : 1 );
  
   printf("proble size: m = %d , n = %d , seed = %d \n", m, n, seed);
   
   if(fin == NULL)
      wta.Init(m, n, seed);
   else
   {
      wta.Init(fin, seed);
      m = wta.m;
      n = wta.n;
      fclose(fin);
   }

   wta.separate_fractional_solutions = separate_fractional_solutions;
   wta.function_form = function_form;

   printf ("Benders' cuts separated to cut off: ");
   if ( separate_fractional_solutions ) {
      printf ("Integer and fractional infeasible solutions.\n");
   }
   else {
      printf ("Only integer infeasible solutions.\n");
   }
   fflush (stdout);

   /* Init the CPLEX environment */

   env = CPXopenCPLEX (&status);
   if ( env == NULL ) {
      fprintf (stderr, "Failure in CPXopenCPLEX, status = %d.\n", status);
      goto TERMINATE;
   }

   /* Turn on output to the screen */

   status = CPXsetintparam (env, CPXPARAM_ScreenOutput, CPX_ON); 
   if ( status ) {
      fprintf (stderr, "Failed to turn on screen indicator, status = %d.\n",
               status);
      goto TERMINATE;
   }

   /* Set MIP log interval to 1 */

   /*
   status = CPXsetintparam (env, CPXPARAM_MIP_Interval, 1000);
   if ( status )  {
      fprintf (stderr,
             "Failed to set CPXPARAM_MIP_Interval, status = %d.\n", status);
      goto TERMINATE;
   }*/

   /* Create the master ILP */

   lp = CPXcreateprob (env, &status, "master_ILP.lp");
   if ( lp == NULL ) {
      fprintf (stderr, "Failure in CPXcreateprob, status = %d.\n", status);
      goto TERMINATE;
   }

   status = create_master_ILP (env, lp, wta);
   if ( status ) {
      fprintf (stderr,
               "Failed to create the master ILP.\n");
      goto TERMINATE;
   }

   /* Set up environment parameters to use the function benders_callback 
      as cut callback function */

   status = set_benders_callback (env, &wta);
   if ( status ) {
      fprintf (stderr,
               "Failure in function set_benders_callback: status = %d.\n",
               status);
      goto TERMINATE;
   }

   /* Optimize the problem and obtain solution status */

   status = CPXmipopt (env, lp);
   if ( status ) {
      fprintf (stderr, "Failed to optimize MIP, status = %d.\n", status);
      goto TERMINATE;
   }

   solstat = CPXgetstat (env, lp);
   printf ("\nSolution status: %d\n", solstat);

   /* Write out the objective value */ 

   if ( CPXgetobjval (env, lp, &objval) ) {
      printf ("Failed to obtain objective value.\n");
   }
   else {
      printf ("Objective value: %17.10e\n", objval);
   }

   if ( solstat == CPXMIP_OPTIMAL ) {
    
      /* Write out the optimal tour */
      
      printf ("Solution status is CPX_STAT_OPTIMAL\n");
      num_x_cols = CPXgetnumcols (env, lp);
      x = (double*)malloc (num_x_cols * sizeof(*x));
      if ( x == NULL ) {
         fprintf (stderr, "No memory for x array.\n");
         status = -1;
         goto TERMINATE;
      }
      status = CPXgetx (env, lp, x, 0, num_x_cols-1);
      if ( status ) {
         fprintf (stderr, "Failed to obtain solution, status = %d.\n", status);
         goto TERMINATE;
      }
      /*
      for(i = 0; i < m*n; i++)
         printf("%.0f, ", x[i]);
      for(; i < num_x_cols; i++)
         printf("%.8f, ", x[i]);
      printf("\n");
      */
      printf("Node: %d\n", CPXgetnodecnt(env, lp));
   } 
   else {
      printf ("Solution status is not CPX_STAT_OPTIMAL\n");
      printf("Node: %d\n", CPXgetnodecnt(env, lp));
   }
   
   wta.Delete();
   
TERMINATE:

   /* Free the allocated memory if necessary */

   free_and_null ((char **) &x);

   if ( lp != NULL ) {
      int local_status = CPXfreeprob (env, &lp);
      if ( local_status ) {
         fprintf (stderr, "CPXfreeprob failed, error code %d.\n",
                  local_status);
         status = local_status;
      }
   }

   /* Free the CPLEX environment, if necessary */

   if ( env != NULL ) {
      int local_status = CPXcloseCPLEX (&env);
      if ( local_status ) {
         fprintf (stderr, 
                  "Could not close CPLEX environment, status = %d.\n", 
                  local_status);
         status = local_status;
      }
   }

   return status;

} /* END main */


/* This routine initializes the data structure for 
   the user callback function benders_callback.
   In particular, it creates the worker LP that will be used by 
   the function benders_callback to separate Benders' cuts.

   Worker LP model (dual of flow constraints and 
   capacity constraints of the flow MILP)

   Modeling variables:
   forall k in V0, i in V:
      u(k,i) = dual variable associated with flow constraint (k,i)
             
   forall k in V0, forall (i,j) in A: 
      v(k,i,j) = dual variable associated with capacity constraint (k,i,j)
   
   Objective:
   minimize sum(k in V0) sum((i,j) in A) x(i,j) * v(k,i,j) 
            - sum(k in V0) u(k,0) + sum(k in V0) u(k,k)

   Constraints:
   forall k in V0, forall (i,j) in A: u(k,i) - u(k,j) <= v(k,i,j)

   Nonnegativity on variables v(k,i,j)
   forall k in V0, forall (i,j) in A: v(k,i,j) >= 0 */

static int
init_user_cbhandle  (USER_CBHANDLE *user_cbhandle, int num_nodes, 
                     int separate_fractional_solutions)
{
   int i, j, k;
   int status = 0;
   
   /* Data structures to create columns and add rows */

   int num_rows, nzcnt;
   char *sense = NULL;
   double *rhs = NULL;
   int *rmatbeg = NULL;
   int *rmatind = NULL;
   double *rmatval = NULL;
   char *colname = NULL;

   /* Init user_cbhandle */

   user_cbhandle->separate_fractional_solutions = separate_fractional_solutions;
   user_cbhandle->num_nodes  = num_nodes;
   user_cbhandle->num_x_cols = num_nodes * num_nodes;
   user_cbhandle->num_v_cols = (num_nodes - 1) * user_cbhandle->num_x_cols;
   user_cbhandle->num_u_cols = (num_nodes - 1) * num_nodes;
   user_cbhandle->env        = NULL;
   user_cbhandle->lp         = NULL;
   user_cbhandle->x          = NULL;
   user_cbhandle->indices    = NULL;
   user_cbhandle->ray        = NULL;
   user_cbhandle->cutval     = NULL;
   user_cbhandle->cutind     = NULL;
   
   user_cbhandle->x = (double*)malloc (user_cbhandle->num_x_cols *
                              sizeof(*user_cbhandle->x));
   if ( user_cbhandle->x == NULL ) {
      fprintf (stderr, "No memory for x array.\n");
      goto TERMINATE;
   }
   user_cbhandle->indices = (int* )malloc (user_cbhandle->num_x_cols *
                                    sizeof(*user_cbhandle->indices));
   if ( user_cbhandle->indices == NULL ) {
      fprintf (stderr, "No memory for indices array.\n");
      goto TERMINATE;
   }
   user_cbhandle->ray = (double*)malloc ((user_cbhandle->num_v_cols +
                                 user_cbhandle->num_u_cols) *
                                sizeof(*user_cbhandle->ray));
   if ( user_cbhandle->ray == NULL ) {
      fprintf (stderr, "No memory for ray array.\n");
      goto TERMINATE;
   }
   user_cbhandle->cutval = (double*)malloc (user_cbhandle->num_x_cols *
                                   sizeof(*user_cbhandle->cutval));
   if ( user_cbhandle->cutval == NULL ) {
      fprintf (stderr, "No memory for cutval array.\n");
      goto TERMINATE;
   }
   user_cbhandle->cutind = (int*)malloc (user_cbhandle->num_x_cols *
                                   sizeof(*user_cbhandle->cutind));
   if ( user_cbhandle->cutind == NULL ) {
      fprintf (stderr, "No memory for cutind array.\n");
      goto TERMINATE;
   }
   
   /* Create the environment for the worker LP */

   user_cbhandle->env = CPXopenCPLEX (&status);
   if ( user_cbhandle->env == NULL ) {
      fprintf (stderr, 
         "Could not open CPLEX environment for the worker LP: status = %d.\n", 
         status); 
      goto TERMINATE;
   }

   /* Turn off the presolve reductions */

   status = CPXsetintparam (user_cbhandle->env, CPXPARAM_Preprocessing_Reduce, 0);
   if ( status ) {
      fprintf(stderr, 
         "Failed to set CPXPARAM_Preprocessing_Reduce, status = %d.\n", status);
      goto TERMINATE;
   }

   /* Create the worker LP */

   user_cbhandle->lp = CPXcreateprob (user_cbhandle->env, &status, "atsp_worker.lp");
   if ( user_cbhandle->lp == NULL ) {
      fprintf (stderr, "Failed to create the worker LP: status = %d\n", status);
      goto TERMINATE;
   }

   /* Allocate memory for column names */

   colname = (char*)malloc (100 * sizeof(*colname));
   if ( colname == NULL ) {
      fprintf (stderr, "No memory for colname array.\n");
      status = -1;
      goto TERMINATE;
   }
  
   /* Create variables v(k,i,j), one per time 
      For simplicity, also dummy variables v(k,i,i) are created.
      Those variables are fixed to 0 and do not partecipate to 
      the constraints */
   
   for (k = 1; k < num_nodes; ++k) {
      for (i = 0; i < num_nodes; ++i) {
         for (j = 0; j < num_nodes; ++j) {
            double ub = ( i == j ? 0. : CPX_INFBOUND );
            sprintf (colname, "v.%d.%d.%d", k, i, j);
            status = CPXnewcols (user_cbhandle->env, user_cbhandle->lp, 1, 
                                 NULL, NULL, &ub, NULL, &colname);
            if ( status ) {
               fprintf (stderr, "Error in CPXnewcols, status = %d.\n", status);
               goto TERMINATE;
            }
         }
      }
   }

   /* Create variables u(k,i), one per time */
   
   for (k = 1; k < num_nodes; ++k) {
      for (i = 0; i < num_nodes; ++i) {
         double obj = 0.;
         double lb = -CPX_INFBOUND;
         double ub = CPX_INFBOUND;
         sprintf (colname, "u.%d.%d", k, i);
         if ( i == 0 )
            obj = -1.;
         else if ( i == k )
            obj = 1.;
         status = CPXnewcols (user_cbhandle->env, user_cbhandle->lp, 1, 
                              &obj, &lb, &ub, NULL, &colname); 
         if ( status ) {
            fprintf (stderr, "Error in CPXnewcols, status = %d.\n", status);
            goto TERMINATE;
         }
      }
   }

   /* Init data structures for CPXaddrows */
   
   num_rows = user_cbhandle->num_x_cols * (num_nodes - 1);

   rhs = (double*)malloc (num_rows * sizeof (*rhs));
   if ( rhs == NULL ) {
      fprintf (stderr, "No memory for rhs array.\n");
      status = -1;
      goto TERMINATE;
   }
   sense = (char*)malloc (num_rows * sizeof (*sense));
   if ( sense == NULL ) {
      fprintf (stderr, "No memory for sense array.\n");
      status = -1;
      goto TERMINATE;
   }
   rmatbeg = (int*)malloc ( (num_rows + 1) * sizeof (*rmatbeg));
   if ( rmatbeg == NULL ) {
      fprintf (stderr, "No memory for rmatbeg array.\n");
      status = -1;
      goto TERMINATE;
   }
   rmatind = (int*)malloc (3 * num_rows * sizeof (*rmatind));
   if ( rmatind == NULL ) {
      fprintf (stderr, "No memory for rmatind array.\n");
      status = -1;
      goto TERMINATE;
   }
   rmatval = (double*)malloc (3 * num_rows * sizeof (*rmatval));
   if ( rmatval == NULL ) {
      fprintf (stderr, "No memory for rmatval array.\n");
      status = -1;
      goto TERMINATE;
   }

   /* Populate data structures for CPXaddrows and add all the constraints:
      forall k in V0, forall (i,j) in A: u(k,i) - u(k,j) <= v(k,i,j) */
   
   num_rows = 0;
   nzcnt = 0;
   for (k = 1; k < num_nodes; ++k) {
      for (i = 0; i < num_nodes; ++i) {
         for (j = 0; j < num_nodes; ++j) {
            if ( i != j ) {
               rhs[num_rows]     =  0.;
               sense[num_rows]   =  'L';
               rmatbeg[num_rows] =  nzcnt;
               rmatind[nzcnt]    =  (k-1) * user_cbhandle->num_x_cols +
                                    i * num_nodes + j;
               rmatval[nzcnt++]  = -1.;
               rmatind[nzcnt]    =  user_cbhandle->num_v_cols + 
                                    (k-1) * num_nodes + i; 
               rmatval[nzcnt++]  =  1.;
               rmatind[nzcnt]    =  user_cbhandle->num_v_cols + 
                                    (k-1) * num_nodes + j; 
               rmatval[nzcnt++]  = -1.;
               ++num_rows;
            }
         }
      }
   }
   rmatbeg[num_rows] = nzcnt; 

   status = CPXaddrows (user_cbhandle->env, user_cbhandle->lp, 
                        0, num_rows, nzcnt, rhs, sense, 
                        rmatbeg, rmatind, rmatval, NULL, NULL);
   if ( status ) {
      fprintf (stderr, "Error in CPXaddrows: status = %d\n", status);
      goto TERMINATE;
   }
   
TERMINATE:

   free_and_null ((char **) &colname);
   free_and_null ((char **) &sense);
   free_and_null ((char **) &rhs);
   free_and_null ((char **) &rmatbeg);
   free_and_null ((char **) &rmatind);
   free_and_null ((char **) &rmatval);

   return status;

} /* END init_user_cbhandle */
   

/* 
This routine frees up the data structure for the user cutcallback
created by init_user_cbhandle
*/

static int
free_user_cbhandle  (USER_CBHANDLE *user_cbhandle)
{
   int status = 0; 

   if (user_cbhandle == NULL) goto TERMINATE;
   
   free_and_null ((char **) &user_cbhandle->x);
   free_and_null ((char **) &user_cbhandle->indices);
   free_and_null ((char **) &user_cbhandle->ray);
   free_and_null ((char **) &user_cbhandle->cutval);
   free_and_null ((char **) &user_cbhandle->cutind);
   
   if ( user_cbhandle->lp != NULL ) {
      int local_status = CPXfreeprob (user_cbhandle->env, &(user_cbhandle->lp) );
      if ( local_status ) {
         fprintf (stderr, "CPXfreeprob failed, error code %d.\n", status);
         status = local_status;
      }
      else
         user_cbhandle->lp = NULL;
   }

   if ( user_cbhandle->env != NULL ) {
      int local_status = CPXcloseCPLEX ( &(user_cbhandle->env) );
      if ( local_status ) {
         fprintf (stderr, "CPXcloseCPLEX failed, error code %d.\n", status);
         status = local_status;
      }
      else
         user_cbhandle->env = NULL;
   }

TERMINATE:

   return status; 

} /* END free_user_cbhandle */


/* This routine sets up the environment parameters to use the function
   benders_callback to separate and add Bender's cut during the branch-and-cut.
   benders_callback is always set to generate Benders' cuts 
   as lazy constraints, with CPXsetlazyconstraintcallbackfunc.
   Depending on the parameter user_cbhandle->separate_fractional_solutions,
   benders_callback may also be set to generate Benders' cut 
   as user cutting planes, with CPXsetusercutcallbackfunc. */

static int 
set_benders_callback  (CPXENVptr env, WTA *wta)
{
   int status = 0;

   /* Let MIP callbacks work on the original model */

   status = CPXsetintparam (env, CPXPARAM_MIP_Strategy_CallbackReducedLP,
                            CPX_OFF);
   if ( status )  {
      fprintf (stderr,
        "Failed to set CPXPARAM_MIP_Strategy_CallbackReducedLP, status = %d.\n",
        status);
      goto TERMINATE;
   }

   /* Set the maximum number of threads to 1. 
      This instruction is redundant: If MIP control callbacks are registered, 
      then by default CPLEX uses 1 (one) thread only.
      Note that the current example may not work properly if more than 1 threads 
      are used, because the callback functions modify shared global data.
      We refer the user to the documentation to see how to deal with multi-thread 
      runs in presence of MIP control callbacks. */
   
   status = CPXsetintparam (env, CPXPARAM_Threads, 1);
   if ( status )  {
      fprintf (stderr,
             "Failed to set CPXPARAM_Threads, status = %d.\n", status);
      goto TERMINATE;
   }
   
   status = CPXsetdblparam (env, CPX_PARAM_EPAGAP, 0.0);
   if ( status )  {
      fprintf (stderr,
             "Failed to set CPX_PARAM_EPAGAP, status = %d.\n", status);
      goto TERMINATE;
   }
   
   status = CPXsetdblparam (env, CPX_PARAM_EPGAP, 0.0);
   if ( status )  {
      fprintf (stderr,
             "Failed to set CPX_PARAM_EPGAP, status = %d.\n", status);
      goto TERMINATE;
   }
  
   /* Turn on traditional search for use with control callbacks */

   status = CPXsetintparam (env, CPXPARAM_MIP_Strategy_Search,
                            CPX_MIPSEARCH_TRADITIONAL);
   if ( status )  {
      fprintf (stderr,
             "Failed to set CPXPARAM_MIP_Strategy_Search, status = %d.\n",
             status);
      goto TERMINATE;
   }
   
   status = CPXsetdblparam (env, CPX_PARAM_TILIM, 3600);
   
   if ( status )  {
      fprintf (stderr,
             "Failed to set CPX_PARAM_TILIM, status = %d.\n",
             status);
      goto TERMINATE;
   }

   status = CPXsetintparam(env, CPX_PARAM_RANDOMSEED, wta->seed);
   printf("seed = %d\n", wta->seed);

   /* Set up to use the function benders_callback to generate lazy constraints */

   status = CPXsetlazyconstraintcallbackfunc (env, benders_callback, 
                                                wta);
   if ( status )  {
      fprintf (stderr,
         "Error in CPXsetlazyconstraintcallbackfunc, status = %d.\n",
         status);
      goto TERMINATE;
   }

   /* Set up to use the function benders_callback also to generate user cuts */
   
   if ( wta->separate_fractional_solutions ) {
      status = CPXsetusercutcallbackfunc (env, benders_callback, 
                                          wta);
      if ( status )  {
          fprintf (stderr,
             "Error in CPXsetusercutcallbackfunc, status = %d.\n",
             status);
         goto TERMINATE;
      }
   }

TERMINATE:

   return status;

} /* END set_benders_callback */


/* This function separate Benders' cuts violated by the current solution and
   add them to the current relaxation through CPXcutcallbackadd.
   If benders_callback is called as lazy constraints callback, then 
   wherefrom can be CPX_CALLBACK_MIP_CUT_FEAS or CPX_CALLBACK_MIP_CUT_UNBD.
   If benders_callback is called as user cut callback, then 
   wherefrom can be CPX_CALLBACK_MIP_CUT_LOOP or CPX_CALLBACK_MIP_CUT_LAST.
   Note that the case CPX_CALLBACK_MIP_CUT_UNBD cannot occur in our context, 
   the current LP relaxation is bounded by construnction. */

static int CPXPUBLIC 
benders_callback  (CPXCENVptr env, void *cbdata, int wherefrom, 
                   void *cbhandle, int *useraction_p) 
{
   int status = 0;
   int do_separate = 0;
   WTA* wta = (WTA*) cbhandle;
   int m = wta->m;
   int n = wta->n;
   int i, j, nzcnt;
   int purgeable;
   double sum = 0;

   /* Data structures to add the Benders' cut */

   *useraction_p = CPX_CALLBACK_DEFAULT;
   
   /* Decide if we want to separate cuts, depending on 
      the parameter wherefrom */
   
   switch (wherefrom) {
      case CPX_CALLBACK_MIP_CUT_FEAS: 
         do_separate = 1; 
         break;
      case CPX_CALLBACK_MIP_CUT_LAST: 
         do_separate = 1;
         break;
      case CPX_CALLBACK_MIP_CUT_LOOP: 
         do_separate = 0;
         break;
      default:
         fprintf (stderr, "Unexpected value of wherefrom: %d\n", wherefrom);
         do_separate = 0;
   }

   if( !do_separate ) goto TERMINATE; 

   /* Get the current x solution */
   
   status = CPXgetcallbacknodex (env, cbdata, wherefrom, wta->x, 0, m*n+n-1);
   if ( status ) {
      fprintf (stderr, "Error in CPXgetcallbacknodex: status = %d\n", status);
      goto TERMINATE;
   }

   for (j = 0; j < n; ++j) {
      nzcnt = 0;
      sum = 0.0;

      double temp_fjx = 0;
      if(wta->function_form == 0)
      {
         temp_fjx = wta->cal_fjx_exp_(j);
      }
      else{
         temp_fjx = wta->cal_fjx_poly_(j);
      }

      wta->rhs = - temp_fjx;
      for (i = 0; i < m; ++i) 
      {
         wta->cutind[nzcnt] = i * n + j;
         double partial_value = 0;
         if (wta->function_form == 0)
         {
            partial_value = wta->par_fj_xij_exp_(i,j,temp_fjx); 
         }
         if (wta->function_form == 1)
         {
            partial_value = wta->par_fj_xij_poly_(i,j,temp_fjx); 
         }
         wta->cutval[nzcnt] = partial_value;
         if(wta->x[i*n+j] > WTA_EPS)
         {
            wta->rhs = wta->rhs + wta->cutval[nzcnt] * wta->x[i * n + j];
         }
         nzcnt++;
      }
      wta->cutind[nzcnt] = m*n + j;
      wta->cutval[nzcnt] = -1;
      nzcnt++;

      purgeable = CPX_USECUT_FORCE;

      /* With this choice of the purgeable parameter,
         the cut is added to the current relaxation and
         it cannot be purged.
         Note that the value CPX_USECUT_FILTER is not allowed if
         Benders' cuts are added as lazy constraints (i.e., if 
         wherefrom is CPX_CALLBACK_MIP_CUT_LOOP or 
         CPX_CALLBACK_MIP_CUT_LAST). 
         Possible values and meaning of the purgeable parameter 
         are illustrated in the documentation of CPXcutcallbackadd */

      /* Add the cut to the master ILP */
      if(wta->x[m * n + j] + WTA_EPS < temp_fjx)
      {
         //printf("------ the cut is from %ld ---------\n", wherefrom);
         bool is_integer = wta->print_solution_of_targetj(j);
         if(!is_integer)
         {
            //printf("add a filter cut.\n" );
            purgeable = CPX_USECUT_FILTER;
            if(wherefrom != CPX_CALLBACK_MIP_CUT_LAST)
            {
               printf("CPX_CALLBACK_MIP_CUT_FEAS = %d\n", wherefrom == CPX_CALLBACK_MIP_CUT_FEAS);
            }
         }
         /*
         else
         {
            printf("add a force cut.\n" );
            if(wherefrom != CPX_CALLBACK_MIP_CUT_FEAS)
               printf("CPX_CALLBACK_MIP_CUT_LAST = %d\n", wherefrom == CPX_CALLBACK_MIP_CUT_LAST);
         }
         double ss = 0;
         double ff = 1;
         for(int ii = 0; ii < nzcnt - 1; ii++)
         {
            int ind = wta->cutind[ii];
            ss += wta->cutval[ii] * (wta->real_sol[ind] - wta->x[ind]);
            ff = ff * (1 - wta->p[ii][j] * wta->x[ind]);
         }
         if(ss + ff > wta->real_sol[m*n+j] + 1e-8)
         {
            for(int ii = 0; ii < nzcnt - 1; ii++)
            {
               int ind = wta->cutind[ii];
               //if(wta->real_sol[ind] > 1e-8)
                  printf("%.5f %.5f(%.0f,%.0f) ", wta->p[ii][j], wta->cutval[ii], wta->x[ind], wta->real_sol[ind]);
            }
            int ind = wta->cutind[nzcnt-1];
            //if(wta->real_sol[ind] > 1e-8)
            printf("%.5f(%.8f,%.8f) ", wta->cutval[nzcnt-1],  wta->x[m*n + j],  wta->real_sol[m*n + j]);
            printf(" --- %d\n", j);
         }
         assert(ss + ff <= wta->real_sol[m*n+j] + 1e-8);
         printf("rhs = %.2f, naznt = %d \n", wta->rhs, nzcnt);

         for(int i = 0; i < nzcnt; i++)
         {
            printf("%d, %.4f ",wta->cutind[i], wta->cutval[i]);
         }
         printf("\n");
         */


         status = CPXcutcallbackadd (env, cbdata, wherefrom, nzcnt, wta->rhs, 'L', 
            wta->cutind, wta->cutval,
            purgeable);
      }
      if ( status ) {
         fprintf (stderr, "Error in CPXcutcallbackadd: status = %d\n", status);
         goto TERMINATE;
      }
   }

      /* Tell CPLEX that cuts have been created */ 

   *useraction_p = CPX_CALLBACK_SET; 

TERMINATE:

      /* If an error has been encountered, we fail */

   if ( status ) *useraction_p = CPX_CALLBACK_FAIL; 

   return status;

} /* END benders_callback */


/* This routine creates the master ILP (arc variables x and degree constraints).

   Modeling variables:
   forall (i,j) in A: 
      x(i,j) = 1, if arc (i,j) is selected
             = 0, otherwise

   Objective:
   minimize sum((i,j) in A) c(i,j) * x(i,j)

   Degree constraints:
   forall i in V: sum((i,j) in delta+(i)) x(i,j) = 1
   forall i in V: sum((j,i) in delta-(i)) x(j,i) = 1

   Binary constraints on arc variables:
   forall (i,j) in A: x(i,j) in {0, 1} */

static int
create_master_ILP   (CPXENVptr env, CPXLPptr lp, WTA& wta)
{
   int i, j;
   int status = 0;
   int n = wta.n;
   int m = wta.m;
   double* w = wta.w;
   double** p = wta.p;
  
   char sense, *colname = NULL;
   int nzcnt, rmatbeg, *rmatind = NULL;
   double rhs, *rmatval = NULL;
   
   
   /* Change problem type */
   
   status = CPXchgprobtype (env, lp, CPXPROB_MILP);
   if ( status ) {
      fprintf (stderr, "Error in CPXchgprobtype, status = %d.\n", status);
      goto TERMINATE;
   }

   /* Create arc variables x(i,j), one per time 
      For simplicity, also dummy variables x(i,i) are created.
      Those variables are fixed to 0 and do not partecipate to 
      the constraints */
   
   colname = (char*)malloc ( 100* sizeof(*colname));
   if ( colname == NULL ) {
      fprintf (stderr, "No memory for colname array.\n");
      status = -1;
      goto TERMINATE;
   }
   for (i = 0; i < n; ++i) 
   {
      for (j = 0; j < m; ++j) 
      {
         double cost = 0;
         double lb   = 0;
         double ub   = 1;
         char type   = 'B';
         sprintf (colname, "x.%d.%d", i, j);
         status = CPXnewcols (env, lp, 1, &cost, &lb, &ub, &type, &colname);
         if ( status ) {
            fprintf (stderr, "Error in CPXnewcols, status = %d.\n", status);
            goto TERMINATE;
         }
      }
   }
   for (j = 0; j < n; ++j) 
   {
      double cost = w[j];
      double lb   = 0;
      double ub   = 1;
      char type   = 'C';
      sprintf (colname, "theta.%d", j);
      status = CPXnewcols (env, lp, 1, &cost, &lb, &ub, &type, &colname);
      if ( status ) {
         fprintf (stderr, "Error in CPXnewcols, status = %d.\n", status);
         goto TERMINATE;
      }
   }

   /* Init data structures to add capacity constraints */
   
   rhs = 1.;
   sense = 'L';
   rmatbeg = 0;
   rmatind = (int*)malloc ( m * sizeof (*rmatind));
   if ( rmatind == NULL ) {
      fprintf (stderr, "No memory for rmatind array.\n");
      status = -1;
      goto TERMINATE;
   }
   rmatval = (double*)malloc ( m * sizeof (*rmatval));
   if ( rmatval == NULL ) {
      fprintf (stderr, "No memory for rmatval array.\n");
      status = -1;
      goto TERMINATE;
   }

   /* Add the out degree constraints, one at a time:
      forall i in V: sum((i,j) in delta+(i)) x(i,j) = 1 */

   for (i = 0; i < m; ++i) {
      nzcnt = 0;
      for (j = 0; j < n; ++j) {
         rmatind[nzcnt]   = i * n + j;
         rmatval[nzcnt++] = 1.;
      }
      status = CPXaddrows (env, lp, 0, 1, n, &rhs, &sense,
                           &rmatbeg, rmatind, rmatval, NULL, NULL);
      if ( status ) {
         fprintf (stderr, "Error in CPXaddrows, status = %d.\n", status);
         goto TERMINATE;
      }
   }
   
TERMINATE:

   free_and_null ((char **) &colname);
   free_and_null ((char **) &rmatind);
   free_and_null ((char **) &rmatval);
   
   return status;

} /* END create_master_ILP */


/* This routine read an array of doubles from an input file  */

static int
read_file (FILE *in, WTA& wta)
{
   int  status = 0;
   char ch;
      
   int read = fscanf (in, "%c", &ch);
   
#if 0
   int  max, num;

   num = 0;
   max = 10;

   for (;;) {
      int read = fscanf (in, "%c", &ch);
      if ( read == 0 ) {
         status = -1;
         goto TERMINATE;
      }
      if ( ch == '\t' ||
           ch == '\r' ||
           ch == ' '  ||
           ch == '\n'   ) continue;
      if ( ch == '[' ) break;
      status = -1;
      goto TERMINATE;
   }

   for(;;) {
      int read = fscanf (in, "%lg", (*data_p)+num);
      if ( read == 0 ) {
         status = -1;
         goto TERMINATE;
      }
      num++;
      if ( num >= max ) {
         max *= 2;
         *data_p = (double*)realloc(*data_p, max * sizeof(double));
         if ( *data_p == NULL ) {
            status = CPXERR_NO_MEMORY;
            goto TERMINATE;
         }
      }
      do {
         read = fscanf (in, "%c", &ch);
         if ( read == 0 ) {
            status = -1;
            goto TERMINATE;
         }
      } while (ch == ' ' || ch == '\n' || ch == '\t'  || ch == '\r');
      if ( ch == ']' ) break;
      else if ( ch != ',' ) {
         status = -1;
         goto TERMINATE;
      }
   }

   *num_p = num;
#endif
TERMINATE:

   return status;

} /* END read_file */


/* This routine frees up the pointer *ptr, and sets *ptr to 
   NULL */

static void
free_and_null (char **ptr)
{
   if ( *ptr != NULL ) {
      free (*ptr);
      *ptr = NULL;
   }
} /* END free_and_null */ 


static void
usage (char *progname)
{
   fprintf (stderr,
      "Usage:     %s {0|1} [m] [n] [seed] [form]\n", progname);
   fprintf (stderr,
      " 0:        Benders' cuts only used as lazy constraints,\n");
   fprintf (stderr,
      "           to separate integer infeasible solutions.\n");
   fprintf (stderr,
      " 1:        Benders' cuts also used as user cuts,\n");
   fprintf (stderr,
      "           to separate fractional infeasible solutions.\n");
   fprintf (stderr,
      " m:        number of weapens.\n");
   fprintf (stderr, 
      " n:        number of targets.\n");
   fprintf (stderr, 
      " seed:     random seed.\n");
   fprintf (stderr, 
      " form:     0 for exponential form, 1 for polynominal form.\n");
   
} /* END usage */

