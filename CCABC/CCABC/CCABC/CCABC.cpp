#include "stdafx.h"
#include "CCABC.h"


CCABC::CCABC()
{
}

CCABC::~CCABC()
{
}

void CCABC::SetParameters(int PS, int Theita, long TimeLimit)
{
	this->m_PSize = PS;
	this->m_Theita = Theita;
	this->m_TimeLimit = TimeLimit;	
	this->m_FamSeqArray.clear();
	this->m_FamSeqArray.resize(this->m_PSize);
	this->m_JobSeqinFamArray.clear();
	this->m_JobSeqinFamArray.resize(this->m_PSize);
	this->m_SpanArray1.clear();
	this->m_SpanArray1.resize(this->m_PSize);
	this->m_SpanArray2.clear();
	this->m_SpanArray2.resize(this->m_PSize);
	this->m_OperType1.clear();
	this->m_OperType1.resize(this->m_PSize, rand() % 2);
	this->m_OperType2.clear();
	this->m_OperType2.resize(this->m_PSize, rand() % 2);
}

void CCABC::InitPop()
{
	//Generate m_PS initial solutions	
	for (int PS = 0; PS < this->m_PSize; PS++)
	{
		vector<int> FamPrmu(m_Families);
		vector<int>FamSeq;
		vector<vector<int>> JobSeqinFam;
		int Span;
		if (PS == 0) //Generate job sequence in each family and family sequence using LPT
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

		NEHFam(FamPrmu, JobSeqinFam, FamSeq, Span);

		m_JobSeqinFamArray[PS] = JobSeqinFam;
		m_FamSeqArray[PS] = FamSeq;
		m_SpanArray1[PS] = Span;
		m_SpanArray2[PS] = m_SpanArray1[PS];
	}

	//Initialize best solution found so far
	vector<int>::iterator It= min_element(m_SpanArray1.begin(), m_SpanArray1.end());
	m_bestSpan = *It;
	int pt = It - m_SpanArray1.begin();
	m_bestFamSeq=m_FamSeqArray[pt];
	m_bestJobSeqinFam = m_JobSeqinFamArray[pt];

	// Initialize Base
	m_BaseFamSeq = m_bestFamSeq;
	m_BaseJobSeqinFam = m_bestJobSeqinFam;
	m_BaseSpan = m_bestSpan;

	//Re-evaluate solutions using base;
	for (int PS = 0; PS < this->m_PSize; PS++)
	{
		m_SpanArray1[PS] = this->GetSpan(m_FamSeqArray[PS], m_BaseJobSeqinFam);
		m_SpanArray2[PS] = this->GetSpan(m_BaseFamSeq, m_JobSeqinFamArray[PS]);
	}
}

