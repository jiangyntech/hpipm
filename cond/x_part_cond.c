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

void COMPUTE_QP_SIZE_OCP2OCP(struct OCP_QP_SIZE *ocp_size, struct OCP_QP_SIZE *part_dense_size)
	{

	int N = ocp_size->N;
	int *nx = ocp_size->nx;
	int *nu = ocp_size->nu;
	int *nb = ocp_size->nb;
	int *nbx = ocp_size->nbx;
	int *nbu = ocp_size->nbu;
	int *ng = ocp_size->ng;
	int *ns = ocp_size->ns;

	int N2 = part_dense_size->N;
	int *nx2 = part_dense_size->nx;
	int *nu2 = part_dense_size->nu;
	int *nb2 = part_dense_size->nb;
	int *nbx2 = part_dense_size->nbx;
	int *nbu2 = part_dense_size->nbu;
	int *ng2 = part_dense_size->ng;
	int *ns2 = part_dense_size->ns;

	int ii, jj;

	int N1 = N/N2; // (floor) horizon of small blocks
	int R1 = N - N2*N1; // the first R1 blocks have horizon N1+1
	int M1 = R1>0 ? N1+1 : N1; // (ceil) horizon of large blocks
	int T1; // horizon of current block

	int N_tmp = 0; // temporary sum of horizons
	int nbb; // box constr that remain box constr
	int nbg; // box constr that becomes general constr
	for(ii=0; ii<N2; ii++)
		{
		T1 = ii<R1 ? M1 : N1;
		nx2[ii] = nx[N_tmp+0];
		nu2[ii] = nu[N_tmp+0];
		nbx2[ii] = nbx[N_tmp+0];
		nbu2[ii] = nbu[N_tmp+0];
		nb2[ii] = nb[N_tmp+0];
		ng2[ii] = ng[N_tmp+0];
		ns2[ii] = ns[N_tmp+0];
		for(jj=1; jj<T1; jj++)
			{
			nx2[ii] += 0;
			nu2[ii] += nu[N_tmp+jj];
			nbx2[ii] += 0;
			nbu2[ii] += nbu[N_tmp+jj];
			nb2[ii] += nbu[N_tmp+jj];
			ng2[ii] += ng[N_tmp+jj] + nbx[N_tmp+jj];
			ns2[ii] += ns[N_tmp+jj];
			}
		N_tmp += T1;
		}
	nx2[N2] = nx[N];
	nu2[N2] = nu[N];
	nbx2[N2] = nbx[N];
	nbu2[N2] = nbu[N];
	nb2[N2] = nb[N];
	ng2[N2] = ng[N];
	ns2[N2] = ns[N];

	return;

	}



int MEMSIZE_COND_QP_OCP2OCP(struct OCP_QP_SIZE *ocp_size, struct OCP_QP_SIZE *part_dense_size)
	{

	struct OCP_QP_SIZE tmp_ocp_size;

	int ii;

	int N = ocp_size->N;
	int N2 = part_dense_size->N;
	int N1 = N/N2; // (floor) horizon of small blocks
	int R1 = N - N2*N1; // the first R1 blocks have horizon N1+1
	int M1 = R1>0 ? N1+1 : N1; // (ceil) horizon of large blocks
	int T1; // horizon of current block

	int size = 0;

	size += N2*sizeof(struct COND_QP_OCP2DENSE_WORKSPACE);

	int N_tmp = 0; // temporary sum of horizons
	for(ii=0; ii<N2; ii++)
		{

		T1 = ii<R1 ? M1 : N1;

		// alias ocp_size
		tmp_ocp_size.N = T1;
		tmp_ocp_size.nx = ocp_size->nx+N_tmp;
		tmp_ocp_size.nu = ocp_size->nu+N_tmp;
		tmp_ocp_size.nbx = ocp_size->nbx+N_tmp;
		tmp_ocp_size.nbu = ocp_size->nbu+N_tmp;
		tmp_ocp_size.nb = ocp_size->nb+N_tmp;
		tmp_ocp_size.ng = ocp_size->ng+N_tmp;

		size += MEMSIZE_COND_QP_OCP2DENSE(&tmp_ocp_size);

		N_tmp += T1;

		}

	size = (size+63)/64*64; // make multiple of typical cache line size
	size += 1*64; // align once to typical cache line size

	return size;

	}



