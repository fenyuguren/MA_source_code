#pragma once
#include "NOperator.h"
class Heu :
	virtual public NOperator
{
public:
	
	void WriteFile(string FileName, vector<unsigned int> FactorLevelsArray, vector<vector<double>> RPIArray);
public:
	Heu();
	virtual ~Heu();
protected:
	void SortJobsinFam(int Factor, int SortMethod,vector<vector<int>> &JobSeqinFam); //Method=0:LPT; Method=1:SPT
	void SortFam(int Factor, int SortMethod, vector<int> &FamPrmu);
	int NEHFam(vector<int> FamPrmu, vector<vector<int>> JobSeqinFam, vector<vector<int>> &FamSeq, vector<int> &FacSpan);
	int NEHFam(vector<int> FamPrmuInFac, vector<vector<int>> JobSeqinFam, vector<int> &FamSeqInFac, int &SpanInFac);
	
};

