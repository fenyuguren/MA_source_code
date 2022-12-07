#include "stdafx.h"
#include "TLIG.h"


TLIG::TLIG()
{
}

TLIG::~TLIG()
{
}
void TLIG::SetParameters(int d, double TempFactor, long TimeLimit)
{
	this->m_d = d;
	this->m_TempFactor = TempFactor;
	this->m_TimeLimit = TimeLimit;
}
void TLIG::Destruction(vector<int> &FacFamSeq, vector<vector<int>> JobSeqinFam, int &FacSpan, int d, vector<int> &FamsExtracted)
{
	FamsExtracted.clear();
	while (FamsExtracted.size() < d)
	{
		int Pos = rand() % FacFamSeq.size();
		FamsExtracted.push_back(FacFamSeq[Pos]);
		FacFamSeq.erase(FacFamSeq.begin() + Pos);
	}

	GetSpan(FacFamSeq, JobSeqinFam, FacSpan);
}

void TLIG::MyDestruction(vector<int> &FacFamSeq, vector<vector<int>> &JobSeqinFam, vector<int> &FamsExtracted, vector<vector<int>> &JobSeqinFamExtracted)
{
	FamsExtracted.clear();
	int sigma = rand() % FacFamSeq.size();
	while (FamsExtracted.size() < sigma)
	{
		int Pos = rand() % FacFamSeq.size();
		FamsExtracted.push_back(FacFamSeq[Pos]);
		FacFamSeq.erase(FacFamSeq.begin() + Pos);
	}
	JobSeqinFamExtracted.clear();
	JobSeqinFamExtracted.resize(sigma);
	for (int i = 0; i < sigma; i++) {	
		int fam=FamsExtracted[i];
		vector<int> jobseq = JobSeqinFam[fam];
		int len = jobseq.size();
		if (len == 1)continue;
		int peta = rand() % (len-1);
		while (JobSeqinFamExtracted[i].size() < peta)
		{
			int Pos = rand() % jobseq.size();
			JobSeqinFamExtracted[i].push_back(jobseq[Pos]);
			jobseq.erase(jobseq.begin() + Pos);
		}
		JobSeqinFam[fam] = jobseq;
	}	
}
int TLIG::MyConstruction(vector<int> &FacFamSeq, vector<vector<int>> &JobSeqinFam, vector<int> FamsExtracted, vector<vector<int>> JobSeqinFamExtracted)
{
	
	int sigma=FamsExtracted.size();
	for (int i = 0; i < sigma; i++) {
		int CurInsertFam=FamsExtracted[i];
		vector<int> TemSeq;
		TemSeq.clear();
		TemSeq.push_back(CurInsertFam);
		int PreFacSpan = GetSpan(FacFamSeq, JobSeqinFam);
		int Span = NEHFam(TemSeq, JobSeqinFam, FacFamSeq, PreFacSpan);
		vector<int>ReinsertJobSeq=JobSeqinFamExtracted[i];
		int len= ReinsertJobSeq.size();
		if (len > 0) {
			
			for (int j = 0; j < len; j++) {
				int InsertJob= ReinsertJobSeq[j];
				vector<int> jobseq = JobSeqinFam[CurInsertFam];
				int positions = jobseq.size() + 1;
				int bestpos = -1;
				int minspan = INT_MAX;
				for (int pos = 0; pos < positions; pos++) {
					vector <int >TempJobSeq = jobseq;
					TempJobSeq.insert(TempJobSeq.begin() + pos, InsertJob);
					JobSeqinFam[CurInsertFam] = TempJobSeq;
					int tempspan=GetSpan(FacFamSeq, JobSeqinFam);
					if (tempspan < minspan) {					
						minspan = tempspan;
						bestpos = pos;					
					}				
				}
				jobseq.insert(jobseq.begin() + bestpos, InsertJob);
				JobSeqinFam[CurInsertFam] = jobseq;			
			}		
		}	
	}
	int FacSpan = GetSpan(FacFamSeq, JobSeqinFam);	
	return FacSpan;
}


int TLIG::Construction(vector<int> &FacFamSeq, vector<vector<int>> JobSeqinFam, vector<int> FamsExtracted, int &FacSpan)
{
	int Span = NEHFam(FamsExtracted, JobSeqinFam, FacFamSeq, FacSpan);
	return Span;
}
double TLIG::GetTemperature()
{
	int Sum = 0;
	for (int j = 0; j < this->m_Jobs; j++)
		for (int m = 0; m < this->m_Machines; m++)
			Sum += this->m_JobOperPTime[j][m];
	double t = this->m_TempFactor*Sum / (10 * this->m_Jobs*this->m_Machines);
	return t;
}