void CREATE_COND_QP_OCP2OCP(struct OCP_QP_SIZE *ocp_size, struct OCP_QP_SIZE *part_dense_size, struct COND_QP_OCP2OCP_WORKSPACE *cond_ws, void *mem)
	{

	struct OCP_QP_SIZE tmp_ocp_size;

	int ii;

	int N = ocp_size->N;
	int N2 = part_dense_size->N;
	int N1 = N/N2; // (floor) horizon of small blocks
	int R1 = N - N2*N1; // the first R1 blocks have horizon N1+1
	int M1 = R1>0 ? N1+1 : N1; // (ceil) horizon of large blocks
	int T1; // horizon of current block

	// cond workspace struct
	struct COND_QP_OCP2DENSE_WORKSPACE *cws_ptr = mem;
	cond_ws->cond_workspace = cws_ptr;
	cws_ptr += N2;

	// align to typicl cache line size
	size_t s_ptr = (size_t) cws_ptr;
	s_ptr = (s_ptr+63)/64*64;

	char *c_ptr = (char *) s_ptr;

	int N_tmp = 0; // temporary sum of horizons
	for(ii=0; ii<N2; ii++)
		{

		T1 = ii<R1 ? M1 : N1;

		// alias ocp_size
		tmp_ocp_size.N = T1;
		tmp_ocp_size.nx = ocp_size->nx+N_tmp;
		tmp_ocp_size.nu = ocp_size->nu+N_tmp;
		tmp_ocp_size.nbx = ocp_size->nbx+N_tmp;
		tmp_ocp_size.nbu = ocp_size->nbu+N_tmp;
		tmp_ocp_size.nb = ocp_size->nb+N_tmp;
		tmp_ocp_size.ng = ocp_size->ng+N_tmp;

		CREATE_COND_QP_OCP2DENSE(&tmp_ocp_size, cond_ws->cond_workspace+ii, c_ptr);
		c_ptr += (cond_ws->cond_workspace+ii)->memsize;
		(cond_ws->cond_workspace+ii)->cond_last_stage = 0;

		N_tmp += T1;

		}

	cond_ws->memsize = MEMSIZE_COND_QP_OCP2OCP(ocp_size, part_dense_size);

	#if defined(RUNTIME_CHECKS)
	if(c_ptr > ((char *) mem) + cond_ws->memsize)
		{
		printf("\nCreate_cond_qp_ocp2ocp: outsize memory bounds!\n\n");
		exit(1);
		}
#endif

return;

	}

	

