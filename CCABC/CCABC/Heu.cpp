#include "stdafx.h"
#include "Heu.h"


Heu::Heu()
{
}

Heu::~Heu()
{
}



void  Heu::WriteFile(string FileName, vector<unsigned int> FactorLevelsArray, vector<vector<double>> RPIArray)
{
	ofstream ofile;
	ofile.open(FileName);
	for (int Ins = 0; Ins < RPIArray.size(); Ins++)
	{
		for (int Method = 0; Method < RPIArray[Ins].size(); Method++)
		{
			int No = Method;
			vector<int> FactorLevel;
			int Start = 0;
			do
			{
				int Multiply = 1;
				for (int i = Start + 1; i<FactorLevelsArray.size(); i++)
					Multiply *= FactorLevelsArray[i];
				int Level = No / Multiply;
				FactorLevel.push_back(Level);
				No %= Multiply;
				Start++;
			} while (Start<FactorLevelsArray.size());

			ofile << Ins << "\t";
			for (int i = 0; i < FactorLevel.size(); i++)
				ofile << FactorLevel[i] << "\t";
			ofile << RPIArray[Ins][Method] << endl;
		}
	}
	ofile.close();
}

void Heu::SortJobsinFam(int Factor, int SortMethod, vector<vector<int>> &JobSeqinFam)
{
	JobSeqinFam = this->m_JobsinEachFamily; //initialize memeory
	for (int Fam = 0; Fam < JobSeqinFam.size(); Fam++)
	{
		Pair<int> *ch = new Pair <int>[JobSeqinFam[Fam].size()];
		for (int j = 0; j < JobSeqinFam[Fam].size(); j++)
		{
			ch[j].dim = JobSeqinFam[Fam][j];
			switch (Factor)
			{
			case 0:ch[j].value = this->m_JobTotalPTime[JobSeqinFam[Fam][j]]; break;
			case 1:ch[j].value = this->m_JobOperPTime[JobSeqinFam[Fam][j]][0]; break;
			case 2:ch[j].value = -this->m_JobOperPTime[JobSeqinFam[Fam][j]][this->m_Machines-1];break;
			case 3:ch[j].value = this->m_JobOperPTime[JobSeqinFam[Fam][j]][0]- this->m_JobOperPTime[JobSeqinFam[Fam][j]][this->m_Machines - 1]; break;
			case 4:ch[j].value = this->m_JobWeightTotalPTime[JobSeqinFam[Fam][j]]; break;
			default:break;
			}
		}

		if (SortMethod == 0)
			sort(ch, ch + JobSeqinFam[Fam].size(), PairGreater<int>());
		else
			sort(ch, ch + JobSeqinFam[Fam].size(), PairLess<int>());
		for (int j = 0; j < JobSeqinFam[Fam].size(); j++)
			JobSeqinFam[Fam][j] = ch[j].dim;
		delete[] ch;
	}
}

void Heu::SortFam(int Factor, int SortMethod, vector<int> &FamPrmu)
{
	FamPrmu.clear();
	FamPrmu.resize(this->m_Families);
	for (int Fam = 0; Fam < this->m_Families; Fam++)
		FamPrmu[Fam] = Fam;

	Pair<double> *ch = new Pair<double>[FamPrmu.size()];
	for (int Fam = 0; Fam <FamPrmu.size(); Fam++)
	{
		ch[Fam].dim = FamPrmu[Fam];
		switch (Factor)
		{
		case 0:ch[Fam].value = this->m_FamTotalPTime[FamPrmu[Fam]]; break;
		case 1:ch[Fam].value = this->m_FamTotalPTime[FamPrmu[Fam]] + this->m_FamAvgSetupTime[FamPrmu[Fam]]; break;
		case 2:ch[Fam].value = this->m_FamTotalPTime[FamPrmu[Fam]] + this->m_FamAvgmaxSetupTime[FamPrmu[Fam]];break;
		case 3:ch[Fam].value = this->m_FamWeightTotalPTime[FamPrmu[Fam]]; break;
		case 4:ch[Fam].value = this->m_FamTotalPTimeOnFirstMachine[FamPrmu[Fam]]; break;
		case 5:ch[Fam].value = this->m_FamTotalPTimeOnLastMachine[FamPrmu[Fam]]; break;
		case 6:ch[Fam].value = this->m_FamTotalPTimeOnLastMachine[FamPrmu[Fam]] + this->m_FamTotalPTimeOnFirstMachine[FamPrmu[Fam]]; break;
		case 7:ch[Fam].value = this->m_FamTotalPTimeOnLastMachine[FamPrmu[Fam]] - this->m_FamTotalPTimeOnFirstMachine[FamPrmu[Fam]]; break;

		default:cout << "Factor is out of range"; break;
		}			
	}
	if(SortMethod == 0)
		sort(ch, ch + this->m_Families, PairGreater<double>());
	else
		sort(ch, ch + this->m_Families, PairLess<double>());
	for (int Fam = 0; Fam < this->m_Families; Fam++)
		FamPrmu[Fam] = ch[Fam].dim;
	delete[]ch;
}