void TLIG::Evolution()
{
	
	//---------Initialization-----------
	int Span;
	vector<vector<int>> JobSeqinFam;
    vector<int>FamSeq, FamPrmu;	
	SchedulingJobsinFam(JobSeqinFam);
	GetJobTotalPTime();	
	GetFamTotalPTime();
	SortFam(0, 0, FamPrmu);
	NEHFam(FamPrmu, JobSeqinFam, FamSeq, Span);

	this->m_bestSpan = Span;
	this->m_bestFamSeq = FamSeq;
	this->m_bestJobSeqinFam = JobSeqinFam;

	vector <int> FamsExtracted;
	vector<vector<int>>JobSeqinFamExtracted;
	vector<int> TempFamSeq;
	vector<vector<int>> TempJobSeqinFam;	
	int TempSpan;

	//double Temperature = this->GetTemperature();
	int SumTime = accumulate(m_JobTotalPTime.begin(), m_JobTotalPTime.end(), 0.0);
	double T = (SumTime/(m_Jobs*m_Machines))*0.9;
	
	m_InitTime = ::GetElapsedProcessTime();
	while (true) {
		TempFamSeq = FamSeq;
		TempJobSeqinFam = JobSeqinFam;
		TempSpan = Span;	
		MyDestruction(TempFamSeq, TempJobSeqinFam,  FamsExtracted, JobSeqinFamExtracted);
		int TempSpan=MyConstruction(TempFamSeq, TempJobSeqinFam, FamsExtracted, JobSeqinFamExtracted);
		//int TempSpan = GetSpan(TempFamSeq, TempJobSeqinFam);
		//CheckSol(TempFamSeq, TempJobSeqinFam, TempSpan);
		if (TempSpan <= Span)
		{
			FamSeq = TempFamSeq;
			JobSeqinFam = TempJobSeqinFam;
			Span = TempSpan;			
			if (Span < this->m_bestSpan)
			{
				this->m_bestSpan = Span;
				this->m_bestJobSeqinFam = JobSeqinFam;
				this->m_bestFamSeq = FamSeq;
			}
		}
		else
		{		
			double detC = m_bestSpan - TempSpan;
			double r = rand() / (double)RAND_MAX;
			if (r < T / (T*T + detC * detC)) {
				FamSeq = TempFamSeq;
				JobSeqinFam = TempJobSeqinFam;
				Span = TempSpan;			
			}			
		}
        long CurTime = GetElapsedProcessTime();
	    long ElapsedTime = CurTime - m_InitTime;
		if (ElapsedTime >= m_TimeLimit) break;

	}
	
}

void TLIG::RunEvolution_OneConf(int CPUFactor, int Rep)
{
	ReadInstanceFileNameList("TS-I\\");
	int d = 4; //Calibrated 20190821
	double TempFactor = 2.5;
	int Instances = 135;
	vector<int> result;
	for (int ins = 0; ins < Instances; ins++)
	{
		ReadInstance(ins);
		vector<int>SpanArray;
		for (int cishu = 0; cishu < Rep; cishu++) {
			long StartTime_Ins = ::GetElapsedProcessTime();
			SetParameters(d, TempFactor, CPUFactor*m_Jobs*m_Families*m_Machines);
			Evolution();
			CheckSol(m_bestFamSeq, m_bestJobSeqinFam, m_bestSpan);
			long EndTime_Ins = ::GetElapsedProcessTime();
			SpanArray.push_back(this->m_bestSpan);
			cout << ins <<"\t"<<cishu<< "\t" << m_Families << "\t" << m_Jobs << "\t" << m_Machines
				<< "\t"  << CPUFactor << "\t" << m_bestSpan << "\t" << EndTime_Ins - StartTime_Ins << endl;
		}
		double sumValue = accumulate(SpanArray.begin(), SpanArray.end(), 0.0);   // accumulate函数就是求vector和的函数；
		double meanValue = sumValue / SpanArray.size();                   // 求均值
		result.push_back(meanValue);

	}
	ofstream ofile;
	ostringstream ostr;
	ostr << "TLIG_" << m_Families<<"_"<< CPUFactor << "_" << Rep << ".txt";
	ofile.open(ostr.str());
	for (int i = 0; i < result.size(); i++){		
		ofile << result[i] << endl;		
	}
	ofile.close();
}
