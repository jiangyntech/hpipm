/**************************************************************************************************
*                                                                                                 *
* This file is part of HPIPM.                                                                     *
*                                                                                                 *
* HPIPM -- High Performance Interior Point Method.                                                *
* Copyright (C) 2017 by Gianluca Frison.                                                          *
* Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              *
* All rights reserved.                                                                            *
*                                                                                                 *
* HPIPM is free software; you can redistribute it and/or                                          *
* modify it under the terms of the GNU Lesser General Public                                      *
* License as published by the Free Software Foundation; either                                    *
* version 2.1 of the License, or (at your option) any later version.                              *
*                                                                                                 *
* HPIPM is distributed in the hope that it will be useful,                                        *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                                  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                                            *
* See the GNU Lesser General Public License for more details.                                     *
*                                                                                                 *
* You should have received a copy of the GNU Lesser General Public                                *
* License along with HPIPM; if not, write to the Free Software                                    *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA                  *
*                                                                                                 *
* Author: Gianluca Frison, gianluca.frison (at) imtek.uni-freiburg.de                             *
*                                                                                                 *
**************************************************************************************************/



#if defined(RUNTIME_CHECKS)
#include <stdlib.h>
#include <stdio.h>
#endif

#include <blasfeo_target.h>
#include <blasfeo_common.h>
#include <blasfeo_d_aux.h>
#include <blasfeo_d_blas.h>

#include "../include/hpipm_d_ocp_nlp.h"

#define CREATE_STRMAT d_create_strmat
#define CREATE_STRVEC d_create_strvec
#define CVT_MAT2STRMAT d_cvt_mat2strmat
#define CVT_TRAN_MAT2STRMAT d_cvt_tran_mat2strmat
#define CVT_VEC2STRVEC d_cvt_vec2strvec
#define OCP_NLP d_ocp_nlp
#define OCP_NLP_MODEL d_ocp_nlp_model
#define REAL double
#define SIZE_STRMAT d_size_strmat
#define SIZE_STRVEC d_size_strvec
#define STRMAT d_strmat
#define STRVEC d_strvec
#define SYMV_L_LIBSTR dsymv_l_libstr

#define MEMSIZE_OCP_NLP d_memsize_ocp_nlp
#define CREATE_OCP_NLP d_create_ocp_nlp
#define CVT_COLMAJ_TO_OCP_NLP d_cvt_colmaj_to_ocp_nlp



int MEMSIZE_OCP_NLP(int N, int *nx, int *nu, int *nb, int *ng, int *ns)
	{

	int ii;

	int nuxM = 0;
	for(ii=0; ii<=N; ii++)
		{
		nuxM = nu[ii]+nx[ii]>nuxM ? nu[ii]+nx[ii] : nuxM;
		}

	int size = 0;

	size += 5*(N+1)*sizeof(int); // nx nu nb ng ns
	size += 2*(N+1)*sizeof(int *); // idxb idxs
	size += 1*N*sizeof(struct OCP_NLP_MODEL); // model
	size += 2*(N+1)*sizeof(struct STRMAT); // RSQ DCt
	size += 4*(N+1)*sizeof(struct STRVEC); // rq d Z z
	size += 2*sizeof(struct STRVEC); // e0 tmp_nuxM

	for(ii=0; ii<=N; ii++)
		{
		size += nb[ii]*sizeof(int); // idxb
		size += ns[ii]*sizeof(int); // idxs
		size += SIZE_STRMAT(nu[ii]+nx[ii], nu[ii]+nx[ii]); // RSQ
		size += SIZE_STRVEC(nu[ii]+nx[ii]); // rq
		size += SIZE_STRMAT(nu[ii]+nx[ii], ng[ii]); // DCt
		size += 1*SIZE_STRVEC(2*nb[ii]+2*ng[ii]); // Z z
		size += 2*SIZE_STRVEC(2*ns[ii]); // Z z
		}
	
	size += 1*SIZE_STRVEC(nuxM); // tmp_nuxM

	size = (size+63)/64*64; // make multiple of typical cache line size
	size += 64; // align to typical cache line size
		return size;

	}



