#pragma once
#include "Heu.h"
class IGPan ://EWSA2019
	public Heu
{
public:
	IGPan();
	virtual ~IGPan();
	//void Calibration_OneConf(int No);
	//void CalibrationTwoStage_OneConf(int No);
	//void Calibration();
	//void CalibrationTwoStage();
	//void Sta_Calibration();
	//void Sta_CalibrationTwoStage();
	void RunEvolution_OneConf(int CPUFactor, int Rep);
	//void RunEvolutionTwoStage_OneConf(int CPUFactor, int Rep);
	//void RunEvolution();
private:
	void SetParameters(int d, double TempFactor, long TimeLimit);
	//void SetParameters(int d, int d2, double Ro, double TempFactor, long TimeLimit);
	void Destruction(vector<int> &FacFamSeq, vector<vector<int>> JobSeqinFam, int &FacSpan, int d, vector<int> &FamsExtracted);
	//void Destruction_InsideFac(vector<int> &FamSeqInFac, vector<vector<int>> JobSeqinFam, int d, vector<int> &FamsExtracted, int &Span);
	int Construction(vector<int> &FacFamSeq, vector<vector<int>> JobSeqinFam, vector<int> FamsExtracted, int &FacSpan);
	//int Construction_InsideFac(vector<int> &FamSeqInFac, vector<vector<int>> JobSeqinFam, vector<int> FamsExtracted, int &Span);
	double GetTemperature();
	void RandPointLocalSearch(vector<int>&FamSeq, vector<vector<int>> JobSeqinFam, int &Span, int len);
	void Evolution();
	//void EvolutionTwoStage();
	int m_d;
	double m_TempFactor;
	long m_TimeLimit;
	long m_InitTime;
	vector<int> m_RecordSpan;
	vector<int> m_bestFacSpan;
	double m_Ro;//For two-stage IG
	int m_d2;
};


