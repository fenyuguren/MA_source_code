// FlowshopgroupScheduling.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Problem.h"
#include "Heu.h"
#include "CCABC.h"

#pragma comment(lib,"ws2_32.lib") // 静态库



int main(int argc,char **argv)
{
	
	/*IGPan igpan;
	igpan.RunEvolution_OneConf(300, 5);*/
//	igpan.Sta_CalibrationTwoStage();
//	WriteRunConfs("RunConfs.txt");
//	::FindLostConfs("");
//	RunEvolution();
	CCABC ccabc;
	ccabc.RunEvolution_OneConf(5, 5);
//	string Dir = "..//..//manuscript_CCEA//Revise//Span100_200_300//";
//	GetbestSpan(Dir);
//	CompAlgs(Dir, 0);
//	CompAlgs(Dir, 1);
//	CompAlgs(Dir, 2);
//	CCEAnM cceanm;
	//cceanm.Sta_Calibration();
//	CCEAB cceab;
//	cceab.Calibration_OneConf(0);
    return 0;
}

