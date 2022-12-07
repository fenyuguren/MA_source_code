#include "stdafx.h"
#include "IGPan.h"


IGPan::IGPan()
{
}

IGPan::~IGPan()
{
}
void IGPan::SetParameters(int d, long TimeLimit)
{
	this->m_d = d;	
	this->m_TimeLimit = TimeLimit;
}
void IGPan::Destruction(vector<int> &FacFamSeq, vector<vector<int>> JobSeqinFam, int &FacSpan, int d, vector<int> &FamsExtracted)
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
int IGPan::Construction(vector<int> &FacFamSeq, vector<vector<int>> JobSeqinFam, vector<int> FamsExtracted, int &FacSpan)
{
	int Span = NEHFamwithTiebreak(FamsExtracted, JobSeqinFam, FacFamSeq, FacSpan);
	return Span;
}

void IGPan::RandPointLocalSearch(vector<int>&FamSeq, vector<vector<int>> JobSeqinFam,int &Span,int H){
	vector<int> TempFamSeq = FamSeq;
	vector<int> PartialFamSeq;
	PartialFamSeq.clear();
	int RandPoint = rand() % (m_Families-H+1);
	for (int j = RandPoint; j < RandPoint+H; j++) {
		PartialFamSeq.push_back(TempFamSeq[j]);
	}
	int len = PartialFamSeq.size() + 1;
	vector<vector<int>>CostMatrix(len);
	for (int j = 0; j < len; j++) {
		CostMatrix[j].resize(len);
	}
	int head, tail;
	if (RandPoint == 0) {
		head = -1;
	}
	else {
		head = TempFamSeq[RandPoint-1];
	}
	if (RandPoint == m_Families - H) {
		tail = m_Families;
	}
	else {
		tail = TempFamSeq[RandPoint + H];
	}	 
	
	GetCostMatrix(CostMatrix, PartialFamSeq, head, tail);
	vector<int>OptimalRing = LocalSearch(CostMatrix);
	vector<int> AjustPartialFamSeq;
	AjustPartialFamSeq.clear();
	for (int index = 1; index < OptimalRing.size(); index++) {
		//cout << OptimalRing[index]<<" ";
		AjustPartialFamSeq.push_back(PartialFamSeq[OptimalRing[index] - 1]);
	}
	//cout << endl;
	int start = 0;
	for (int j = RandPoint; j < RandPoint + H; j++) {
		TempFamSeq[j] = AjustPartialFamSeq[start];
		start++;
	}
	int TempSpan=GetSpan(TempFamSeq, JobSeqinFam);
	if (TempSpan < Span) {
		Span = TempSpan;
		FamSeq = TempFamSeq;
	}
}
void IGPan::Evolution()
{

	//---------Initialization-----------
	int Span;
	vector<vector<int>> JobSeqinFam;
	vector<int>FamSeq;
	JPA(FamSeq, JobSeqinFam, Span);
	
	this->m_bestSpan = Span;
	this->m_bestFamSeq = FamSeq;
	this->m_bestJobSeqinFam = JobSeqinFam;

	vector <int> FamsExtracted;
	vector<int> TempFamSeq;
	vector<vector<int>> TempJobSeqinFam;
	//vector<int> TempFacSpan(this->m_Factories, 0);
	int TempSpan;
	int Gen = 0, H = 19;
	this->m_RecordSpan.clear();
	m_InitTime = ::GetElapsedProcessTime();
	//int Record = 1;
	while (true)
	{
		TempFamSeq = FamSeq;
		TempJobSeqinFam = JobSeqinFam;
		TempSpan = Span;

		this->Destruction(TempFamSeq, TempJobSeqinFam, TempSpan, this->m_d, FamsExtracted);
		TempSpan = this->Construction(TempFamSeq, TempJobSeqinFam, FamsExtracted, TempSpan);

		JobTieInsert(TempFamSeq, TempJobSeqinFam, TempSpan);
		JobSwap(TempFamSeq, TempJobSeqinFam, TempSpan);
		FamInsert(TempFamSeq, TempJobSeqinFam, TempSpan);
		FamSwap(TempFamSeq, TempJobSeqinFam, TempSpan);

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
			double r = rand() / (double)RAND_MAX;
			if (r < 0.1)
			{
				FamSeq = TempFamSeq;
				JobSeqinFam = TempJobSeqinFam;
				Span = TempSpan;
			}
		}
		//以一定的概率执行局部搜索
		double r = rand() / (double)RAND_MAX;
		if (m_Families > 20 && r < 0.1) {
			TempFamSeq = FamSeq;
			TempJobSeqinFam = JobSeqinFam;
			TempSpan = Span;
			//fam局部搜索
			RandPointLocalSearch(TempFamSeq, TempJobSeqinFam, TempSpan, H);
			//job局部搜索
			JobTieInsert(TempFamSeq, TempJobSeqinFam, TempSpan);
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
		}

		long CurTime = GetElapsedProcessTime();
		long ElapsedTime = CurTime - m_InitTime;
		if (ElapsedTime >= m_TimeLimit) break;
		Gen++;
	}

}

void IGPan::RunEvolution_OneConf(int CPUFactor, int Rep)
{
	ReadInstanceFileNameList("TS-I\\");
	int d = 4; //Calibrated 20190821
	//double TempFactor = 2.5;
	int Instances = 135;
	vector<int> result;
	ofstream ofile;
	ostringstream ostr;
	ostr << "MA_Instance_Result"<<".txt";
	ofile.open(ostr.str());
	for (int ins = 0; ins < Instances; ins++)
	{
		ReadInstance(ins);
		vector<int>SpanArray;
		for (int cishu = 0; cishu < Rep; cishu++) {
			long StartTime_Ins = ::GetElapsedProcessTime();
			SetParameters(d, CPUFactor*m_Jobs*m_Families*m_Machines);
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
		ofile << ins<<" "<<meanValue << endl;
	}
	ofile.close();
	ofstream ofile_v1;
	ostringstream ostr_v1;
	ostr_v1 << "MA_"<< CPUFactor << "_" << Rep << ".txt";
	ofile_v1.open(ostr_v1.str());
	for (int i = 0; i < result.size(); i++){		
		ofile_v1 << result[i] << endl;
	}
	ofile_v1.close();
}