void CCABC::Evolution()
{
	double v = 0.75;
	vector<int>FamSeq;
	vector<vector<int>> JobSeq;	
	int It_nonImp = 0;
	m_InitTime = GetElapsedProcessTime();
	while (true)
	{
		//-------Evolving Family population-----------
		//Employed bee phase
		for (int PS = 0; PS < m_PSize; PS++)
		{
			FamSeq = m_FamSeqArray[PS];
			int OperType;
			double r = rand() / (double)RAND_MAX;
			if (r < v)
			{
				if (m_OperType1[PS] == 0)
				{
					ShiftFam(FamSeq);
					OperType = 0;
				}
				else
				{
					SwapFam(FamSeq);
					OperType = 1;
				}
			}
			else
			{
				if (rand() % 2)
				{
					ShiftFam(FamSeq);
					OperType = 0;
				}
				else
				{
					SwapFam(FamSeq);
					OperType = 1;
				}
			}

			int TempSpan = GetSpan(FamSeq, m_BaseJobSeqinFam);
			if (TempSpan < m_SpanArray1[PS])
			{
				m_SpanArray1[PS] = TempSpan;
				m_FamSeqArray[PS] = FamSeq;
				m_OperType1[PS] = OperType;
			}
		}

		//onlooker bee phase
		for (int PS = 0; PS < m_PSize; PS++)
		{
			//tournament selection
			int Pt1 = rand() % m_PSize, Pt2;
			do {
				Pt2 = rand() % m_PSize;
			} while (Pt1 == Pt2);
			FamSeq = (m_SpanArray1[Pt1] > m_SpanArray1[Pt2]) ? m_FamSeqArray[Pt2] : m_FamSeqArray[Pt1];

			int OperType;
			double r = rand() / (double)RAND_MAX;
			if (r < v)
			{
				if (m_OperType1[PS] == 0)
				{
					ShiftFam(FamSeq);
					OperType = 0;
				}
				else
				{
					SwapFam(FamSeq);
					OperType = 1;
				}
			}
			else
			{
				if (rand() % 2)
				{
					ShiftFam(FamSeq);
					OperType = 0;
				}
				else
				{
					SwapFam(FamSeq);
					OperType = 1;
				}
			}

			int TempSpan = GetSpan(FamSeq, m_BaseJobSeqinFam);
			bool bIdentical = false;
			int ind = 0;
			do
			{
				if (FamSeq == m_FamSeqArray[ind])
				{
					bIdentical = true;
					break;
				}
				ind++;
			} while (ind < m_PSize);

			if (bIdentical) continue;

			//Replace the worst 
			vector<int>::iterator It = max_element(m_SpanArray1.begin(), m_SpanArray1.end());
			int maxSpan = *It;
			int maxPt = It - m_SpanArray1.begin();
			if (TempSpan < m_SpanArray1[maxPt])
			{
				m_FamSeqArray[maxPt] = FamSeq;
				m_SpanArray1[maxPt] = TempSpan;
				m_OperType1[maxPt] = OperType;
			}
		}

		//update base
		vector<int>::iterator it = min_element(m_SpanArray1.begin(), m_SpanArray1.end());
		int minSpan = *it;
		int minPt = it - m_SpanArray1.begin();
		if (minSpan < this->m_BaseSpan)
		{
			this->m_BaseSpan = minSpan;
			this->m_BaseFamSeq = m_FamSeqArray[minPt];

			//update makespan for jobseq population
			for (int PS = 0; PS < m_PSize; PS++)
				m_SpanArray2[PS] = GetSpan(m_BaseFamSeq, m_JobSeqinFamArray[PS]);
			It_nonImp = 0;
		}
		else
			It_nonImp++;

		//update bestsofar
		if (minSpan < this->m_bestSpan)
		{
			this->m_bestSpan = minSpan;
			this->m_bestFamSeq = m_FamSeqArray[minPt];
			this->m_bestJobSeqinFam = m_BaseJobSeqinFam;
			//cout <<"FamEv:" <<this->m_bestSpan << endl;
		}

		//-------------JobSeq evolution phase--------------
		//Employed bee phase
		for (int PS = 0; PS < m_PSize; PS++)
		{
			JobSeq = m_JobSeqinFamArray[PS];
			int OperType;
			double r = rand() / (double)RAND_MAX;
			if (r < v)
			{
				if (m_OperType2[PS] == 0)
				{
					ShiftJob(JobSeq);
					OperType = 0;
				}
				else
				{
					SwapJob(JobSeq);
					OperType = 1;
				}
			}
			else
			{
				if (rand() % 2)
				{
					ShiftJob(JobSeq);
					OperType = 0;
				}
				else
				{
					SwapJob(JobSeq);
					OperType = 1;
				}
			}

			int TempSpan = GetSpan(m_BaseFamSeq, JobSeq);
			if (TempSpan < m_SpanArray2[PS])
			{
				m_SpanArray2[PS] = TempSpan;
				m_JobSeqinFamArray[PS] = JobSeq;
				m_OperType2[PS] = OperType;
			}
		}

		//onlooker bee phase
		for (int PS = 0; PS < m_PSize; PS++)
		{
			//tournament selection
			int Pt1 = rand() % m_PSize, Pt2;
			do {
				Pt2 = rand() % m_PSize;
			} while (Pt1 == Pt2);
			JobSeq = (m_SpanArray2[Pt1] > m_SpanArray2[Pt2]) ? m_JobSeqinFamArray[Pt2] : m_JobSeqinFamArray[Pt1];

			int OperType;
			double r = rand() / (double)RAND_MAX;
			if (r < v)
			{
				if (m_OperType2[PS] == 0)
				{
					ShiftJob(JobSeq);
					OperType = 0;
				}
				else
				{
					SwapJob(JobSeq);
					OperType = 1;
				}
			}
			else
			{
				if (rand() % 2)
				{
					ShiftJob(JobSeq);
					OperType = 0;
				}
				else
				{
					SwapJob(JobSeq);
					OperType = 1;
				}
			}

			int TempSpan = GetSpan(m_BaseFamSeq,JobSeq);
			bool bIdentical = false;
			int ind = 0;
			do
			{
				if (JobSeq == m_JobSeqinFamArray[ind])
				{
					bIdentical = true;
					break;
				}
				ind++;
			} while (ind < m_PSize);

			if (bIdentical) continue;

			//Replace the worst 
			vector<int>::iterator It = max_element(m_SpanArray2.begin(), m_SpanArray2.end());
			int maxSpan = *It;
			int maxPt = It - m_SpanArray2.begin();
			if (TempSpan < m_SpanArray2[maxPt])
			{
				m_JobSeqinFamArray[maxPt] = JobSeq;
				m_SpanArray2[maxPt] = TempSpan;
				m_OperType2[maxPt] = OperType;
			}
		}

		//update base
		it = min_element(m_SpanArray2.begin(), m_SpanArray2.end());
		minSpan = *it;
		minPt = it - m_SpanArray2.begin();
		if (minSpan < this->m_BaseSpan)
		{
			this->m_BaseSpan = minSpan;
			this->m_BaseJobSeqinFam = m_JobSeqinFamArray[minPt];
			for (int PS = 0; PS < m_PSize; PS++)//update makespan for Famseq population
				m_SpanArray1[PS] = GetSpan(m_FamSeqArray[PS], m_BaseJobSeqinFam);
			It_nonImp = 0;
		}
		else
			It_nonImp++;

		//update bestsofar
		if (minSpan < this->m_bestSpan)
		{
			this->m_bestSpan = minSpan;
			this->m_bestFamSeq = m_BaseFamSeq;
			this->m_bestJobSeqinFam = m_JobSeqinFamArray[minPt];
			//cout << "JobEv:" << this->m_bestSpan << endl;
		}

		//---------------Exploration phase-----------
		if (It_nonImp >= this->m_Theita)
		{
			vector<int>bestFamSeq;
			vector<vector<int>>bestJobSeq;
			int bestSpan = INT_MAX, Iters = 3;
			
			for (int PS = 0; PS < m_PSize; PS++)//regenerate base
			{
				FamSeq = m_BaseFamSeq;
				JobSeq = m_BaseJobSeqinFam;
				for (int iter = 0; iter < Iters; iter++)
				{
					int op = rand() % 4;
					switch (op)
					{
					case 0: SwapFam(FamSeq); break;
					case 1: ShiftFam(FamSeq); break;
					case 2: SwapJob(JobSeq); break;
					case 3: ShiftJob(JobSeq); break;
					default:break;
					}
				}
				int TempSpan = GetSpan(FamSeq, JobSeq);
				if (TempSpan < bestSpan)
				{
					bestFamSeq = FamSeq;
					bestJobSeq = JobSeq;
					bestSpan = TempSpan;
				}
			}
			
			this->m_BaseFamSeq = bestFamSeq;
			this->m_BaseJobSeqinFam = bestJobSeq;
			this->m_BaseSpan = bestSpan;

			if (this->m_BaseSpan < this->m_bestSpan)
			{
				this->m_bestSpan = this->m_BaseSpan;
				this->m_bestFamSeq = this->m_BaseFamSeq;
				this->m_bestJobSeqinFam = this->m_BaseJobSeqinFam;
				//cout << "Explore: " << this->m_bestSpan << endl;
			}

			//reinit populations
			for (int PS = 0; PS < m_PSize; PS++)
			{
				this->m_FamSeqArray[PS] = this->m_BaseFamSeq;
				for (int iter = 0; iter < Iters; iter++)
				{
					if(rand() % 2)
						SwapFam(m_FamSeqArray[PS]);
					else
						ShiftFam(m_FamSeqArray[PS]);
				}
				this->m_SpanArray1[PS] = GetSpan(m_FamSeqArray[PS], m_BaseJobSeqinFam);
			}

			for (int PS = 0; PS < m_PSize; PS++)
			{
				this->m_JobSeqinFamArray[PS] = this->m_BaseJobSeqinFam;
				for (int iter = 0; iter < Iters; iter++)
				{
					if (rand() % 2)
						SwapJob(m_JobSeqinFamArray[PS]);
					else
						ShiftJob(m_JobSeqinFamArray[PS]);
				}
				this->m_SpanArray2[PS] = GetSpan(m_BaseFamSeq, m_JobSeqinFamArray[PS]);
			}
			this->m_Theita = 0;
		}

		long CurTime = GetElapsedProcessTime();
		long ElapsedTime = CurTime - m_InitTime;
		if (ElapsedTime >= m_TimeLimit) break;		
	}
}