//Get a solution by perfoming NEHinsert to Fam
int Heu::NEHFam(vector<int> FamPrmu, vector<vector<int>> JobSeqinFam, vector<vector<int>> &FacFamSeq, vector<int> &FacSpan)
{
	//check FacFamSeq and FacSpan
	if (FacFamSeq.size() != FacSpan.size())
	{
		cout << "Error in NEHJob: FacFamSeq.size() != FacSpan.size()" << endl;
		exit(0);
	}
	if (!FacFamSeq.size())
	{
		cout << "Error in NEHJob: FacFamSeq.size()=0" << endl;
		exit(0);
	}

	vector<vector<int>> jCTime(this->m_Jobs), jSTime;
	for (int j = 0; j < this->m_Jobs; j++)
		jCTime[j].resize(this->m_Machines,0);
	jSTime = jCTime;
	vector<bool> Is_Calculated(FacFamSeq.size(), false); //whether jCTime and jSTime are calculated in the given Factory
	
	for (int Fam = 0; Fam < FamPrmu.size(); Fam++)
	{
		for (int Fac = 0; Fac < FacFamSeq.size(); Fac++) 
		{
			if (!Is_Calculated[Fac])
			{
				this->GetJCTime_Forward_InFactory(FacFamSeq[Fac], JobSeqinFam, jCTime);
				this->GetJSTime_Backward_InFactory(FacFamSeq[Fac], JobSeqinFam, jSTime);
				Is_Calculated[Fac] = true;
			}
		}

		int CurFam = FamPrmu[Fam];
		int bestFac = -1, bestPos = -1;
		int Span = this->FindBestPosToInsertFam(FacFamSeq, JobSeqinFam, jSTime, jCTime, CurFam, bestFac, bestPos);
		
		//Insert CurFam to bestPos at bestFac
		FacFamSeq[bestFac].insert(FacFamSeq[bestFac].begin() + bestPos, CurFam);
		FacSpan[bestFac] = Span;
		Is_Calculated[bestFac] = false;
	}
	return *max_element(FacSpan.begin(),FacSpan.end());
}

//Get a FamSeq in a factory by NEHinsert to Fam 20190420
int Heu::NEHFam(vector<int> FamPrmuInFac, vector<vector<int>> JobSeqinFam, vector<int> &FamSeqInFac, int &SpanInFac)
{
	vector<vector<int>> jCTime(this->m_Jobs), jSTime;
	for (int j = 0; j < this->m_Jobs; j++)
		jCTime[j].resize(this->m_Machines,0);
	jSTime = jCTime;

	for (int Fam = 0; Fam < FamPrmuInFac.size(); Fam++)
	{
		this->GetJCTime_Forward_InFactory(FamSeqInFac, JobSeqinFam, jCTime);
		this->GetJSTime_Backward_InFactory(FamSeqInFac, JobSeqinFam, jSTime);

		int CurFam = FamPrmuInFac[Fam];
		int bestFac = -1, bestPos = -1;
		SpanInFac = this->FindBestPosToInsertFam_InFactory(FamSeqInFac, JobSeqinFam, jSTime, jCTime, CurFam, bestPos);

		//Insert CurFam to bestPos at bestFac
		FamSeqInFac.insert(FamSeqInFac.begin() + bestPos, CurFam);
		
	}

	return SpanInFac;
}

//NEHinsert to a job prmutation;
int Heu::NEHJob(vector<int> JobSeq, vector<int> MachReadyTime, vector<int> &NewJobSeq)
{
	vector<vector<int>> jCTime(this->m_Jobs), jSTime;
	for (int j = 0; j < this->m_Jobs; j++)
		jCTime[j].resize(this->m_Machines);
	jSTime = jCTime;
	vector<int> MachReadyTimeR(this->m_Machines, 0);

	NewJobSeq.clear();
	int Span;
	for (int j = 0; j < JobSeq.size(); j++)
	{
		this->GetJCTime_Forward_InJobSeq(NewJobSeq, MachReadyTime, jCTime);
		this->GetJSTime_Backward_InJobSeq(NewJobSeq, MachReadyTimeR, jSTime);
		int CurJob = JobSeq[j];
		int bestPos = -1;
		Span = this->FindBestPosToInsertJob(NewJobSeq, jSTime, jCTime, MachReadyTime, MachReadyTimeR, CurJob, bestPos);
		NewJobSeq.insert(NewJobSeq.begin() + bestPos, CurJob);
	}
	
	return Span;
}

