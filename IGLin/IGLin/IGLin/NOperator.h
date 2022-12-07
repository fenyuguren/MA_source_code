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
	int GetSpanInFac(vector <int> FamSeqInFac, vector <vector <int>> JobSeqinFam);
	int GetJCTime_Forward_InFactory(vector <int> FamSeqInFac, vector <vector <int>> JobSeqinFam, vector<vector<int>> &JCTime);
	void GetJCTime_Forward_InJobSeq(vector<int> JobSeq, vector<int> MachReadyTime, vector<vector<int>> &JCTime);
	int GetJCTime_Forward(vector <int> FamSeq, vector <vector <int>> JobSeqinFam, vector<vector<int>> &JCTime, int &FacSpan);
	int GetJSTime_Backward_InFactory(vector <int> FamSeqInFac, vector <vector <int>> JobSeqinFam, vector<vector<int>> &JSTime);
	void GetJSTime_Backward_InJobSeq(vector<int> JobSeq, vector<int> MachReadyTimeR, vector<vector<int>> &JSTime);
	int GetJSTime_Backward(vector <vector <int>> FamSeq, vector <vector <int>> JobSeqinFam, vector<vector<int>> &JSTime, vector<int> &FacSpan);
	int FindBestPosToInsertJob(vector <int> JobSeq, vector<vector<int>> JSTime, vector<vector<int>> JCTime, vector<int> MachReadyTime, vector<int> MachReadyTimeR, int InsJob, int &bestPos);
	int FindBestPosToInsertFam(vector <vector <int>> FamSeq, vector <vector <int>> JobSeqinFam, vector<vector<int>> JSTime, vector<vector<int>> JCTime, int InsFam, int &bestFac, int &bestPos);
	int FindBestPosToInsertFam_InFactory(vector <int> FamSeqInFac, vector <vector <int>> JobSeqinFam, vector<vector<int>> JSTime, vector<vector<int>> JCTime, int InsFam, int &bestPos);
	int FindBestPosToInsertFam_InFactory_OptJobSeq(vector <int> FamSeqInFac, vector <vector <int>> JobSeqinFam, vector<vector<int>> JSTime, vector<vector<int>> JCTime, int InsFam, int &bestPos, vector<int> &bestJobSeq);
	void LocalSearch_JobsInFam(vector <int> &JobSeq, int &Span, vector<int> MachReadyTime, vector<int> MachReadyTimeR);
	void LocalSearch_JobsInFac(vector <int> FamSeqInFac, vector <vector <int>> &JobSeqinFam, int &Span);
	int LocalSearch_Jobs(vector<vector <int>> FamSeqInFac, vector <vector <int>> &JobSeqinFam, vector<int> &FacSpan);
	void LocalSearch_FamsInFac(vector <int> &FamSeqInFac, vector <vector <int>> JobSeqinFam, int &Span);
	void LocalSearch_FamsInFac_OptJobSeq(vector <int> &FamSeqInFac, vector <vector <int>> &JobSeqinFam, int &Span);
	int LocalSearch_Fams(vector<vector <int>> &FamSeq, vector<vector <int>> RefFamSeq, vector <vector <int>> JobSeqinFam, vector<int> &FacSpan, int &k);
	void SwapFaminFac(vector <int> &FamSeqInFac);
	void SwapFamBetweenFacs(vector<vector <int>> &FamSeq);
	void SwapFam(vector<vector <int>> &FamSeq);
	void ShiftFam(vector<vector <int>> &FamSeq);
	void SwapJob(vector<vector<int>> &JobSeqinFam);
	void ShiftJob(vector<vector<int>> &JobSeqinFam);
	void FamInsert(vector <int> &FamSeq, vector <vector <int>> JobSeqinFam, int &Span);
	NOperator();
	virtual ~NOperator();
};

