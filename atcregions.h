///////////////////////////////////////////////////////////////////////
// Определяется область счетчиков в локальных координатах,
// по которой смотрится число фотоэлектронов
// длины в мм, углы в радианах
///////////////////////////////////////////////////////////////////////

#include "KrAtc/atc_geometry.h"
#include <math.h>

struct ATCRange {
// координаты вдоль счетчика: z - в барельном, r - в торцевом
	float lmin, lmax;
// азимутальные углы одной из половинок счетчика (вторая - зеркальная)
	float phiMin, phiMax;
};
                                                      //отступы
// Барельные счетчики
static const double zInd=20;
static const double phiBind=10./RinBarrel1;

// sbRange составляет 43% от площади счетчика
// sbTestRange составляет 7% от площади счетчика
// lbRange составляет 56% от площади счетчика
// lbTestRange составляет 6% от площади счетчика
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

// Торцевые счетчики
// eRange составляет 42% от площади счетчика
// eTestRange составляет 9% от площади счетчика
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

