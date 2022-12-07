#pragma once
#include "Heu.h"
class IGLin :
	public Heu
{
public:
	IGLin();
	virtual ~IGLin();	
	void RunEvolution_OneConf(int CPUFactor, int Rep);	
private:
	void SetParameters(double T0, double Lamda, double Alpha,int Iter, int dmin, int dmax, long TimeLimit);
	void Destruction(vector<int> &FamSeq, vector<vector<int>> JobSeqinFam,int &FacSpan, int d, vector<int> &FamsExtracted);
	int Construction(vector<int> &FamSeq, vector<vector<int>> JobSeqinFam, vector<int> FamsExtracted, int &FacSpan);
	void Evolution();
	double m_Alpha;
	double m_T0;
	double m_Lamda;
	DWORD m_TimeLimit;
	int m_NonImps;
	int m_Iter;
	int m_dmin;
	int m_dmax;
	long m_InitTime;
};