//Append Fam to the first available factory. NEH to job sequence in Fam; 20190421
int Heu::AppendFam_NEHJob(vector<int> FamPrmu, vector<vector<int>> &JobSeqinFam, vector<vector<int>> &FacFamSeq, vector<int> &FacSpan)
{
	FacFamSeq.clear();
	FacFamSeq.resize(this->m_Factories);
	FacSpan.clear();
	FacSpan.resize(this->m_Factories, 0);

	vector<vector<int>> MachReadyTime(this->m_Factories);
	for (int Fac = 0; Fac < MachReadyTime.size(); Fac++)
		MachReadyTime[Fac].resize(this->m_Machines, 0);
	vector<vector<int>> JCTime(this->m_Jobs);
	for (int j = 0; j < this->m_Jobs; j++)
		JCTime[j].resize(this->m_Machines, 0);

	for (int Fam = 0; Fam < FamPrmu.size(); Fam++) //schedule Fam one by one
	{
		int CurFam = FamPrmu[Fam];
		//Select the first availabe factory for CurFam
		int SelFac = 0, minFacSpan = MachReadyTime[0][this->m_Machines - 1];
		for (int Fac = 1; Fac < this->m_Factories; Fac++)
		{
			if (MachReadyTime[Fac][this->m_Machines - 1] < minFacSpan)
			{
				minFacSpan = MachReadyTime[Fac][this->m_Machines - 1];
				SelFac = Fac;
			}
		}

		//Schedule CurFam. Get MachReadyTime after setup
		if (FacFamSeq[SelFac].size() == 0)
		{
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTime[SelFac][m] = m_SetupTime[m][CurFam][CurFam];
		}
		else
		{
			int PreFam = FacFamSeq[SelFac][FacFamSeq[SelFac].size() - 1];
			for (int m = 0; m<this->m_Machines; m++)
				MachReadyTime[SelFac][m] += this->m_SetupTime[m][PreFam][CurFam];
		}

		//Obtain a Job sequence by NEH
		FacSpan[SelFac] = this->NEHJob(JobSeqinFam[CurFam], MachReadyTime[SelFac], JobSeqinFam[CurFam]);
		
		//Scheduling Jobs in CurFam
		FacFamSeq[SelFac].push_back(CurFam);
		for (int j = 0; j < JobSeqinFam[CurFam].size(); j++)
		{
			int CurJob = JobSeqinFam[CurFam][j];
			JCTime[CurJob][0] = MachReadyTime[SelFac][0] + m_JobOperPTime[CurJob][0];//on the first machine
			for (int m = 1; m < this->m_Machines; m++) //on the rest machine
				JCTime[CurJob][m] = max(JCTime[CurJob][m - 1], MachReadyTime[SelFac][m]) + m_JobOperPTime[CurJob][m];
			MachReadyTime[SelFac] = JCTime[CurJob];
		}

		if(FacSpan[SelFac]!= MachReadyTime[SelFac][this->m_Machines - 1]) cout<<"ERROR!"<<endl;
	}
	return *max_element(FacSpan.begin(), FacSpan.end());
}

