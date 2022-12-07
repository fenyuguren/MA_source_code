#pragma once
#include "Heu.h"
class CCEA :
	public Heu
{
public:
	CCEA();
	virtual ~CCEA();	
	void RunEvolution2_OneConf(int CPUFactor, int Rep);
	
private:
	vector<vector<int>> m_RefFamSeqArray;
	vector<vector<vector<int>>> m_RefJobSeqinFamArray;	
	vector<int> m_RefSpanArray;	
	int m_RefSize;
	
	vector<vector<int>> m_FamSeqArray;
	vector<vector<vector<int>>> m_JobSeqinFamArray;
	vector<int> m_SpanArray1;
	vector<int> m_SpanArray2;
	
	int m_DesLen;
	int m_PS1;
	int m_PS2;
	vector<int> m_Map1;
	vector<int> m_Map2;
	vector<bool> m_bFlag1;// for the reference familiy sequence updated
	vector<bool> m_bFlag2;// for the reference job sequence updated
	vector<int> m_Age1;
	vector<int> m_Age2;
	int m_AgeLimit;
	long m_InitTime;
	long m_TimeLimit;
	vector<int> m_RecordSpan;
	
	

private:
	void InitPop();
	void ReInitPop();	
	void SetParameters(int RefSize, int PS1, int PS2, int AgeLimit,int DesLen,long TimeLimit);
	int Destruction_Construction(int Len, vector<int> &FamSeq, vector<vector<int>> JobSeqinFam, int &Span);
	
	void Evolution2();// the propose CCEA	
};

