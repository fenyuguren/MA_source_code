// FlowshopgroupScheduling.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Problem.h"
#include "Heu.h"
#include "CCEA.h"

#pragma comment(lib,"ws2_32.lib") // 静态库


int main(int argc,char **argv){
	
	CCEA ccea;
	ccea.RunEvolution2_OneConf(5,  5);
    return 0;
}

