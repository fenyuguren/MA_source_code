#include "stdafx.h"
#include "IGLin.h"

IGLin::IGLin()
{
}

IGLin::~IGLin()
{
}

void IGLin::SetParameters(double T0, double Lamda, double Alpha, int Iter, int dmin, int dmax, long TimeLimit)
{
	this->m_T0 = T0;
	this->m_Lamda = Lamda;
	this->m_Alpha = Alpha;
	this->m_Iter = Iter;
	this->m_dmax = dmax;
	this->m_dmin = dmin;
	this->m_TimeLimit = TimeLimit;
}

void IGLin::Destruction(vector<int> &FamSeq, vector<vector<int>> JobSeqinFam, int &Span, int d, vector<int> &FamsExtracted)
{
	FamsExtracted.clear();

	while (FamsExtracted.size() < d)
	{
		int Pos = rand() % FamSeq.size();
		FamsExtracted.push_back(FamSeq[Pos]);
		FamSeq.erase(FamSeq.begin() + Pos);
	}

	GetSpan(FamSeq, JobSeqinFam, Span);
}

int IGLin::Construction(vector<int> &FamSeq, vector<vector<int>> JobSeqinFam, vector<int> FamsExtracted, int &FacSpan)
{
	int Span = NEHFam(FamsExtracted, JobSeqinFam, FamSeq, FacSpan);
	return Span;
}


void IGLin::Evolution()
{

	//---------Initialization-----------
	vector<int> FamPrmu(m_Families);
	vector<int>FamSeq;
	vector<vector<int>> JobSeqinFam;
	int Span;
	GetJobTotalPTime();
	GetFamTotalPTime();
	SortJobsinFam(0, 0, JobSeqinFam);
	SortFam(0, 0, FamPrmu);
	NEHFam(FamPrmu, JobSeqinFam, FamSeq, Span);
	//update bestsofar
	this->m_bestSpan = Span;
	this->m_bestFamSeq = FamSeq;
	this->m_bestJobSeqinFam = JobSeqinFam;

	vector <int> FamsExtracted;
	vector<int> TempFamSeq;
	vector<vector<int>> TempJobSeqinFam;
	int TempSpan;
	double Temperature = this->m_T0;
	int Gen = 0;
	m_InitTime = ::GetElapsedProcessTime();
	while (true)
	{
		TempFamSeq = FamSeq;
		TempJobSeqinFam = JobSeqinFam;
		TempSpan = Span;

		int d = this->m_dmin + rand() % (this->m_dmax - this->m_dmin + 1);
		this->Destruction(TempFamSeq, JobSeqinFam, TempSpan, d, FamsExtracted);
		TempSpan = this->Construction(TempFamSeq, JobSeqinFam, FamsExtracted, TempSpan);
		if (TempSpan <= Span)
		{
			FamSeq = TempFamSeq;
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
			double r = rand() / (double)RAND_MAX;
			if (r < exp((Span - TempSpan) / Temperature))
			{
				FamSeq = TempFamSeq;
				Span = TempSpan;
			}
		}

		TempFamSeq = FamSeq;
		TempJobSeqinFam = JobSeqinFam;
		TempSpan = Span;
		//执行一次交换
		int pt = rand() % TempFamSeq.size();
		int Fam = TempFamSeq[pt];
		if (TempJobSeqinFam[Fam].size() > 1)
		{
			// randomly select two jobs from Fam to exchange
			int pt1, pt2;
			do {
				pt1 = rand() % TempJobSeqinFam[Fam].size();
				pt2 = rand() % TempJobSeqinFam[Fam].size();
			} while (pt1 == pt2);
			int Job = TempJobSeqinFam[Fam][pt1];
			TempJobSeqinFam[Fam][pt1] = TempJobSeqinFam[Fam][pt2];
			TempJobSeqinFam[Fam][pt2] = Job;
		}
		TempSpan = this->GetSpanInFac(TempFamSeq, TempJobSeqinFam);
		FamInsert(TempFamSeq, TempJobSeqinFam, TempSpan);

		if (TempSpan < Span)
		{
			JobSeqinFam = TempJobSeqinFam;
			FamSeq = TempFamSeq;
			Span = TempSpan;
			if (Span < this->m_bestSpan)
			{
				this->m_bestSpan = Span;
				this->m_bestJobSeqinFam = JobSeqinFam;
				this->m_bestFamSeq = FamSeq;
			}
		}

		long CurTime = GetElapsedProcessTime();
		long ElapsedTime = CurTime - m_InitTime;
		if (ElapsedTime >= m_TimeLimit) break;
		Gen++;
		if (Gen % this->m_Iter == 0)
			Temperature *= this->m_Lamda;

	}
}

void IGLin::RunEvolution_OneConf(int CPUFactor, int Rep)
{
	ReadInstanceFileNameList("TS-I\\");
	double T0 = 5; //calibrated 20190815
	double Alpha = 0.9;
	double Lamda = 0.9;
	int Iter = 300;
	int dmin = 2;
	int dmax = 7;
	int Instances = 135;
	vector<int> result;
	for (int ins = 0; ins < Instances; ins++)
	{
		ReadInstance(ins);
		vector<int>SpanArray;
		for (int cishu = 0; cishu < Rep; cishu++) {
			long StartTime_Ins = ::GetElapsedProcessTime();
			SetParameters(T0, Lamda, Alpha, Iter, dmin, dmax, CPUFactor*m_Families*m_Machines*m_Jobs);
			Evolution();
			CheckSol(m_bestFamSeq, m_bestJobSeqinFam, m_bestSpan);
			long EndTime_Ins = ::GetElapsedProcessTime();
			SpanArray.push_back(m_bestSpan);
			cout << ins << "\t" << cishu << "\t" << m_Families << "\t" << m_Jobs << "\t" << m_Machines
				<< "\t" << CPUFactor << "\t" << m_bestSpan << "\t" << EndTime_Ins - StartTime_Ins << endl;

		}
		double sumValue = accumulate(SpanArray.begin(), SpanArray.end(), 0.0);   // accumulate函数就是求vector和的函数；
		double meanValue = sumValue / SpanArray.size();                   // 求均值
		result.push_back(meanValue);
	}
	ofstream ofile;
	ostringstream ostr;
	ostr << "IGLin_" << m_Families << "_" << CPUFactor << "_" << Rep << ".txt";
	ofile.open(ostr.str());
	for (int i = 0; i < result.size(); i++) {
		ofile << result[i] << endl;
	}
	ofile.close();
}

