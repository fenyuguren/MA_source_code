#include "stdafx.h"
#include "Problem.h"
#include "Heu.h"
#include "TLIG.h"
#pragma comment(lib,"ws2_32.lib") // ¾²Ì¬¿â
int main(int argc, char **argv)
{

	TLIG tlig;
	tlig.RunEvolution_OneConf(1, 5);
	//cout << "hexuan" << endl;
	
	return 0;
}
