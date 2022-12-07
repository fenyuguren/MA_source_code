#include "stdafx.h"
#include "Problem.h"
#include "Heu.h"
#include "IGPan.h"
#pragma comment(lib,"ws2_32.lib") // ¾²Ì¬¿â
int main(int argc, char **argv)
{

	IGPan igpan;
	igpan.RunEvolution_OneConf(1, 5);
	//cout << "hexuan" << endl;
	
	return 0;
}
