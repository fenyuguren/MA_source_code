#include "stdafx.h"
#include "CCEA.h"

CCEA::CCEA()
{
}

CCEA::~CCEA()
{
}

void CCEA::SetParameters(int RefSize, int PS1, int PS2, int AgeLimit,int DesLen,long TimeLimit) //PS1>RefSize; PS2>RefSize;
{
	m_RefSize = RefSize;
	m_PS1 = PS1;
	m_PS2 = PS2;
	m_AgeLimit = AgeLimit;
	m_DesLen = DesLen;	
	m_TimeLimit = TimeLimit;
	

	//Reference set
	m_RefSpanArray.clear();
	m_RefSpanArray.resize(m_RefSize);	
	m_RefFamSeqArray.clear();
	m_RefFamSeqArray.resize(m_RefSize);
	m_RefJobSeqinFamArray.clear();
	m_RefJobSeqinFamArray.resize(m_RefSize);
	
	m_bFlag1.clear();
	m_bFlag1.resize(m_RefSize);
	m_bFlag2.clear();
	m_bFlag2.resize(m_RefSize);

	m_SpanArray1.clear();
	m_SpanArray1.resize(m_PS1);	
	m_Map1.clear();
	m_Map1.resize(m_PS1);
	m_Age1.clear();
	m_Age1.resize(m_PS1);
	m_FamSeqArray.clear();
	m_FamSeqArray.resize(m_PS1);

	m_SpanArray2.clear();
	m_SpanArray2.resize(m_PS2);	
	m_Map2.clear();
	m_Map2.resize(m_PS2);
	m_Age2.clear();
	m_Age2.resize(m_PS2);
	m_JobSeqinFamArray.clear();
	m_JobSeqinFamArray.resize(m_PS2);
}

void CCEA::InitPop()
{
	//Initialize Reference set and Set flags
	for (int PS = 0; PS < m_RefSize; PS++)
	{
		vector<int> FamPrmu(m_Families);
		vector<int>FamSeq;
		vector<vector<int>> JobSeqinFam;
		int Span;
		if (PS == 0) {
			JPA(FamSeq, JobSeqinFam, Span);
		}
		else {
			if (PS == 1) //Generate job sequence in each family and family sequence using LPT
			{
				GetJobTotalPTime();
				GetFamTotalPTime();
				SortJobsinFam(0, 0, JobSeqinFam);
				SortFam(0, 0, FamPrmu);
			}
			else //Generate job sequence in each family and family sequence randomly
			{
				JobSeqinFam = m_JobsinEachFamily;
				for (int fam = 0; fam < JobSeqinFam.size(); fam++)
					random_shuffle(JobSeqinFam[fam].begin(), JobSeqinFam[fam].end());
				for (int fam = 0; fam < FamPrmu.size(); fam++)
					FamPrmu[fam] = fam;
				random_shuffle(FamPrmu.begin(), FamPrmu.end());
			}
			//Generte a solution using assignment rule--include	
			NEHFam(FamPrmu, JobSeqinFam, FamSeq, Span);
		
		}	
		//Initialize a reference
		m_RefJobSeqinFamArray[PS] = JobSeqinFam;
		m_RefFamSeqArray[PS] = FamSeq;
		m_RefSpanArray[PS] = Span;
		m_bFlag1[PS] = false;
		m_bFlag2[PS] = false;
	}

	//Initialize best solution found so far
	m_bestSpan = m_RefSpanArray[0];
	int pt = 0;
	for (int PS = 1; PS < m_RefSize; PS++)
	{
		if (m_RefSpanArray[PS] < m_bestSpan)
		{
			m_bestSpan = m_RefSpanArray[PS];
			pt = PS;
		}
	}
	m_bestFamSeq = m_RefFamSeqArray[pt];
	m_bestJobSeqinFam = m_RefJobSeqinFamArray[pt];

	//Initilize Familiy-sequence population, i.e., PS1
	for (int PS = 0; PS < min(m_PS1,m_RefSize); PS++)
	{
		m_FamSeqArray[PS] = m_RefFamSeqArray[PS];
		m_SpanArray1[PS] = m_RefSpanArray[PS];
		m_Map1[PS] = PS;		
	}
	for (int PS = min(m_PS1, m_RefSize); PS < m_PS1; PS++)
	{
		//randomly select a Family Sequence from reference set and Swap
		int r = rand() % m_RefSize;
		m_FamSeqArray[PS] = m_RefFamSeqArray[r];
		SwapFam(m_FamSeqArray[PS]);
		SwapFam(m_FamSeqArray[PS]);
		//randomly select a job seqence from reference set
		m_Map1[PS] = rand() % m_RefSize;		
		m_SpanArray1[PS] = GetSpan(m_FamSeqArray[PS], m_RefJobSeqinFamArray[m_Map1[PS]]);
		
	}

	//Initialize Job-Sequence population, i.e., PS2
	for (int PS = 0; PS < min(m_PS2,m_RefSize); PS++)
	{
		m_JobSeqinFamArray[PS] = m_RefJobSeqinFamArray[PS];
		m_SpanArray2[PS] = m_RefSpanArray[PS];
		m_Map2[PS] = PS;		
	}
	for (int PS = min(m_PS2, m_RefSize); PS < m_PS2; PS++)
	{
		//randomly select a Job Sequence from reference set and Swap
		int r = rand() % m_RefSize;
		m_JobSeqinFamArray[PS] = m_RefJobSeqinFamArray[r];
		SwapJob(m_JobSeqinFamArray[PS]);
		SwapJob(m_JobSeqinFamArray[PS]);
		//randomly select a Family seqence from reference set
		m_Map2[PS] = rand() % m_RefSize;		
		m_SpanArray2[PS] = GetSpan(m_RefFamSeqArray[m_Map2[PS]], m_JobSeqinFamArray[PS]);		
	}
}