void CREATE_OCP_NLP(int N, int *nx, int *nu, int *nb, int *ng, int *ns, struct OCP_NLP *nlp, void *mem)
	{

	int ii;


	// horizon length
	nlp->N = N;


	int nuxM = 0;
	for(ii=0; ii<=N; ii++)
		{
		nuxM = nu[ii]+nx[ii]>nuxM ? nu[ii]+nx[ii] : nuxM;
		}


	// int pointer stuff
	int **ip_ptr;
	ip_ptr = (int **) mem;

	// idxb
	nlp->idxb = ip_ptr;
	ip_ptr += N+1;
	// idxs
	nlp->idxs = ip_ptr;
	ip_ptr += N+1;


	// model
	struct OCP_NLP_MODEL *m_ptr = (struct OCP_NLP_MODEL *) ip_ptr;
	//
	nlp->model = m_ptr;
	m_ptr += N;


	// matrix struct stuff
	struct STRMAT *sm_ptr = (struct STRMAT *) m_ptr;

	// RSQ
	nlp->RSQ = sm_ptr;
	sm_ptr += N+1;
	// DCt
	nlp->DCt = sm_ptr;
	sm_ptr += N+1;


	// vector struct stuff
	struct STRVEC *sv_ptr = (struct STRVEC *) sm_ptr;

	// rq
	nlp->rq = sv_ptr;
	sv_ptr += N+1;
	// d
	nlp->d = sv_ptr;
	sv_ptr += N+1;
	// Z
	nlp->Z = sv_ptr;
	sv_ptr += N+1;
	// z
	nlp->z = sv_ptr;
	sv_ptr += N+1;
	// tmp_nuxM
	nlp->tmp_nuxM = sv_ptr;
	sv_ptr += 1;


	// integer stuff
	int *i_ptr;
	i_ptr = (int *) sv_ptr;

	// nx
	nlp->nx = i_ptr;
	for(ii=0; ii<=N; ii++)
		{
		i_ptr[ii] = nx[ii];
		}
	i_ptr += N+1;
	// nu
	nlp->nu = i_ptr;
	for(ii=0; ii<=N; ii++)
		{
		i_ptr[ii] = nu[ii];
		}
	i_ptr += N+1;
	// nb
	nlp->nb = i_ptr;
	for(ii=0; ii<=N; ii++)
		{
		i_ptr[ii] = nb[ii];
		}
	i_ptr += N+1;
	// ng
	nlp->ng = i_ptr;
	for(ii=0; ii<=N; ii++)
		{
		i_ptr[ii] = ng[ii];
		}
	i_ptr += N+1;
	// ns
	nlp->ns = i_ptr;
	for(ii=0; ii<=N; ii++)
		{
		i_ptr[ii] = ns[ii];
		}
	i_ptr += N+1;
	// idxb
	for(ii=0; ii<=N; ii++)
		{
		(nlp->idxb)[ii] = i_ptr;
		i_ptr += nb[ii];
		}
	// idxs
	for(ii=0; ii<=N; ii++)
		{
		(nlp->idxs)[ii] = i_ptr;
		i_ptr += ns[ii];
		}


	// align to typical cache line size
	long long l_ptr = (long long) i_ptr;
	l_ptr = (l_ptr+63)/64*64;


	// double stuff
	char *c_ptr;
	c_ptr = (char *) l_ptr;

	char *tmp_ptr;

	// RSQ
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRMAT(nu[ii]+nx[ii], nu[ii]+nx[ii], nlp->RSQ+ii, c_ptr);
		c_ptr += (nlp->RSQ+ii)->memory_size;
		}
	// DCt
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRMAT(nu[ii]+nx[ii], ng[ii], nlp->DCt+ii, c_ptr);
		c_ptr += (nlp->DCt+ii)->memory_size;
		}
	// rq
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(nu[ii]+nx[ii], nlp->rq+ii, c_ptr);
		c_ptr += (nlp->rq+ii)->memory_size;
		}
	// d
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(2*nb[ii]+2*ng[ii], nlp->d+ii, c_ptr);
		c_ptr += (nlp->d+ii)->memory_size;
		}
	// Z
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(2*ns[ii], nlp->Z+ii, c_ptr);
		c_ptr += (nlp->Z+ii)->memory_size;
		}
	// z
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(2*ns[ii], nlp->z+ii, c_ptr);
		c_ptr += (nlp->z+ii)->memory_size;
		}
	// tmp_nuxM
	CREATE_STRVEC(nuxM, nlp->tmp_nuxM, c_ptr);
	c_ptr += nlp->tmp_nuxM->memory_size;


	nlp->memsize = MEMSIZE_OCP_NLP(N, nx, nu, nb, ng, ns);


