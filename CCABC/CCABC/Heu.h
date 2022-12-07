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
	int NEHJob(vector<int> JobSeq, vector<int> MachReadyTime, vector<int> &NewJobSeq);
	int AppendFam_NEHJob(vector<int> FamPrmu, vector<vector<int>>&JobSeqinFam, vector<vector<int>> &FacFamSeq, vector<int> &FacSpan);
	int AppendFam_NEHJob_Include(vector<int> FamPrmu, vector<vector<int>>&JobSeqinFam, vector<vector<int>> &FacFamSeq, vector<int> &FacSpan);
	int SimpleHeu_NEH(int HeuType, vector<vector<int>> &JobSeqinFam, vector<vector<int>> &FacFamSeq, vector<int> &FacSpan);
};

