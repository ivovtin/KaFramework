///////////////////////////////////////////////////////////////////////
// ������������ ������� ��������� � ��������� �����������,
// �� ������� ��������� ����� ��������������
// ����� � ��, ���� � ��������
///////////////////////////////////////////////////////////////////////

#include "KrAtc/atc_geometry.h"
#include <math.h>

struct ATCRange {
// ���������� ����� ��������: z - � ���������, r - � ��������
	float lmin, lmax;
// ������������ ���� ����� �� ��������� �������� (������ - ����������)
	float phiMin, phiMax;
};
                                                      //�������
// ��������� ��������
static const double zInd=20;
static const double phiBind=10./RinBarrel1;

// sbRange ���������� 43% �� ������� ��������
// sbTestRange ���������� 7% �� ������� ��������
// lbRange ���������� 56% �� ������� ��������
// lbTestRange ���������� 6% �� ������� ��������
static const ATCRange sbRange = {
	-LshortBarrel/2+65+zInd, LshortBarrel/2-96-zInd,
//	0, LshortBarrel/2-96-zInd,
	phiBind, HalfPhiSizeBarrel1-phiBind };

static const ATCRange lbRange = {
	-LlongBarrel/2+15+zInd, LlongBarrel/2-96-zInd,
//	0, LlongBarrel/2-96-zInd,
	phiBind, HalfPhiSizeBarrel1-phiBind };

static const ATCRange sbTestRange = {
	-LshortBarrel/2+156, -LshortBarrel/2+256,
	19./MeanRbarrel1, 50./MeanRbarrel1 };

static const ATCRange lbTestRange = {
	-LlongBarrel/2+156, -LlongBarrel/2+256,
	19./MeanRbarrel1, 50./MeanRbarrel1 };

// �������� ��������
// eRange ���������� 42% �� ������� ��������
// eTestRange ���������� 9% �� ������� ��������
static const double rInd=5, phiEind=10./RmaxEndcap;
static const ATCRange eRange = { 350, 640, .02+phiEind, .1457-phiEind };
static const ATCRange eTestRange = { RmaxEndcap-312, RmaxEndcap-212,
19./(RmaxEndcap-260), 50./(RmaxEndcap-260) };

static bool match_range(float lin,float phiin,float lout,float phiout,const ATCRange &range)
{
	if( lin>range.lmin && lin<range.lmax && lout>range.lmin && lout<range.lmax &&
		(phiin>range.phiMin && phiin<range.phiMax && phiout>range.phiMin && phiout<range.phiMax ||
		 phiin>-range.phiMax && phiin<-range.phiMin && phiout>-range.phiMax && phiout<-range.phiMin ) )
		return true;
	return false;
}