#if defined(RUNTIME_CHECKS)
	if(c_ptr > ((char *) mem) + nlp->memsize)
		{
		printf("\nCreate_ocp_nlp: outsize memory bounds!\n\n");
		exit(1);
		}
#endif


	return;

	}



void CVT_COLMAJ_TO_OCP_NLP(struct OCP_NLP_MODEL *model, REAL **Q, REAL **S, REAL **R, REAL **x_ref, REAL **u_ref, int **idxb, REAL **d_lb, REAL **d_ub, REAL **C, REAL **D, REAL **d_lg, REAL **d_ug, REAL **Zl, REAL **Zu, REAL **zl, REAL **zu, int **idxs, struct OCP_NLP *nlp)
	{

	int N = nlp->N;
	int *nx = nlp->nx;
	int *nu = nlp->nu;
	int *nb = nlp->nb;
	int *ng = nlp->ng;
	int *ns = nlp->ns;

	int ii, jj;

	for(ii=0; ii<N; ii++)
		{
		nlp->model[ii] = model[ii];
		}
	
	for(ii=0; ii<=N; ii++)
		{
		CVT_MAT2STRMAT(nu[ii], nu[ii], R[ii], nu[ii], nlp->RSQ+ii, 0, 0);
		CVT_TRAN_MAT2STRMAT(nu[ii], nx[ii], S[ii], nu[ii], nlp->RSQ+ii, nu[ii], 0);
		CVT_MAT2STRMAT(nx[ii], nx[ii], Q[ii], nx[ii], nlp->RSQ+ii, nu[ii], nu[ii]);
		CVT_VEC2STRVEC(nu[ii], u_ref[ii], nlp->tmp_nuxM, 0);
		CVT_VEC2STRVEC(nx[ii], x_ref[ii], nlp->tmp_nuxM, nu[ii]);
		SYMV_L_LIBSTR(nu[ii]+nx[ii], nu[ii]+nx[ii], 1.0, nlp->RSQ+ii, 0, 0, nlp->tmp_nuxM, 0, 0.0, nlp->rq+ii, 0, nlp->rq+ii, 0);
		}
	
	for(ii=0; ii<=N; ii++)
		{
		if(nb[ii]>0)
			{
			for(jj=0; jj<nb[ii]; jj++)
				nlp->idxb[ii][jj] = idxb[ii][jj];
			CVT_VEC2STRVEC(nb[ii], d_lb[ii], nlp->d+ii, 0);
			CVT_VEC2STRVEC(nb[ii], d_ub[ii], nlp->d+ii, nb[ii]+ng[ii]);
			}
		}
	
	for(ii=0; ii<=N; ii++)
		{
		if(ng[ii]>0)
			{
			CVT_TRAN_MAT2STRMAT(ng[ii], nu[ii], D[ii], ng[ii], nlp->DCt+ii, 0, 0);
			CVT_TRAN_MAT2STRMAT(ng[ii], nx[ii], C[ii], ng[ii], nlp->DCt+ii, nu[ii], 0);
			CVT_VEC2STRVEC(ng[ii], d_lg[ii], nlp->d+ii, nb[ii]);
			CVT_VEC2STRVEC(ng[ii], d_ug[ii], nlp->d+ii, 2*nb[ii]+ng[ii]);
			}
		}

	for(ii=0; ii<=N; ii++)
		{
		if(ns[ii]>0)
			{
			for(jj=0; jj<ns[ii]; jj++)
				nlp->idxs[ii][jj] = idxs[ii][jj];
			CVT_VEC2STRVEC(ns[ii], Zl[ii], nlp->Z+ii, 0);
			CVT_VEC2STRVEC(ns[ii], Zu[ii], nlp->Z+ii, ns[ii]);
			CVT_VEC2STRVEC(ns[ii], zl[ii], nlp->z+ii, 0);
			CVT_VEC2STRVEC(ns[ii], zu[ii], nlp->z+ii, ns[ii]);
			}
		}

	return;

	}




