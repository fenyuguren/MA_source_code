#pragma once
#include "Heu.h"
class CCABC :
	public Heu
{
public:
	CCABC();
	virtual ~CCABC();	
	void RunEvolution_OneConf(int CPUFactor, int Rep);	
private:
	void SetParameters(int PS, int Theita, long TimeLimit);
	void InitPop();
	void Evolution();
private:
	vector<vector<int>> m_FamSeqArray;
	vector<vector<vector<int>>> m_JobSeqinFamArray;
	vector<int> m_SpanArray1;
	vector<int> m_SpanArray2;
	vector<int> m_OperType1;
	vector<int> m_OperType2;
	vector<int> m_BaseFamSeq;
	vector<vector<int>> m_BaseJobSeqinFam;
	int m_BaseSpan;
	int m_PSize;
	int m_Theita;
	long m_TimeLimit;
	long m_InitTime;	
};