void COND_QP_OCP2OCP(struct OCP_QP *ocp_qp, struct OCP_QP *part_dense_qp, struct COND_QP_OCP2OCP_WORKSPACE *part_cond_ws)
	{

	struct OCP_QP_SIZE tmp_ocp_size;
	struct OCP_QP tmp_ocp_qp;

	int ii;

	int N = ocp_qp->size->N;
	int N2 = part_dense_qp->size->N;
	int N1 = N/N2; // (floor) horizon of small blocks
	int R1 = N - N2*N1; // the first R1 blocks have horizon N1+1
	int M1 = R1>0 ? N1+1 : N1; // (ceil) horizon of large blocks
	int T1; // horizon of current block

	int N_tmp = 0; // temporary sum of horizons
	for(ii=0; ii<N2; ii++)
		{

		T1 = ii<R1 ? M1 : N1;

		// alias ocp_size
		tmp_ocp_size.N = T1;
		tmp_ocp_size.nx = ocp_qp->size->nx+N_tmp;
		tmp_ocp_size.nu = ocp_qp->size->nu+N_tmp;
		tmp_ocp_size.nbx = ocp_qp->size->nbx+N_tmp;
		tmp_ocp_size.nbu = ocp_qp->size->nbu+N_tmp;
		tmp_ocp_size.nb = ocp_qp->size->nb+N_tmp;
		tmp_ocp_size.ng = ocp_qp->size->ng+N_tmp;
		tmp_ocp_size.ns = ocp_qp->size->ns+N_tmp;

		// alias ocp_qp
		tmp_ocp_qp.size = &tmp_ocp_size;
		tmp_ocp_qp.idxb = ocp_qp->idxb+N_tmp;
		tmp_ocp_qp.BAbt = ocp_qp->BAbt+N_tmp;
		tmp_ocp_qp.b = ocp_qp->b+N_tmp;
		tmp_ocp_qp.RSQrq = ocp_qp->RSQrq+N_tmp;
		tmp_ocp_qp.rq = ocp_qp->rq+N_tmp;
		tmp_ocp_qp.DCt = ocp_qp->DCt+N_tmp;
		tmp_ocp_qp.d = ocp_qp->d+N_tmp;
		tmp_ocp_qp.Z = ocp_qp->Z+N_tmp;
		tmp_ocp_qp.z = ocp_qp->z+N_tmp;
		tmp_ocp_qp.idxs = ocp_qp->idxs+N_tmp;

		COND_BABT(&tmp_ocp_qp, part_dense_qp->BAbt+ii, part_dense_qp->b+ii, part_cond_ws->cond_workspace+ii);

		COND_RSQRQ_N2NX3(&tmp_ocp_qp, part_dense_qp->RSQrq+ii, part_dense_qp->rq+ii, part_cond_ws->cond_workspace+ii);

		COND_DCTD(&tmp_ocp_qp, part_dense_qp->idxb[ii], part_dense_qp->DCt+ii, part_dense_qp->d+ii, part_dense_qp->idxs[ii], part_dense_qp->Z+ii, part_dense_qp->z+ii, part_cond_ws->cond_workspace+ii);

		N_tmp += T1;

		}

	// copy last stage
	int *nx = ocp_qp->size->nx;
	int *nu = ocp_qp->size->nu;
	int *nb = ocp_qp->size->nb;
	int *ng = ocp_qp->size->ng;
	int *ns = ocp_qp->size->ns;

	GECP_LIBSTR(nu[N]+nx[N]+1, nu[N]+nx[N], ocp_qp->RSQrq+N, 0, 0, part_dense_qp->RSQrq+N2, 0, 0);
	VECCP_LIBSTR(nu[N]+nx[N], ocp_qp->rq+N, 0, part_dense_qp->rq+N2, 0);
	GECP_LIBSTR(nu[N]+nx[N], ng[N], ocp_qp->DCt+N, 0, 0, part_dense_qp->DCt+N2, 0, 0);
	VECCP_LIBSTR(2*nb[N]+2*ng[N], ocp_qp->d+N, 0, part_dense_qp->d+N2, 0);
	for(ii=0; ii<nb[N]; ii++) part_dense_qp->idxb[N2][ii] = ocp_qp->idxb[N][ii];
	VECCP_LIBSTR(2*ns[N], ocp_qp->Z+N, 0, part_dense_qp->Z+N2, 0);
	VECCP_LIBSTR(2*ns[N], ocp_qp->z+N, 0, part_dense_qp->z+N2, 0);
	for(ii=0; ii<ns[N]; ii++) part_dense_qp->idxs[N2][ii] = ocp_qp->idxs[N][ii];

	return;

	}



