#pragma once
#include "NOperator.h"
class Heu :
	virtual public NOperator
{
public:
	void JPA(vector<int>&InitialFamSeq, vector<vector<int>>&InitialJobinFamSeq, int &Cmax);
	void GetCostMatrix(vector<vector<int>>&CostMatrix, vector<int>PartialFamSeq, int head, int tail);
	void setdiff(vector<int> Originseq, vector<int> Removeseq, vector<int> &Restseq);
	void LRX(vector<vector<int>> FamPop, vector<vector<vector<int>>> &JobSeqinFamPop);
	void schedulejob(vector<int>&jobseq, vector<int> &MachReadyTime);
	void WriteFile(string FileName, vector<unsigned int> FactorLevelsArray, vector<vector<double>> RPIArray);
public:
	Heu();
	virtual ~Heu();
protected:
	void SortJobsinFam(int Factor, int SortMethod, vector<vector<int>> &JobSeqinFam); //Method=0:LPT; Method=1:SPT
	void SortFam(int Factor, int SortMethod, vector<int> &FamPrmu);	
	int NEHFam(vector<int> FamPrmuInFac, vector<vector<int>> JobSeqinFam, vector<int> &FamSeqInFac, int &SpanInFac);
	int NEHFamwithTiebreak(vector<int> FamPrmuInFac, vector<vector<int>> JobSeqinFam, vector<int> &FamSeqInFac, int &SpanInFac);
};

