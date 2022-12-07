#pragma once
#include "Heu.h"
class TLIG ://EWSA2019
	public Heu
{
public:
	TLIG();
	virtual ~TLIG();	
	void RunEvolution_OneConf(int CPUFactor, int Rep);
private:
	void SetParameters(int d, double TempFactor, long TimeLimit);
	void Destruction(vector<int> &FacFamSeq, vector<vector<int>> JobSeqinFam, int &FacSpan, int d, vector<int> &FamsExtracted);
	void MyDestruction(vector<int> &FacFamSeq, vector<vector<int>> &JobSeqinFam,  vector<int> &FamsExtracted, vector<vector<int>> &JobSeqinFamExtracted);
	int  MyConstruction(vector<int> &FacFamSeq, vector<vector<int>> &JobSeqinFam, vector<int> FamsExtracted, vector<vector<int>> JobSeqinFamExtracted);
	int Construction(vector<int> &FacFamSeq, vector<vector<int>> JobSeqinFam, vector<int> FamsExtracted, int &FacSpan);
	double GetTemperature();	
	void Evolution();
	int m_d;
	double m_TempFactor;
	long m_TimeLimit;
	long m_InitTime;
	vector<int> m_RecordSpan;
	vector<int> m_bestFacSpan;
	double m_Ro;//For two-stage IG
	int m_d2;
};