void COND_RHS_QP_OCP2OCP(struct OCP_QP *ocp_qp, struct OCP_QP *part_dense_qp, struct COND_QP_OCP2OCP_WORKSPACE *part_cond_ws)
	{

	struct OCP_QP_SIZE tmp_ocp_size;
	struct OCP_QP tmp_ocp_qp;

	int ii;

	int N = ocp_qp->size->N;
	int N2 = part_dense_qp->size->N;
	int N1 = N/N2; // (floor) horizon of small blocks
	int R1 = N - N2*N1; // the first R1 blocks have horizon N1+1
	int M1 = R1>0 ? N1+1 : N1; // (ceil) horizon of large blocks
	int T1; // horizon of current block

	int N_tmp = 0; // temporary sum of horizons
	for(ii=0; ii<N2; ii++)
		{

		T1 = ii<R1 ? M1 : N1;

		// alias ocp_size
		tmp_ocp_size.N = T1;
		tmp_ocp_size.nx = ocp_qp->size->nx+N_tmp;
		tmp_ocp_size.nu = ocp_qp->size->nu+N_tmp;
		tmp_ocp_size.nbx = ocp_qp->size->nbx+N_tmp;
		tmp_ocp_size.nbu = ocp_qp->size->nbu+N_tmp;
		tmp_ocp_size.nb = ocp_qp->size->nb+N_tmp;
		tmp_ocp_size.ng = ocp_qp->size->ng+N_tmp;
		tmp_ocp_size.ns = ocp_qp->size->ns+N_tmp;

		// alias ocp_qp
		tmp_ocp_qp.size = &tmp_ocp_size;
		tmp_ocp_qp.idxb = ocp_qp->idxb+N_tmp;
		tmp_ocp_qp.BAbt = ocp_qp->BAbt+N_tmp;
		tmp_ocp_qp.b = ocp_qp->b+N_tmp;
		tmp_ocp_qp.RSQrq = ocp_qp->RSQrq+N_tmp;
		tmp_ocp_qp.rq = ocp_qp->rq+N_tmp;
		tmp_ocp_qp.DCt = ocp_qp->DCt+N_tmp;
		tmp_ocp_qp.d = ocp_qp->d+N_tmp;
		tmp_ocp_qp.Z = ocp_qp->Z+N_tmp;
		tmp_ocp_qp.z = ocp_qp->z+N_tmp;
		tmp_ocp_qp.idxs = ocp_qp->idxs+N_tmp;

		COND_B(&tmp_ocp_qp, part_dense_qp->b+ii, part_cond_ws->cond_workspace+ii);

		COND_RQ_N2NX3(&tmp_ocp_qp, part_dense_qp->rq+ii, part_cond_ws->cond_workspace+ii);

		COND_D(&tmp_ocp_qp, part_dense_qp->d+ii, part_dense_qp->z+ii, part_cond_ws->cond_workspace+ii);

		N_tmp += T1;

		}

	// copy last stage
	int *nx = ocp_qp->size->nx;
	int *nu = ocp_qp->size->nu;
	int *nb = ocp_qp->size->nb;
	int *ng = ocp_qp->size->ng;
	int *ns = ocp_qp->size->ns;

	VECCP_LIBSTR(nu[N]+nx[N], ocp_qp->rq+N, 0, part_dense_qp->rq+N2, 0);
	VECCP_LIBSTR(2*nb[N]+2*ng[N], ocp_qp->d+N, 0, part_dense_qp->d+N2, 0);
	VECCP_LIBSTR(2*ns[N], ocp_qp->z+N, 0, part_dense_qp->z+N2, 0);

	return;

	}



