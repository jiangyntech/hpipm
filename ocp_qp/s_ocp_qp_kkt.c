/**************************************************************************************************
*                                                                                                 *
* This file is part of HPIPM.                                                                     *
*                                                                                                 *
* HPIPM -- High Performance Interior Point Method.                                                *
* Copyright (C) 2017 by Gianluca Frison.                                                          *
* Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              *
* All rights reserved.                                                                            *
*                                                                                                 *
* HPMPC is free software; you can redistribute it and/or                                          *
* modify it under the terms of the GNU Lesser General Public                                      *
* License as published by the Free Software Foundation; either                                    *
* version 2.1 of the License, or (at your option) any later version.                              *
*                                                                                                 *
* HPMPC is distributed in the hope that it will be useful,                                        *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                                  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                                            *
* See the GNU Lesser General Public License for more details.                                     *
*                                                                                                 *
* You should have received a copy of the GNU Lesser General Public                                *
* License along with HPMPC; if not, write to the Free Software                                    *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA                  *
*                                                                                                 *
* Author: Gianluca Frison, gianluca.frison (at) imtek.uni-freiburg.de                             *
*                                                                                                 *
**************************************************************************************************/



#include <math.h>

#include <blasfeo_target.h>
#include <blasfeo_common.h>
#include <blasfeo_s_aux.h>
#include <blasfeo_s_blas.h>

#include "../include/hpipm_s_ocp_qp_size.h"
#include "../include/hpipm_s_ocp_qp.h"
#include "../include/hpipm_s_ocp_qp_sol.h"
#include "../include/hpipm_s_ocp_qp_ipm.h"
#include "../include/hpipm_s_core_qp_ipm.h"
#include "../include/hpipm_s_core_qp_ipm_aux.h"



#define SINGLE_PRECISION

#define AXPY_LIBSTR saxpy_libstr
#define COMPUTE_LAM_T_QP s_compute_lam_t_qp
#define COMPUTE_GAMMA_GAMMA_QP s_compute_Gamma_gamma_qp
#define COMPUTE_GAMMA_QP s_compute_gamma_qp
#define CORE_QP_IPM_WORKSPACE s_core_qp_ipm_workspace
#define DIAAD_SP_LIBSTR sdiaad_sp_libstr
#define GEAD_LIBSTR sgead_libstr
#define GECP_LIBSTR sgecp_libstr
#define GEMM_R_DIAG_LIBSTR sgemm_r_diag_libstr
#define GEMV_DIAG_LIBSTR sgemv_diag_libstr
#define GEMV_N_LIBSTR sgemv_n_libstr
#define GEMV_NT_LIBSTR sgemv_nt_libstr
#define GEMV_T_LIBSTR sgemv_t_libstr
#define OCP_QP s_ocp_qp
#define OCP_QP_IPM_WORKSPACE s_ocp_qp_ipm_workspace
#define OCP_QP_RES s_ocp_qp_res
#define OCP_QP_SIZE s_ocp_qp_size
#define OCP_QP_SOL s_ocp_qp_sol
#define POTRF_L_MN_LIBSTR spotrf_l_mn_libstr
#define PRINT_E_MAT s_print_e_mat
#define PRINT_E_STRVEC s_print_e_strvec
#define PRINT_E_TRAN_STRVEC s_print_e_tran_strvec
#define PRINT_STRMAT s_print_strmat
#define PRINT_STRVEC s_print_strvec
#define PRINT_TRAN_STRVEC s_print_tran_strvec
#define REAL float
#define ROWAD_SP_LIBSTR srowad_sp_libstr
#define ROWEX_LIBSTR srowex_libstr
#define ROWIN_LIBSTR srowin_libstr
#define STRMAT s_strmat
#define STRVEC s_strvec
#define SYMV_L_LIBSTR ssymv_l_libstr
#define SYRK_POTRF_LN_LIBSTR ssyrk_spotrf_ln_libstr
#define TRCP_L_LIBSTR strcp_l_libstr
#define TRMM_RLNN_LIBSTR strmm_rlnn_libstr
#define TRMV_LNN_LIBSTR strmv_lnn_libstr
#define TRMV_LTN_LIBSTR strmv_ltn_libstr
#define TRSV_LNN_LIBSTR strsv_lnn_libstr
#define TRSV_LTN_LIBSTR strsv_ltn_libstr
#define TRSV_LNN_MN_LIBSTR strsv_lnn_mn_libstr
#define TRSV_LTN_MN_LIBSTR strsv_ltn_mn_libstr
#define VECAD_SP_LIBSTR svecad_sp_libstr
#define VECCP_LIBSTR sveccp_libstr
#define VECEX_SP_LIBSTR svecex_sp_libstr
#define VECMULDOT_LIBSTR svecmuldot_libstr
#define VECSC_LIBSTR svecsc_libstr



#define INIT_VAR_OCP_QP s_init_var_ocp_qp
#define COMPUTE_RES_OCP_QP s_compute_res_ocp_qp
#define FACT_SOLVE_KKT_UNCONSTR_OCP_QP s_fact_solve_kkt_unconstr_ocp_qp
#define COND_SLACKS s_cond_slacks
#define FACT_SOLVE_KKT_STEP_OCP_QP s_fact_solve_kkt_step_ocp_qp
#define SOLVE_KKT_STEP_OCP_QP s_solve_kkt_step_ocp_qp



#include "x_ocp_qp_kkt.c"

