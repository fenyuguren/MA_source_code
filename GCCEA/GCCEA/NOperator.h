#pragma once
#include "Problem.h"
class NOperator :
	virtual public Problem
{
public:
	void CheckSol(vector <int> FamSeq, vector <vector <int>> JobSeqinFam, int Span);
	int GetSol(vector<int> FamPrmu, vector<vector<int>>JobSeqinFam, vector<vector<int>> &FacFamSeq, vector<int> &FacSpan);
	int GetSol_Include(vector<int> FamPrmu, vector<vector<int>>JobSeqinFam, vector<vector<int>> &FacFamSeq, vector<int> &FacSpan);
	void GetRPI(vector<vector<int>> SpanArray, vector<vector<double>> &RPIArray);
	int GetSpan(vector <int> FamSeq, vector <vector <int>> JobSeqinFam);
	int GetSpan(vector <int> FamSeq, vector <vector <int>> JobSeqinFam, int &FacSpan);

	int GetJCTime_Forward_InFactory(vector <int> FamSeqInFac, vector <vector <int>> JobSeqinFam, vector<vector<int>> &JCTime);
	int GetJSTime_Backward_InFactory(vector <int> FamSeqInFac, vector <vector <int>> JobSeqinFam, vector<vector<int>> &JSTime);
	int GetJCTime_Forward(vector <int> FamSeq, vector <vector <int>> JobSeqinFam, vector<vector<int>> &JCTime, int &FacSpan);

	int FindBestPosToInsertFam_InFactory(vector <int> FamSeqInFac, vector <vector <int>> JobSeqinFam, vector<vector<int>> JSTime, vector<vector<int>> JCTime, int InsFam, int &bestPos);
	int FindBestPosToInsertFam(vector <vector <int>> FamSeq, vector <vector <int>> JobSeqinFam, vector<vector<int>> JSTime, vector<vector<int>> JCTime, int InsFam, int &bestFac, int &bestPos);
	void FamInsert(vector <int> &FamSeq, vector <vector <int>> JobSeqinFam, int &Span);
	void FamSwap(vector <int> &FamSeq, vector <vector <int>> JobSeqinFam, int &Span);
	void JobSwap(vector <int>FamSeq, vector <vector <int>> &JobSeqinFam, int &Span);
	void JobInsert(vector <int>FamSeq, vector <vector <int>> &JobSeqinFam, int &Span);
	void SwapJob(vector<vector<int>> &JobSeqinFam);
	void SwapFam(vector <int>&FamSeq);
	void SwapFaminFac(vector <int> &FamSeqInFac);
	NOperator();
	virtual ~NOperator();
};

