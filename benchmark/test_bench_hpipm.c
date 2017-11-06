/*
 *    This file is part of acados.
 *
 *    acados is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    acados is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with acados; if not, write to the Free Software Foundation,
 *    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#define _SVID_SOURCE
// standard
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
// blasfeo
#include <blasfeo_target.h>
#include <blasfeo_common.h>
#include <blasfeo_v_aux_ext_dep.h>
#include <blasfeo_d_aux_ext_dep.h>
#include <blasfeo_i_aux_ext_dep.h>
#include <blasfeo_d_aux.h>
#include <blasfeo_d_blas.h>
// hpipm
#include "../include/hpipm_d_dense_qp.h"
#include "../include/hpipm_d_dense_qp_sol.h"
#include "../include/hpipm_d_dense_qp_ipm.h"
// benchmark
#include "dense_qp_in_common.h"
// define regularization parameter
#define EPSILON 1e-5

/************************************************
              benchmark problem
************************************************/
int main() {

    printf("\n");
    printf("\n");
    printf("\n");
    printf(
        " HPIPM -- High-Performance Interior Point Method.\n");
    printf("\n");
    printf("\n");
    printf("\n");

    int nQP = 0 , nvc = 0, nec = 0, ngc = 0;
    int nproblems, i, j;
    struct dirent **namelist;
    char resstr[200], OQPproblem[200];
    char *problem;
    nproblems = scandir("/home/jiangyn/hpipm/benchmark/problems", &namelist, NULL, alphasort);
    /*
    int ii,jj;
    char filename[1024];
    FILE * pFile;                                         
    */
    /************** benchmark loop *********************/
    for (i = 0; i < nproblems; i++) {

        /************************************************
        * bechmark data setting
        ************************************************/

        /* skip special directories and zip file cuter.*bz2 */
        if (namelist[i]->d_name[0] == '.' || namelist[i]->d_name[0] == 'c') {
            free(namelist[i]);
            continue;
        }
        problem = namelist[i]->d_name;
        snprintf(OQPproblem, 199, "/home/jiangyn/hpipm/benchmark/problems/%s/", problem);

        /* read dimensions */
        readOQPdimensions( OQPproblem, &nQP, &nvc, &ngc, &nec );

        /************************************************
        * dense qp qposes
        ************************************************/

        int nc = ngc-nec; // inequality constraint
        dense_qp_in_qpoases *qpd_qpoases = create_dense_qp_in_qpoases(nvc, nec, nc);

        double *H = qpd_qpoases->H;
        double *g = qpd_qpoases->g;
        double *lbx = qpd_qpoases->lbx;
        double *ubx = qpd_qpoases->ubx;
        double *C = qpd_qpoases->C;
        double *lbC = qpd_qpoases->lbC;
        double *ubC = qpd_qpoases->ubC;

        /* read data */
        readOQPdata(OQPproblem, &nQP, &nvc, &ngc, &nec, H, g, C, lbx, ubx, lbC, ubC, NULL, NULL, NULL);
        // print data to text files
        /*
        snprintf(filename, sizeof(filename), "matrixH%d.txt", i);
        pFile = fopen(filename,"w");
        for (ii = 0; ii < nv; ii++){
           for (jj = 0; jj < nv; jj++)
           {
               fprintf(pFile, "%e ", H[ii*nv+jj]);
           }
           fputc('\n', pFile);
        }
        fclose(pFile);                                                                                                                                                                                      
        */

        /************************************************
        * qposes to hpipm
        ************************************************/

        qpoases_to_hpipm *tran_space = create_qpoases_to_hpipm(nvc, nec, ngc);

        /************************************************
        * dense qp
        ************************************************/

        int nsc = 0;
        int qp_size = d_memsize_dense_qp(nvc, nec, nvc, ngc, nsc);
        void *qp_mem = calloc(qp_size,1);

        struct d_dense_qp qpd_hpipm;
        d_create_dense_qp(nvc, nec, nvc, ngc, nsc, &qpd_hpipm, qp_mem);

        /* qpd_qpoases -> qpd_hpipm */
        qpd_qpoases_to_hpipm(qpd_qpoases, &qpd_hpipm, tran_space);


        /************************************************
        * dense sol
        ************************************************/

        int qp_sol_size = d_memsize_dense_qp_sol(nvc, nec, nvc, ngc, nsc);
        void *qp_sol_mem = calloc(qp_sol_size,1);

        struct d_dense_qp_sol qpd_sol;
        d_create_dense_qp_sol(nvc, nec, nvc, ngc, nsc, &qpd_sol, qp_sol_mem);

        /************************************************
        * ipm arg
        ************************************************/

        int ipm_arg_size = d_memsize_dense_qp_ipm_arg(&qpd_hpipm);
        void *ipm_arg_mem = calloc(ipm_arg_size,1);

        struct d_dense_qp_ipm_arg argd;
        d_create_dense_qp_ipm_arg(&qpd_hpipm, &argd, ipm_arg_mem);
        d_set_default_dense_qp_ipm_arg(&argd);
        /* consistent with setting in acore */
        argd.res_g_max = 1e-6;
        argd.res_b_max = 1e-8;
        argd.res_d_max = 1e-8;
        argd.res_m_max = 1e-8;
        argd.iter_max = 50;
        argd.stat_max = 50;
        argd.alpha_min = 1e-8;
        argd.mu0 = 1;

        /************************************************
        * dense ipm
        ************************************************/
        int ipm_size = d_memsize_dense_qp_ipm(&qpd_hpipm, &argd);
        void *ipm_mem = calloc(ipm_size,1);

        struct d_dense_qp_ipm_workspace workspace;
        d_create_dense_qp_ipm(&qpd_hpipm, &argd, &workspace, ipm_mem);

        int hpipm_return; // 0 normal; 1 max iter

        hpipm_return = d_solve_dense_qp_ipm(&qpd_hpipm, &qpd_sol, &argd, &workspace);

        /************************************************
        * print ipm statistics
        ************************************************/
        printf("Problem %d\n", i-1);
        if (hpipm_return == 1) {
           /* print original H*/
//            printf("\nH_org =\n");
//            d_print_strmat(nvc, nvc, qpd_hpipm.Hg, 0, 0);
            for (j = 0; j < nvc; j++) {
                 H[j*nvc+j] = H[j*nvc+j] + EPSILON;
            }
            /* print modefied H*/
            printf("\nH_reg =\n");
            d_print_strmat(nvc, nvc, qpd_hpipm.Hg, 0, 0);

            qpd_qpoases_to_hpipm(qpd_qpoases, &qpd_hpipm, tran_space);
            hpipm_return = d_solve_dense_qp_ipm(&qpd_hpipm, &qpd_sol, &argd, &workspace);

            /* print primal solution */
            printf("\n\nipm return = %d\n", hpipm_return);
            printf("\nnew_primal_sol = \n");
            d_print_strvec(nvc, qpd_sol.v, 0);
         }

        printf("\nipm iter = %d\n", workspace.iter);
        printf(" inf norm res: %e, %e, %e, %e, %e\n", workspace.qp_res[0],
                                 workspace.qp_res[1], workspace.qp_res[2],
                                 workspace.qp_res[3], workspace.res_mu);
        printf("\n\n\n\n");


        /************************************************
        * free memory
        ************************************************/

        free(qpd_qpoases);
        free(tran_space);
        free(qp_mem);
      	free(qp_sol_mem);
      	free(ipm_mem);

    }

    /************************************************
    * return
    ************************************************/

  	return 0;

}