void EXPAND_SOL_OCP2OCP(struct OCP_QP *ocp_qp, struct OCP_QP *part_dense_qp, struct OCP_QP_SOL *part_dense_qp_sol, struct OCP_QP_SOL *ocp_qp_sol, struct COND_QP_OCP2OCP_WORKSPACE *part_cond_ws)
	{

	struct OCP_QP_SIZE tmp_ocp_size;
	struct OCP_QP tmp_ocp_qp;
	struct OCP_QP_SOL tmp_ocp_qp_sol;
	struct DENSE_QP_SOL dense_qp_sol;

	int *nx = ocp_qp->size->nx;
	int *nu = ocp_qp->size->nu;
	int *nb = ocp_qp->size->nb;
	int *ng = ocp_qp->size->ng;
	int *ns = ocp_qp->size->ns;

	int ii;

	int N = ocp_qp->size->N;
	int N2 = part_dense_qp->size->N;
	int N1 = N/N2; // (floor) horizon of small blocks
	int R1 = N - N2*N1; // the first R1 blocks have horizon N1+1
	int M1 = R1>0 ? N1+1 : N1; // (ceil) horizon of large blocks
	int T1; // horizon of current block

	int N_tmp = 0; // temporary sum of horizons
	for(ii=0; ii<N2; ii++)
		{

		T1 = ii<R1 ? M1 : N1;

		// alias ocp_size
		tmp_ocp_size.N = T1;
		tmp_ocp_size.nx = ocp_qp->size->nx+N_tmp;
		tmp_ocp_size.nu = ocp_qp->size->nu+N_tmp;
		tmp_ocp_size.nbx = ocp_qp->size->nbx+N_tmp;
		tmp_ocp_size.nbu = ocp_qp->size->nbu+N_tmp;
		tmp_ocp_size.nb = ocp_qp->size->nb+N_tmp;
		tmp_ocp_size.ng = ocp_qp->size->ng+N_tmp;
		tmp_ocp_size.ns = ocp_qp->size->ns+N_tmp;

		// alias ocp_qp
		tmp_ocp_qp.size = &tmp_ocp_size;
		tmp_ocp_qp.idxb = ocp_qp->idxb+N_tmp;
		tmp_ocp_qp.BAbt = ocp_qp->BAbt+N_tmp;
		tmp_ocp_qp.b = ocp_qp->b+N_tmp;
		tmp_ocp_qp.RSQrq = ocp_qp->RSQrq+N_tmp;
		tmp_ocp_qp.rq = ocp_qp->rq+N_tmp;
		tmp_ocp_qp.DCt = ocp_qp->DCt+N_tmp;
		tmp_ocp_qp.d = ocp_qp->d+N_tmp;
		tmp_ocp_qp.Z = ocp_qp->Z+N_tmp;
		tmp_ocp_qp.z = ocp_qp->z+N_tmp;
		tmp_ocp_qp.idxs = ocp_qp->idxs+N_tmp;

		// alias ocp qp sol
		tmp_ocp_qp_sol.ux = ocp_qp_sol->ux+N_tmp;
		tmp_ocp_qp_sol.pi = ocp_qp_sol->pi+N_tmp;
		tmp_ocp_qp_sol.lam = ocp_qp_sol->lam+N_tmp;
		tmp_ocp_qp_sol.t = ocp_qp_sol->t+N_tmp;

		// alias ocp qp sol
		dense_qp_sol.v = part_dense_qp_sol->ux+ii;
		dense_qp_sol.pi = part_dense_qp_sol->pi+ii;
		dense_qp_sol.lam = part_dense_qp_sol->lam+ii;
		dense_qp_sol.t = part_dense_qp_sol->t+ii;

		EXPAND_SOL(&tmp_ocp_qp, &dense_qp_sol, &tmp_ocp_qp_sol, part_cond_ws->cond_workspace);

		N_tmp += T1;

		}

	// copy last stage
	VECCP_LIBSTR(nu[N]+nx[N]+2*ns[N], part_dense_qp_sol->ux+N2, 0, ocp_qp_sol->ux+N, 0);
	VECCP_LIBSTR(2*nb[N]+2*ng[N]+2*ns[N], part_dense_qp_sol->lam+N2, 0, ocp_qp_sol->lam+N, 0);
	VECCP_LIBSTR(2*nb[N]+2*ng[N]+2*ns[N], part_dense_qp_sol->t+N2, 0, ocp_qp_sol->t+N, 0);

	return;

	}