//Try to append Fam to all factories and consider NEHinset to Jobs in Fam. Select the factory with lowest makespan. 20190423
int Heu::AppendFam_NEHJob_Include(vector<int> FamPrmu, vector<vector<int>> &JobSeqinFam, vector<vector<int>> &FacFamSeq, vector<int> &FacSpan)
{
	FacFamSeq.clear();
	FacFamSeq.resize(this->m_Factories);
	FacSpan.clear();
	FacSpan.resize(this->m_Factories, 0);

	vector<vector<int>> MachReadyTime(this->m_Factories);
	for (int Fac = 0; Fac < MachReadyTime.size(); Fac++)
		MachReadyTime[Fac].resize(this->m_Machines, 0);

	for (int Fam = 0; Fam < FamPrmu.size(); Fam++)
	{
		int CurFam = FamPrmu[Fam];
		int SelFac = 0, minFacSpan = INT_MAX, TempSpan;
		vector <int> TempJobSeq, BestJobSeq;
		vector<vector<int>> mIdle= MachReadyTime;
		for (int Fac = 0; Fac < this->m_Factories; Fac++) //find the Factory with lowest makespan after include CurFam
		{
			//Get mIdle after setup
			if (FacFamSeq[Fac].size() == 0)
			{
				for (int m = 0; m < this->m_Machines; m++)
					mIdle[Fac][m] = m_SetupTime[m][CurFam][CurFam];
			}
			else
			{
				int PreFam = FacFamSeq[Fac][FacFamSeq[Fac].size() - 1];
				for (int m = 0; m < this->m_Machines; m++)
					mIdle[Fac][m]= MachReadyTime[Fac][m] + this->m_SetupTime[m][PreFam][CurFam];
			}

			//NEH to Jobs in CurFam
			TempSpan = this->NEHJob(JobSeqinFam[CurFam], mIdle[Fac], TempJobSeq);
			if (TempSpan < minFacSpan) //record the best factory
			{
				minFacSpan = TempSpan;
				SelFac = Fac;
				BestJobSeq = TempJobSeq;
			}
		}

		//Schedule Jobs in CurFma at SelFac
		if (FacFamSeq[SelFac].size() == 0)
		{
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTime[SelFac][m] = m_SetupTime[m][CurFam][CurFam];
		}
		else
		{
			int PreFam = FacFamSeq[SelFac][FacFamSeq[SelFac].size() - 1];
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTime[SelFac][m] = MachReadyTime[SelFac][m] + this->m_SetupTime[m][PreFam][CurFam];
		}
		
		//Append CurFam to SelFac
		FacSpan[SelFac] = minFacSpan;
		FacFamSeq[SelFac].push_back(CurFam);
		JobSeqinFam[CurFam] = BestJobSeq;

		for (int j = 0; j < JobSeqinFam[CurFam].size(); j++)
		{
			int CurJob = JobSeqinFam[CurFam][j];
			MachReadyTime[SelFac][0] = MachReadyTime[SelFac][0] + m_JobOperPTime[CurJob][0];
			for (int m = 1; m < this->m_Machines; m++) 
				MachReadyTime[SelFac][m] = max(MachReadyTime[SelFac][m - 1], MachReadyTime[SelFac][m]) + m_JobOperPTime[CurJob][m];
		}

		if (FacSpan[SelFac] != MachReadyTime[SelFac][this->m_Machines - 1]) cout << "ERROR!" << endl;
	}
	return *max_element(FacSpan.begin(), FacSpan.end());
}

//HeuType=0: NEHFam in Fac; 1: NEHFam; 2: no NEH
int Heu::SimpleHeu_NEH(int HeuType, vector<vector<int>> &JobSeqinFam, vector<vector<int>> &FacFamSeq, vector<int> &FacSpan)//For coevoluation algorithm 
{
	this->GetJobTotalPTime();
	this->GetFamTotalPTime();
	this->GetFamAvgSetupTime();
	vector<int> JobSortFactorArray = { 0 };
	vector<int> JobSortMethodArray = { 0,1 };
	vector<int> FamSortFactorArray = { 0,1 };
	vector<int> FamSortMethodArray = { 0,1 };

	int ComType = HeuType / (JobSortFactorArray.size() * JobSortMethodArray.size() * FamSortFactorArray.size() * FamSortMethodArray.size());
	int TempIndex = HeuType % (JobSortFactorArray.size() * JobSortMethodArray.size() * FamSortFactorArray.size() * FamSortMethodArray.size());
	int JobSortFactor = TempIndex / (JobSortMethodArray.size() * FamSortFactorArray.size() * FamSortMethodArray.size());
	TempIndex = TempIndex % (JobSortMethodArray.size() * FamSortFactorArray.size() * FamSortMethodArray.size());
	int JobSortMethod = TempIndex / (FamSortFactorArray.size() * FamSortMethodArray.size());
	TempIndex = TempIndex % (FamSortFactorArray.size() * FamSortMethodArray.size());
	int FamSortFactor = TempIndex / FamSortMethodArray.size();
	TempIndex = TempIndex % FamSortMethodArray.size();
	int FamSortMethod = TempIndex;

	vector<int> FamPrmu;
	this->SortJobsinFam(JobSortFactorArray[JobSortFactor], JobSortMethodArray[JobSortMethod], JobSeqinFam);
	this->SortFam(FamSortFactorArray[FamSortFactor], FamSortMethodArray[FamSortMethod], FamPrmu);

	int Span;
	if (ComType == 0)
	{
		Span = this->GetSol(FamPrmu, JobSeqinFam, FacFamSeq, FacSpan);
		for (int Fac = 0; Fac < this->m_Factories; Fac++)
		{
			vector <int> newFacFamSeq;
			this->NEHFam(FacFamSeq[Fac], JobSeqinFam, newFacFamSeq, FacSpan[Fac]);
			FacFamSeq[Fac] = newFacFamSeq;
		}
		Span = *max_element(FacSpan.begin(), FacSpan.end());
	}
	else if (ComType == 1)
		Span = this->NEHFam(FamPrmu, JobSeqinFam, FacFamSeq, FacSpan);
	else
		Span = this->GetSol(FamPrmu, JobSeqinFam, FacFamSeq, FacSpan);
	return Span;
}



	