void CCABC::RunEvolution_OneConf(int CPUFactor, int Rep)
{
	ReadInstanceFileNameList("TS-II\\");
	int PSize = 50;//Calibrated 20190817
	int Theita = INT_MAX;
	int Instances = 135;
	vector<int> result;
	for (int ins = 0; ins < Instances; ins++)
	{
		ReadInstance(ins);
		vector<int>SpanArray;
		for (int cishu = 0; cishu < Rep; cishu++) {
			long StartTime_Ins = ::GetElapsedProcessTime();
			SetParameters(PSize, Theita, CPUFactor*m_Families*m_Machines*m_Jobs);
			InitPop();
			Evolution();
			CheckSol(m_bestFamSeq, m_bestJobSeqinFam, m_bestSpan);
			SpanArray.push_back(m_bestSpan);
			long EndTime_Ins = ::GetElapsedProcessTime();
			cout << ins << "\t" << cishu << "\t" << m_Families << "\t" << m_Jobs << "\t" << m_Machines
				<< "\t" << CPUFactor << "\t" << m_bestSpan << "\t" << EndTime_Ins - StartTime_Ins << endl;
		}
		double sumValue = accumulate(SpanArray.begin(), SpanArray.end(), 0.0);   // accumulate函数就是求vector和的函数；
		double meanValue = sumValue / SpanArray.size();                   // 求均值
		result.push_back(meanValue);
	}
	ofstream ofile;
	ostringstream ostr;
	ostr << "CCABC" << m_Families << "_" << CPUFactor << "_" << Rep << ".txt";
	ofile.open(ostr.str());
	for (int i = 0; i < result.size(); i++) {
		ofile << result[i] << endl;
	}
	ofile.close();
}