void CCEA::ReInitPop() //reproduce a solution if age Limit is met
{
	for (int PS = 0; PS < m_PS1; PS++) 
	{
		if (m_Age1[PS] < m_AgeLimit) continue;
		m_FamSeqArray[PS] = m_RefFamSeqArray[m_Map1[PS]];
		int FacSpan=0;
		m_SpanArray1[PS] = Destruction_Construction(m_DesLen, m_FamSeqArray[PS], m_RefJobSeqinFamArray[m_Map1[PS]], FacSpan);
		m_Age1[PS] = 0;
	}

	for (int PS = 0; PS < m_PS2; PS++)
	{
		if (m_Age2[PS] < m_AgeLimit) continue;
		m_JobSeqinFamArray[PS] = m_RefJobSeqinFamArray[m_Map2[PS]];
		SwapJob(m_JobSeqinFamArray[PS]);
		SwapJob(m_JobSeqinFamArray[PS]);
		int Span;		
		m_Age2[PS] = 0;
	}
}

int CCEA::Destruction_Construction(int Len, vector<int> &FamSeq, vector<vector<int>> JobSeqinFam, int &Span)
{
	
	//Extract Len/2 families from cirtical factory
	vector<int> ExtractFamSeq;
	do {
		
			int Pos = rand() % FamSeq.size();
			ExtractFamSeq.push_back(FamSeq[Pos]);
			FamSeq.erase(FamSeq.begin() + Pos);		
	} while (ExtractFamSeq.size() < Len);

	GetSpan(FamSeq, JobSeqinFam, Span);
	//Insert the extracted Families into the best Positions
	return NEHFam(ExtractFamSeq, JobSeqinFam, FamSeq, Span);
}
void CCEA::Evolution2()//randomly setup the mapping relationship, compare with the original relationship, considering span and the number of critical pathes
{
	m_InitTime = GetElapsedProcessTime();
	while (true)
	{
		//----------Evolution for Family Populaiton------------
		for (int PS = 0; PS < m_PS1; PS++)
		{
			vector<int> FamSeq = m_FamSeqArray[PS];
			int Map1 = rand() % m_RefSize;//form a solution by randomly selecting a RefJobSeq;
			vector<int> FacSpan(m_Factories, 0);
			double r = double(rand()) / RAND_MAX;
			int Span=GetSpan(FamSeq, m_RefJobSeqinFamArray[Map1]);
			FamInsert(FamSeq, m_RefJobSeqinFamArray[Map1], Span);
			FamSwap(FamSeq, m_RefJobSeqinFamArray[Map1], Span);
			//1: The original Maping Reference JobSeq is not changed; 2. Changed. reset corresponding relations
			if ((!m_bFlag2[m_Map1[PS]]&&(Span < m_SpanArray1[PS]))||m_bFlag2[m_Map1[PS]])//Update solution
				{
					m_SpanArray1[PS] = Span;
					m_Map1[PS] = Map1;
					m_FamSeqArray[PS] = FamSeq;					
					m_Age1[PS] = 0;
				}

			//Update RefSet;
			if (Span < m_RefSpanArray[Map1])
			{
				m_RefSpanArray[Map1] = Span;
				m_RefFamSeqArray[Map1] = FamSeq;
				m_bFlag1[Map1] = true;
			}
			m_Age1[PS]++;
		}

		//Reset m_bFlag2; 
		for (int PS = 0; PS < m_RefSize; PS++)
			m_bFlag2[PS] = false;

		//------------Evolution for populaiton 2-----------
		for (int PS = 0; PS < m_PS2; PS++)
		{
			vector<vector<int>> JobSeqinFam = m_JobSeqinFamArray[PS];
			int Map2 = rand() % m_RefSize;//form a solution by randomly selecting a ReFacFamseq
			int Span = GetSpan(m_RefFamSeqArray[Map2], JobSeqinFam);
			JobInsert(m_RefFamSeqArray[Map2], JobSeqinFam, Span);
			JobSwap(m_RefFamSeqArray[Map2], JobSeqinFam, Span);
			//1: The original Maping Reference FamSeq is not changed; 2. Changed. reset corresponding relations
			if ((!m_bFlag1[m_Map2[PS]]&& Span < m_SpanArray2[PS])|| m_bFlag1[m_Map1[PS]])//Update solution
			{
				m_SpanArray2[PS] = Span;
				m_Map2[PS] = Map2;
				m_JobSeqinFamArray[PS] = JobSeqinFam;				
				m_Age2[PS] = 0;
			}

			//Update RefSet;
			if (Span < m_RefSpanArray[Map2])
			{
				m_RefSpanArray[Map2] = Span;
				m_RefJobSeqinFamArray[Map2] = JobSeqinFam;				
				m_bFlag2[Map2] = true;
			}
			m_Age2[PS]++;
		}

		//Reset m_bFlag1;
		for (int PS = 0; PS < m_RefSize; PS++)
			m_bFlag1[PS] = false;

		//Update BestSolution;
		int bestSpan = m_RefSpanArray[0], pt = 0;
		for (int i = 1; i < m_RefSize; i++)
		{
			//CheckSol(m_RefFamSeqArray[i], m_RefJobSeqinFamArray[i], m_RefSpanArray[i]);
			if (m_RefSpanArray[i] < bestSpan)
			{
				bestSpan = m_RefSpanArray[i];
				pt = i;
			}
		}

		if (bestSpan < m_bestSpan)
		{
			m_bestSpan = bestSpan;
			m_bestFamSeq = m_RefFamSeqArray[pt];
			m_bestJobSeqinFam = m_RefJobSeqinFamArray[pt];
		}

		ReInitPop();
	//	cout << bestSpan << endl;
		long CurTime = GetElapsedProcessTime();
		long ElapsedTime = CurTime - m_InitTime;		
		if (ElapsedTime >= m_TimeLimit) break;
	}
	
}

void CCEA::RunEvolution2_OneConf(int CPUFactor, int Rep)
{
	ReadInstanceFileNameList("TS-I\\");
	int RefSize = 5; //Five paramters are Calibrated on 20190724
	int PS1 = 10;
	int PS2 = 10;
	int AgeLimit = 5;
	int DesLen = 6;
	double Rate = 1.0;
	int Instances = 135;
	vector<int> result;
	for (int ins = 0; ins < Instances; ins++)
	{
		ReadInstance(ins);
		vector<int>SpanArray;
		for (int cishu = 0; cishu < Rep; cishu++) {
			long StartTime_Ins = ::GetElapsedProcessTime();
			SetParameters(RefSize, PS1, PS2, AgeLimit, DesLen, CPUFactor*m_Families*m_Machines*m_Jobs);
			InitPop();
			Evolution2();
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
	ostr << "GCCEA_" << m_Families << "_" << CPUFactor << "_" << Rep << ".txt";
	ofile.open(ostr.str());
	for (int i = 0; i < result.size(); i++) {
		ofile << result[i] << endl;
	}
	ofile.close();
}
