/*---------------------------------------------------------------------------*\
	OneFLOW - LargeScale Multiphysics Scientific Simulation Environment
	Copyright (C) 2017-2019 He Xin and the OneFLOW contributors.
-------------------------------------------------------------------------------
License
	This file is part of OneFLOW.

	OneFLOW is free software: you can redistribute it and/or modify it
	under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	OneFLOW is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
	FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
	for more details.

	You should have received a copy of the GNU General Public License
	along with OneFLOW.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

//#include "UINsCorrectSpeed.h"
#include "INsInvterm.h"
#include "INsVisterm.h"
#include "Iteration.h"
#include "UINsCom.h"
#include "Zone.h"
#include "DataBase.h"
#include "UCom.h"
#include "UGrad.h"
#include "Com.h"
#include "INsCom.h"
#include "INsIDX.h"
#include "HXMath.h"
#include "Ctrl.h"
#include "Boundary.h"
#include "BcRecord.h"

BeginNameSpace(ONEFLOW)

INsInv iinv;



INsInv::INsInv()
{
	;
}

INsInv::~INsInv()
{
	;
}

void INsInv::Init()
{
	int nEqu = inscom.nEqu;
	//prim.resize(nEqu);
	//prim1.resize(nEqu);
	//prim2.resize(nEqu);

	q.resize(nEqu);
	//q1.resize(nEqu);
	//q2.resize(nEqu);

	dq.resize(nEqu);

	flux.resize(nEqu);
	//flux1.resize(nEqu);
	//flux2.resize(nEqu);

}

INsInvterm::INsInvterm()
{
	;
}

INsInvterm::~INsInvterm()
{
	;
}

void INsInvterm::Solve()
{
}

void INsInvterm::CmpINsinvFlux()
{

	INsExtractl(*uinsf.q, iinv.rl, iinv.ul, iinv.vl, iinv.wl, iinv.pl);

	INsExtractr(*uinsf.q, iinv.rr, iinv.ur, iinv.vr, iinv.wr, iinv.pr);

	iinv.rf[ug.fId] = (iinv.rl + iinv.rr) * half;    //��ʼ�����ϵ�ֵ��u��v��w ��

	iinv.uf[ug.fId] = (iinv.ul + iinv.ur) * half;

	iinv.vf[ug.fId] = (iinv.vl + iinv.vr) * half;

	iinv.wf[ug.fId] = (iinv.wl + iinv.wr) * half;

	iinv.pf[ug.fId] = (iinv.pl + iinv.pr) * half;

	iinv.vnflow = gcom.xfn * iinv.uf[ug.fId] + gcom.yfn * iinv.vf[ug.fId] + gcom.zfn * iinv.wf[ug.fId] - gcom.vfn;  //��ʼ������ V*n

	iinv.fq[ug.fId] = iinv.rf[ug.fId] * iinv.vnflow * gcom.farea; //��ʼ�����ϵ�����ͨ��

}

void INsInvterm::CmpINsBcinvFlux()
{
		iinv.fq[ug.fId] = iinv.rf[ug.fId] * ((*ug.a1)[ug.fId] * iinv.uf[ug.fId] + (*ug.a2)[ug.fId] * iinv.vf[ug.fId] + (*ug.a3)[ug.fId] * iinv.wf[ug.fId] - gcom.vfn);
}

void INsInvterm::CmpINsinvTerm()
{
	Real clr = MAX(0, iinv.fq[ug.fId]);  //�ӽ�����൥Ԫ�����Ҳ൥Ԫ����������

	Real crl = clr - iinv.fq[ug.fId];   //�ӽ����Ҳ൥Ԫ������൥Ԫ����������

	ug.lc = (*ug.lcf)[ug.fId];
	ug.rc = (*ug.rcf)[ug.fId];
	iinv.ai[ug.fId][0] += crl;
	iinv.ai[ug.fId][1] += clr;

	//diagonal coefficient
	iinv.spc[ug.lc] += clr;
	iinv.spc[ug.rc] += crl;

	iinv.buc[ug.lc] = 0;
	iinv.buc[ug.rc] = 0;

	iinv.bvc[ug.lc] = 0;
	iinv.bvc[ug.rc] = 0;

	iinv.bwc[ug.lc] = 0;
	iinv.bwc[ug.rc] = 0;
}

void INsInvterm::CmpINsBcinvTerm()
{

	Real clr = MAX(0, iinv.fq[ug.fId]);  //�ӽ�����൥Ԫ�����Ҳ൥Ԫ�ĳ�ʼ��������
	Real crl = clr - iinv.fq[ug.fId];   //�ӽ����Ҳ൥Ԫ������൥Ԫ�ĳ�ʼ��������


	//iinv.ai[ug.fId][0] = clr;           //�߽�����Ӧ��ֻ�����Խ���ϵ������Ӱ��
	//iinv.ai[ug.fId][1] = 0;

	iinv.spc[ug.lc] += clr;

	iinv.buc[ug.lc] = crl * iinv.uf[ug.fId];
	iinv.buc[ug.rc] = 0;

	iinv.bvc[ug.lc] = crl * iinv.vf[ug.fId];
	iinv.bvc[ug.rc] = 0;

	iinv.bwc[ug.lc] = crl * iinv.wf[ug.fId];
	iinv.bwc[ug.rc] = 0;
}

void INsInvterm::CmpINsFaceflux()
{
	INsExtractl(*uinsf.q, iinv.rl, iinv.ul, iinv.vl, iinv.wl, iinv.pl);
	INsExtractr(*uinsf.q, iinv.rr, iinv.ur, iinv.vr, iinv.wr, iinv.pr);

	// Rie-Chow��ֵ
	Real l2rdx = (*ug.xcc)[ug.rc] - (*ug.xcc)[ug.lc];
	Real l2rdy = (*ug.ycc)[ug.rc] - (*ug.ycc)[ug.lc];
	Real l2rdz = (*ug.zcc)[ug.rc] - (*ug.zcc)[ug.lc];

	iinv.VdU[ug.lc] = (*ug.cvol1)[ug.lc] / iinv.spc[ug.lc];
	iinv.VdU[ug.rc] = (*ug.cvol1)[ug.rc] / iinv.spc[ug.rc];
	iinv.VdV[ug.lc] = (*ug.cvol1)[ug.lc] / iinv.spc[ug.lc];
	iinv.VdV[ug.rc] = (*ug.cvol1)[ug.rc] / iinv.spc[ug.rc];
	iinv.VdW[ug.lc] = (*ug.cvol1)[ug.lc] / iinv.spc[ug.lc];
	iinv.VdW[ug.rc] = (*ug.cvol1)[ug.rc] / iinv.spc[ug.rc];

	iinv.Vdvu[ug.fId] = (*ug.fl)[ug.fId] * iinv.VdU[ug.lc] + (*ug.fr)[ug.fId] * iinv.VdU[ug.rc];
	iinv.Vdvv[ug.fId] = (*ug.fl)[ug.fId] * iinv.VdV[ug.lc] + (*ug.fr)[ug.fId] * iinv.VdV[ug.rc];
    iinv.Vdvw[ug.fId] = (*ug.fl)[ug.fId] * iinv.VdW[ug.lc] + (*ug.fr)[ug.fId] * iinv.VdW[ug.rc];

	Real dist = (*ug.a1)[ug.fId] * l2rdx + (*ug.a2)[ug.fId] * l2rdy + (*ug.a3)[ug.fId] * l2rdz;

	Real Df1 = iinv.Vdvu[ug.fId] * (*ug.a1)[ug.fId] / dist;
	Real Df2 = iinv.Vdvv[ug.fId] * (*ug.a2)[ug.fId] / dist;
	Real Df3 = iinv.Vdvw[ug.fId] * (*ug.a3)[ug.fId] / dist;

	Real dx1 = (*ug.xfc)[ug.fId] - (*ug.xcc)[ug.lc];
	Real dy1 = (*ug.yfc)[ug.fId] - (*ug.ycc)[ug.lc];
	Real dz1 = (*ug.zfc)[ug.fId] - (*ug.zcc)[ug.lc];

	Real dx2 = (*ug.xcc)[ug.rc] - (*ug.xfc)[ug.fId];
	Real dy2 = (*ug.ycc)[ug.rc] - (*ug.yfc)[ug.fId];
	Real dz2 = (*ug.zcc)[ug.rc] - (*ug.zfc)[ug.fId];

	Real fdpdx = iinv.dpdx[ug.lc] * dx1 + iinv.dpdx[ug.rc] * dx2 - (iinv.pr - iinv.pl);
	Real fdpdy = iinv.dpdy[ug.lc] * dy1 + iinv.dpdy[ug.rc] * dy2 - (iinv.pr - iinv.pl);
	Real fdpdz = iinv.dpdz[ug.lc] * dz1 + iinv.dpdz[ug.rc] * dz2 - (iinv.pr - iinv.pl);

	iinv.uf[ug.fId] = iinv.ul * (*ug.fl)[ug.fId] + iinv.ur * (*ug.fr)[ug.fId];
	iinv.vf[ug.fId] = iinv.vl * (*ug.fl)[ug.fId] + iinv.vr * (*ug.fr)[ug.fId];
	iinv.wf[ug.fId] = iinv.wl * (*ug.fl)[ug.fId] + iinv.wr * (*ug.fr)[ug.fId];
	
	iinv.rf[ug.fId] = half * (iinv.rl + iinv.rr);
	iinv.uf[ug.fId] += fdpdx * Df1; 
	iinv.vf[ug.fId] += fdpdy * Df2;
	iinv.wf[ug.fId] += fdpdz * Df3;

	iinv.vnflow = (*ug.a1)[ug.fId] * iinv.uf[ug.fId] + (*ug.a2)[ug.fId] * iinv.vf[ug.fId] + (*ug.a3)[ug.fId] * iinv.wf[ug.fId] -(*ug.vfn)[ug.fId] - iinv.dun[ug.fId];
	iinv.fq[ug.fId] = iinv.rf[ug.fId] * iinv.vnflow;  

}


void INsInvterm::CmpINsBcFaceflux()
{
	INsExtractl(*uinsf.q, iinv.rl, iinv.ul, iinv.vl, iinv.wl, iinv.pl);

	if (ug.bctype == BC::SOLID_SURFACE)
	{
		if (inscom.bcdtkey == 0)
		{
			iinv.rf[ug.fId] = iinv.rl;    

			iinv.uf[ug.fId] = (*ug.vfx)[ug.fId];

			iinv.vf[ug.fId] = (*ug.vfy)[ug.fId];

			iinv.wf[ug.fId] = (*ug.vfz)[ug.fId];

			iinv.vnflow = (*ug.a1)[ug.fId] * iinv.uf[ug.fId] + (*ug.a2)[ug.fId] * iinv.vf[ug.fId] + (*ug.a3)[ug.fId] * iinv.wf[ug.fId] -(*ug.vfn)[ug.fId] - iinv.dun[ug.fId];

			iinv.fq[ug.fId] = iinv.rf[ug.fId] * iinv.vnflow;
		}
		else
		{
			iinv.rf[ug.fId] = iinv.rl;    

			iinv.uf[ug.fId] = (*inscom.bcflow)[IIDX::IIU];

			iinv.vf[ug.fId] = (*inscom.bcflow)[IIDX::IIV];

			iinv.wf[ug.fId] = (*inscom.bcflow)[IIDX::IIW];

			iinv.vnflow = (*ug.a1)[ug.fId] * iinv.uf[ug.fId] + (*ug.a2)[ug.fId] * iinv.vf[ug.fId] + (*ug.a3)[ug.fId] * iinv.wf[ug.fId] -(*ug.vfn)[ug.fId] - iinv.dun[ug.fId];

			iinv.fq[ug.fId] = iinv.rf[ug.fId] * iinv.vnflow;
		}

	}

	else if (ug.bctype == BC::INFLOW)
	{
		iinv.rf[ug.fId] = inscom.inflow[IIDX::IIR];    

		iinv.uf[ug.fId] = inscom.inflow[IIDX::IIU];

		iinv.vf[ug.fId] = inscom.inflow[IIDX::IIV];

		iinv.wf[ug.fId] = inscom.inflow[IIDX::IIW];

		iinv.vnflow = (*ug.a1)[ug.fId] * iinv.uf[ug.fId] + (*ug.a2)[ug.fId] * iinv.vf[ug.fId] + (*ug.a3)[ug.fId] * iinv.wf[ug.fId] -(*ug.vfn)[ug.fId] - iinv.dun[ug.fId];

		iinv.fq[ug.fId] = iinv.rf[ug.fId] * iinv.vnflow;
	}

	else if (ug.bctype == BC::OUTFLOW)
	{

		iinv.rf[ug.fId] = iinv.rl;    

		//iinv.uf[ug.fId] = iinv.ul + iinv.Deun * iinv.Bpe;

		//iinv.vf[ug.fId] = iinv.vl + iinv.Devn * iinv.Bpe;

		//iinv.wf[ug.fId] = iinv.wl + iinv.Dewn * iinv.Bpe;
		
		iinv.uf[ug.cId] = iinv.ul;

		iinv.vf[ug.cId] = iinv.vl;

		iinv.wf[ug.cId] = iinv.wl;

		iinv.vnflow = (*ug.a1)[ug.fId] * iinv.uf[ug.fId] + (*ug.a2)[ug.fId] * iinv.vf[ug.fId] + (*ug.a3)[ug.fId] * iinv.wf[ug.fId] -(*ug.vfn)[ug.fId] - iinv.dun[ug.fId];

		iinv.fq[ug.fId] = iinv.rf[ug.fId] * iinv.vnflow;
	}

}

void INsInvterm::CmpINsFaceCorrectPresscoef()
{

	Real Sf1 = iinv.Vdvu[ug.fId] * (*ug.a1)[ug.fId];
	Real Sf2 = iinv.Vdvv[ug.fId] * (*ug.a2)[ug.fId];
	Real Sf3 = iinv.Vdvw[ug.fId] * (*ug.a3)[ug.fId];
	
	Real r2ldx = (*ug.xcc)[ug.rc] - (*ug.xcc)[ug.lc];
	Real r2ldy = (*ug.ycc)[ug.rc] - (*ug.ycc)[ug.lc];
	Real r2ldz = (*ug.zcc)[ug.rc] - (*ug.zcc)[ug.lc];

	Real dist = r2ldx * (*ug.a1)[ug.fId] + r2ldy * (*ug.a2)[ug.fId] + r2ldz * (*ug.a3)[ug.fId];

	Real Sfarea = Sf1 * (*ug.a1)[ug.fId] + Sf2 * (*ug.a2)[ug.fId] + Sf3 * (*ug.a3)[ug.fId];

	iinv.spp[ug.lc] += iinv.rf[ug.fId] * Sfarea / dist;
	iinv.spp[ug.rc] += iinv.rf[ug.fId] * Sfarea / dist;

	iinv.ajp[ug.fId] += iinv.rf[ug.fId] * Sfarea / dist;

	iinv.bp[ug.lc] -= iinv.fq[ug.fId];
	iinv.bp[ug.rc] += iinv.fq[ug.fId];

}

void INsInvterm::CmpINsBcFaceCorrectPresscoef()
{
	int bcType = ug.bcRecord->bcType[ug.fId];

	Real Sf1 = iinv.VdU[ug.lc] * (*ug.a1)[ug.fId];
	Real Sf2 = iinv.VdV[ug.lc] * (*ug.a2)[ug.fId];
	Real Sf3 = iinv.VdW[ug.lc] * (*ug.a3)[ug.fId];

	Real r2ldx = (*ug.xfc)[ug.fId] - (*ug.xcc)[ug.lc];
	Real r2ldy = (*ug.yfc)[ug.fId] - (*ug.ycc)[ug.lc];
	Real r2ldz = (*ug.zfc)[ug.fId] - (*ug.zcc)[ug.lc];

	Real dist = r2ldx * (*ug.a1)[ug.fId] + r2ldy * (*ug.a2)[ug.fId] + r2ldz * (*ug.a3)[ug.fId];

	Real Sfarea = Sf1 * (*ug.a1)[ug.fId] + Sf2 * (*ug.a2)[ug.fId] + Sf3 * (*ug.a3)[ug.fId];

	iinv.spp[ug.lc] += iinv.rf[ug.fId] * Sfarea / dist;
	iinv.ajp[ug.fId] = 0;

	iinv.bp[ug.lc] += iinv.fq[ug.fId];
}

















EndNameSpace