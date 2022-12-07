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
	void JobTieInsert(vector <int>FamSeq, vector <vector <int>> &JobSeqinFam, int &Span);
	int Tiebreaking(vector<vector<int>>fits);
	vector<int> LocalSearch(vector<vector<int>>D);

	void LocalSearch_JobsInFac(vector <int> FamSeqInFac, vector <vector <int>> &JobSeqinFam, int &Span);
	void LocalSearch_JobsInFam(vector <int> &JobSeq, int &Span, vector<int> MachReadyTime, vector<int> MachReadyTimeR);
	int FindBestPosToInsertJob(vector <int> JobSeq, vector<vector<int>> JSTime, vector<vector<int>> JCTime, vector<int> MachReadyTime, vector<int> MachReadyTimeR, int InsJob, int &bestPos);
	void GetJCTime_Forward_InJobSeq(vector<int> JobSeq, vector<int> MachReadyTime, vector<vector<int>> &JCTime);
	void GetJSTime_Backward_InJobSeq(vector<int> JobSeq, vector<int> MachReadyTimeR, vector<vector<int>> &JSTime);
	
	NOperator();
	virtual ~NOperator();
};

