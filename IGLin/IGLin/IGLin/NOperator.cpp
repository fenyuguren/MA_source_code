#include "stdafx.h"
#include "NOperator.h"

NOperator::NOperator()
{
}

NOperator::~NOperator()
{
}

// Select the first available factory to append Fam
int NOperator::GetSol(vector<int> FamPrmu, vector<vector<int>>JobSeqinFam, vector<vector<int>> &FacFamSeq, vector<int> &FacSpan)
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

	for (int Fam = 0; Fam < FamPrmu.size(); Fam++)
	{
		int CurFam = FamPrmu[Fam];
		int SelFac = -1, minFacSpan = INT_MAX;
		for (int Fac = 0; Fac < this->m_Factories; Fac++) //find the first available Factory
		{
			if (MachReadyTime[Fac][this->m_Machines - 1] < minFacSpan)
			{
				minFacSpan = MachReadyTime[Fac][this->m_Machines - 1];
				SelFac = Fac;
			}
		}

		if (FacFamSeq[SelFac].size() == 0)
		{
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTime[SelFac][m] = m_SetupTime[m][CurFam][CurFam];
		}
		else
		{
			int PreFam = FacFamSeq[SelFac][FacFamSeq[SelFac].size()-1];
			for (int m = 0; m<this->m_Machines; m++)
				MachReadyTime[SelFac][m] += this->m_SetupTime[m][PreFam][CurFam];
		}

		for (int j = 0; j < JobSeqinFam[CurFam].size(); j++)//Scheduling Jobs in CurFam
		{
			int CurJob = JobSeqinFam[CurFam][j];
			JCTime[CurJob][0] = MachReadyTime[SelFac][0] + m_JobOperPTime[CurJob][0];//on the first machine
			for (int m = 1; m < this->m_Machines; m++) //on the rest machine
				JCTime[CurJob][m] = max(JCTime[CurJob][m - 1], MachReadyTime[SelFac][m]) + m_JobOperPTime[CurJob][m];
			MachReadyTime[SelFac] = JCTime[CurJob];
		}
		FacSpan[SelFac] = MachReadyTime[SelFac][this->m_Machines - 1];
		FacFamSeq[SelFac].push_back(CurFam);
	}
	return *max_element(FacSpan.begin(), FacSpan.end());
}

//20190415 Select the factory with lowest makespan after the factory includes the appended Fam
int NOperator::GetSol_Include(vector<int> FamPrmu, vector<vector<int>>JobSeqinFam, vector<vector<int>> &FacFamSeq, vector<int> &FacSpan)
{
	FacFamSeq.clear();
	FacFamSeq.resize(this->m_Factories);
	FacSpan.clear();
	FacSpan.resize(this->m_Factories, 0);

	vector<vector<int>> FacMachReadyTime(this->m_Factories);
	for (int Fac = 0; Fac < FacMachReadyTime.size(); Fac++)
		FacMachReadyTime[Fac].resize(this->m_Machines, 0);
	vector<vector<int>> JCTime(this->m_Jobs);
	for (int j = 0; j < this->m_Jobs; j++)
		JCTime[j].resize(this->m_Machines, 0);

	// Assign one job to each Factory
	int MaxFac = min(this->m_Factories, FamPrmu.size());
	for (int Fac = 0; Fac < MaxFac; Fac++)
	{
		int CurFam = FamPrmu[Fac];
		for (int m = 0; m < this->m_Machines; m++) //Family setup
			FacMachReadyTime[Fac][m] = this->m_SetupTime[m][CurFam][CurFam];
		for (int j = 0; j < JobSeqinFam[CurFam].size(); j++) // Process jobs one by one in the Family
		{
			int CurJob = JobSeqinFam[CurFam][j];
			JCTime[CurJob][0] = FacMachReadyTime[Fac][0] + this->m_JobOperPTime[CurJob][0];
			for(int m=1;m<this->m_Machines;m++)
				JCTime[CurJob][m] = max(FacMachReadyTime[Fac][m],JCTime[CurJob][m-1])+ this->m_JobOperPTime[CurJob][m];
			FacMachReadyTime[Fac] = JCTime[CurJob];			
		}
		FacFamSeq[Fac].push_back(CurFam);
		FacSpan[Fac] = FacMachReadyTime[Fac][this->m_Machines - 1];
	}

	//Assign the remaining Jobs
	vector<vector<int>> TempFacMachReadyTime = FacMachReadyTime;
	for (int Fam = this->m_Factories; Fam < FamPrmu.size(); Fam++)
	{
		int CurFam = FamPrmu[Fam];
		int SelFac = -1, minFacSpan = INT_MAX;
		for (int Fac = 0; Fac < this->m_Factories; Fac++) //find the Factory that can complete the Family earliest
		{
			int PreFam = FacFamSeq[Fac][FacFamSeq[Fac].size() - 1];
			int CurFam = FamPrmu[Fam];
			for (int m = 0; m < this->m_Machines; m++) //Family setup time
				TempFacMachReadyTime[Fac][m] = FacMachReadyTime[Fac][m] + this->m_SetupTime[m][PreFam][CurFam];
			for (int j = 0; j < JobSeqinFam[CurFam].size(); j++) //Process Jobs one by one in the Family
			{
				int CurJob = JobSeqinFam[CurFam][j];
				JCTime[CurJob][0]= TempFacMachReadyTime[Fac][0] + this->m_JobOperPTime[CurJob][0];
				for (int m = 1; m<this->m_Machines; m++)
					JCTime[CurJob][m] = max(TempFacMachReadyTime[Fac][m], JCTime[CurJob][m - 1]) + this->m_JobOperPTime[CurJob][m];
				TempFacMachReadyTime[Fac] = JCTime[CurJob];
			}

			if (TempFacMachReadyTime[Fac][this->m_Machines - 1] < minFacSpan) //Record the best Factory
			{
				minFacSpan = TempFacMachReadyTime[Fac][this->m_Machines - 1];
				SelFac = Fac;
			}
		}

		//Assign CurFam to the selected factory
		FacMachReadyTime[SelFac] = TempFacMachReadyTime[SelFac];
		FacFamSeq[SelFac].push_back(CurFam);
		FacSpan[SelFac] = minFacSpan;
	}
	return *max_element(FacSpan.begin(), FacSpan.end());
}

int NOperator::GetJCTime_Forward(vector <int> FamSeq, vector <vector <int>> JobSeqinFam, vector<vector<int>> &JCTime, int &FacSpan)//Forward pass calculation
{

	FacSpan = GetJCTime_Forward_InFactory(FamSeq, JobSeqinFam, JCTime);
	return FacSpan;
}

int NOperator::GetJSTime_Backward(vector <vector <int>> FamSeq, vector <vector <int>> JobSeqinFam, vector<vector<int>> &JSTime, vector<int> &FacSpan) //Backward pass calculation
{
	for (int Fac = 0; Fac < FamSeq.size(); Fac++)
		FacSpan[Fac] = this->GetJSTime_Backward_InFactory(FamSeq[Fac], JobSeqinFam, JSTime);
	return *max_element(FacSpan.begin(), FacSpan.end());
}

int NOperator::GetJCTime_Forward_InFactory(vector <int> FamSeqInFac, vector <vector <int>> JobSeqinFam, vector<vector<int>> &JCTime)
{
	vector<int> MachReadyTime(this->m_Machines, 0);
	for (int Fam = 0; Fam < FamSeqInFac.size(); Fam++)
	{
		int CurFam = FamSeqInFac[Fam];
		if (Fam == 0)//the first group of jobs
		{
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTime[m] = this->m_SetupTime[m][CurFam][CurFam];
		}
		else //from the second group of jobs to the end;
		{
			int PreFam = FamSeqInFac[Fam - 1];
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTime[m] += this->m_SetupTime[m][PreFam][CurFam];
		}

		for (int j = 0; j < JobSeqinFam[CurFam].size(); j++)//Scheduling Jobs in CurFam
		{
			int CurJob = JobSeqinFam[CurFam][j];
			JCTime[CurJob][0] = MachReadyTime[0] + m_JobOperPTime[CurJob][0];//on the first machine
			for (int m = 1; m < this->m_Machines; m++) //on the rest machine
				JCTime[CurJob][m] = max(JCTime[CurJob][m - 1], MachReadyTime[m]) + m_JobOperPTime[CurJob][m];
			MachReadyTime = JCTime[CurJob];
		}
	}

	return MachReadyTime[this->m_Machines - 1];
}

int NOperator::GetJSTime_Backward_InFactory(vector <int> FamSeqInFac, vector <vector <int>> JobSeqinFam, vector<vector<int>> &JSTime)
{
	vector<int> MachReadyTime(this->m_Machines, 0);
	for (int Fam = FamSeqInFac.size() - 1; Fam >= 0; Fam--)
	{
		int CurFam = FamSeqInFac[Fam];
		if (Fam < FamSeqInFac.size() - 1)//Not the last group of jobs			
		{
			int NextFam = FamSeqInFac[Fam + 1];
			for (int m = this->m_Machines - 1; m >= 0; m--)
				MachReadyTime[m] += this->m_SetupTime[m][CurFam][NextFam];
		}

		for (int j = JobSeqinFam[CurFam].size() - 1; j >= 0; j--)//Scheduling Jobs in CurFam
		{
			int CurJob = JobSeqinFam[CurFam][j];
			JSTime[CurJob][this->m_Machines - 1] = MachReadyTime[this->m_Machines - 1] + m_JobOperPTime[CurJob][this->m_Machines - 1];//on the last machine
			for (int m = this->m_Machines - 2; m >= 0; m--) //on the rest machine
				JSTime[CurJob][m] = max(JSTime[CurJob][m + 1], MachReadyTime[m]) + m_JobOperPTime[CurJob][m];
			MachReadyTime = JSTime[CurJob];
		}

		if (Fam == 0) // the first Family
		{
			for (int m = this->m_Machines - 1; m >= 0; m--)
				MachReadyTime[m] += this->m_SetupTime[m][CurFam][CurFam];
		}
	}

	int Span = *max_element(MachReadyTime.begin(), MachReadyTime.end());

	return Span;
}

void NOperator::GetJCTime_Forward_InJobSeq(vector<int> JobSeq, vector<int> MachReadyTime, vector<vector<int>> &JCTime)
{
	for (int j = 0; j < JobSeq.size(); j++)
	{
		int CurJob = JobSeq[j];
		JCTime[CurJob][0] = MachReadyTime[0] + m_JobOperPTime[CurJob][0];//on the first machine
		for (int m = 1; m < this->m_Machines; m++) //on the rest machine
			JCTime[CurJob][m] = max(JCTime[CurJob][m - 1], MachReadyTime[m]) + m_JobOperPTime[CurJob][m];
		MachReadyTime = JCTime[CurJob];
	}
}

void NOperator::GetJSTime_Backward_InJobSeq(vector<int> JobSeq, vector<int> MachReadyTimeR, vector<vector<int>> &JSTime)
{
	for (int j = JobSeq.size() - 1; j >= 0; j--)
	{
		int CurJob = JobSeq[j];
		JSTime[CurJob][this->m_Machines - 1] = MachReadyTimeR[this->m_Machines - 1] + m_JobOperPTime[CurJob][this->m_Machines - 1];//on the last machine
		for (int m = this->m_Machines - 2; m >= 0; m--) //on the rest machine
			JSTime[CurJob][m] = max(JSTime[CurJob][m + 1], MachReadyTimeR[m]) + m_JobOperPTime[CurJob][m];
		MachReadyTimeR = JSTime[CurJob];
	}
}

int NOperator::GetSpan(vector <int> FamSeq, vector <vector <int>> JobSeqinFam)
{
	vector<vector<int>> JCTime(this->m_Jobs);
	for (int j = 0; j < JCTime.size(); j++)
		JCTime[j].resize(this->m_Machines, 0);
	int mSpan = GetJCTime_Forward_InFactory(FamSeq, JobSeqinFam, JCTime);
	return mSpan;
}
//20190524
int NOperator::GetSpan(vector <int> FamSeq, vector <vector <int>> JobSeqinFam, int &FacSpan)
{
	vector<vector<int>> JCTime(this->m_Jobs);
	for (int j = 0; j < JCTime.size(); j++)
		JCTime[j].resize(this->m_Machines, 0);
	FacSpan = this->GetJCTime_Forward(FamSeq, JobSeqinFam, JCTime, FacSpan);
	return FacSpan;
}
//20190810
int NOperator::GetSpanInFac(vector <int> FamSeqInFac, vector <vector <int>> JobSeqinFam)
{
	vector<vector<int>> JCTime(this->m_Jobs);
	for (int j = 0; j < JCTime.size(); j++)
		JCTime[j].resize(this->m_Machines, 0);
	int mSpan = this->GetJCTime_Forward_InFactory(FamSeqInFac, JobSeqinFam, JCTime);
	return mSpan;
}

//20190421
int NOperator::FindBestPosToInsertJob(vector<int> JobSeq, vector<vector<int>> JSTime, vector<vector<int>> JCTime, vector<int> MachReadyTime, vector<int> MachReadyTimeR, int InsJob, int &bestPos)
{
	int minSpan = INT_MAX;
	for (int Pos = 0; Pos <= JobSeq.size(); Pos++)
	{
		vector<int> MachSpan(this->m_Machines, 0);
		vector<int> mIdle(this->m_Machines, 0);//Get machine release time;
		if (Pos == 0) //InsJob is inserted at the first position
			mIdle = MachReadyTime;
		else //InsJob is inserted at the other positions
		{
			int PreJob = JobSeq[Pos - 1];
			mIdle = JCTime[PreJob];
		}

		//Schedule InsJob
		mIdle[0] = mIdle[0] + this->m_JobOperPTime[InsJob][0];
		for (int m = 1; m < this->m_Machines; m++)
			mIdle[m] = max(mIdle[m], mIdle[m - 1]) + this->m_JobOperPTime[InsJob][m];

		//Compute Machine Span
		if (Pos < JobSeq.size()) // The inserted position is not the last one
		{
			int NextJob = JobSeq[Pos];
			for (int m = 0; m < this->m_Machines; m++)
				MachSpan[m] = mIdle[m] + JSTime[NextJob][m];
		}
		else // The inserted position is the last one
		{
			for (int m = 0; m < this->m_Machines; m++)
				MachSpan[m] = mIdle[m] + MachReadyTimeR[m];
		}

		int Span = *max_element(MachSpan.begin(), MachSpan.end());
		if (Span < minSpan)
		{
			minSpan = Span;
			bestPos = Pos;
		}
	}
	return minSpan;
}

int NOperator::FindBestPosToInsertFam_InFactory(vector <int> FamSeqInFac, vector <vector <int>> JobSeqinFam, vector<vector<int>> JSTime, vector<vector<int>> JCTime, int InsFam, int &bestPos)
{
	int minSpan = INT_MAX;
	for (int Pos = 0; Pos <= FamSeqInFac.size(); Pos++)
	{
		vector<int> MachReadyTime(this->m_Machines, 0);
		if (Pos == 0)//Insert at Postion 0
		{
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTime[m] = this->m_SetupTime[m][InsFam][InsFam];
		}
		else //Insert at other Postions
		{
			int PreFam = FamSeqInFac[Pos - 1];
			int LastJobinPreFam = JobSeqinFam[PreFam][JobSeqinFam[PreFam].size() - 1];
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTime[m] = JCTime[LastJobinPreFam][m] + this->m_SetupTime[m][PreFam][InsFam];
		}

		//Scheduling jobs in InsFam
		for (int j = 0; j < JobSeqinFam[InsFam].size(); j++)
		{
			int CurJob = JobSeqinFam[InsFam][j];
			MachReadyTime[0] += this->m_JobOperPTime[CurJob][0];
			for (int m = 1; m < this->m_Machines; m++)
				MachReadyTime[m] = max(MachReadyTime[m - 1], MachReadyTime[m]) + this->m_JobOperPTime[CurJob][m];
		}

		vector<int> MachSpan(this->m_Machines);
		if (Pos < FamSeqInFac.size()) //Compute makespan;
		{
			int NextFam = FamSeqInFac[Pos];
			int FirstJobinNextFam = JobSeqinFam[NextFam][0];
			for (int m = 0; m < this->m_Machines; m++)
				MachSpan[m] = MachReadyTime[m] + this->m_SetupTime[m][InsFam][NextFam] + JSTime[FirstJobinNextFam][m];
		}
		else
			MachSpan = MachReadyTime;
		int Span = *max_element(MachSpan.begin(), MachSpan.end());
		if (Span < minSpan)
		{
			minSpan = Span;
			bestPos = Pos;
		}
	}
	return minSpan;
}

//20190818
int NOperator::FindBestPosToInsertFam_InFactory_OptJobSeq(vector <int> FamSeqInFac, vector <vector <int>> JobSeqinFam, vector<vector<int>> JSTime, vector<vector<int>> JCTime, int InsFam, int &bestPos, vector<int> &bestJobSeq)
{
	int minSpan = INT_MAX;
	bestJobSeq = JobSeqinFam[InsFam];
	for (int Pos = 0; Pos <= FamSeqInFac.size(); Pos++)
	{
		vector<int> MachReadyTime(this->m_Machines, 0), MachReadyTimeR(this->m_Machines, 0),mIdleTime(this->m_Machines,0);
		//Get MachReadyTime
		if (Pos == 0)//Insert at Postion 0
		{
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTime[m] = this->m_SetupTime[m][InsFam][InsFam];
		}
		else //Insert at other Postions
		{
			int PreFam = FamSeqInFac[Pos - 1];
			int LastJobinPreFam = JobSeqinFam[PreFam][JobSeqinFam[PreFam].size() - 1];
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTime[m] = JCTime[LastJobinPreFam][m] + this->m_SetupTime[m][PreFam][InsFam];
		}

		//Get mIdleTime: Scheduling jobs in InsFam
		mIdleTime = MachReadyTime;
		for (int j = 0; j < JobSeqinFam[InsFam].size(); j++)
		{
			int CurJob = JobSeqinFam[InsFam][j];
			mIdleTime[0] += this->m_JobOperPTime[CurJob][0];
			for (int m = 1; m < this->m_Machines; m++)
				mIdleTime[m] = max(mIdleTime[m - 1], mIdleTime[m]) + this->m_JobOperPTime[CurJob][m];
		}

		//Get MachReadyTimeR;
		if (Pos < FamSeqInFac.size())
		{
			int NextFam = FamSeqInFac[Pos];
			int FirstJobinNextFam = JobSeqinFam[NextFam][0];
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTimeR[m] = this->m_SetupTime[m][InsFam][NextFam] + JSTime[FirstJobinNextFam][m];
		}

		vector<int> MachSpan(this->m_Machines);
		for (int m = 0; m < this->m_Machines; m++)
			MachSpan[m] = mIdleTime[m] + MachReadyTimeR[m];
		int Span = *max_element(MachSpan.begin(), MachSpan.end());

		//Optimize JobSeq for InsFam
		vector<int> TempJobSeq = JobSeqinFam[InsFam];
		LocalSearch_JobsInFam(TempJobSeq, Span, MachReadyTime, MachReadyTimeR);
		if (Span < minSpan)
		{
			minSpan = Span;
			bestPos = Pos;
			bestJobSeq = TempJobSeq;
		}
	}
	return minSpan;
}

int NOperator::FindBestPosToInsertFam(vector <vector <int>> FamSeq, vector <vector <int>> JobSeqinFam, vector<vector<int>> JSTime, vector<vector<int>> JCTime, int InsFam, int &bestFac, int &bestPos)
{
	int minSpan = INT_MAX;
	for (int Fac = 0; Fac < FamSeq.size(); Fac++)
	{
		int Pos;
		int Span = FindBestPosToInsertFam_InFactory(FamSeq[Fac], JobSeqinFam, JSTime, JCTime, InsFam, Pos);
		if (Span < minSpan)
		{
			minSpan = Span;
			bestFac = Fac;
			bestPos = Pos;
		}
	}
	return minSpan;
}

void NOperator::CheckSol(vector <int> FamSeq, vector <vector <int>> JobSeqinFam, int Span)
{
	if (!FamSeq.size())
	{
		cout << "Error in CheckSol: FamSeq.size()=0" << endl;
		getchar();
		exit(0);
	}

	//Check Families
	vector <bool> bFamArray(this->m_Families, false);

	for (int Fam = 0; Fam < FamSeq.size(); Fam++)
		bFamArray[FamSeq[Fam]] = true;
	for (int i = 0; i < this->m_Families; i++)
	{
		if (!bFamArray[i])
		{
			cout << "Family " << i << " is lost!" << endl;
			getchar();
			exit(0);
		}
	}

	//Check Jobs
	vector <bool> bJobArray(this->m_Jobs, false);
	for (int Fam = 0; Fam < JobSeqinFam.size(); Fam++)
	{
		for (int j = 0; j < JobSeqinFam[Fam].size(); j++)
		{
			int Job = JobSeqinFam[Fam][j];
			bJobArray[Job] = true;
		}
	}

	for (int i = 0; i < this->m_Jobs; i++)
	{
		if (!bJobArray[i])
		{
			cout << "Job " << i << " is lost!" << endl;
			getchar();
			exit(0);
		}
	}

	//Check Jobs in each Family
	for (int Fam = 0; Fam < this->m_JobSeqinFam.size(); Fam++)
	{
		for (int j = 0; j < this->m_JobSeqinFam[Fam].size(); j++)
		{
			int Job = this->m_JobSeqinFam[Fam][j];
			if (JobSeqinFam[Fam].end() == find(JobSeqinFam[Fam].begin(), JobSeqinFam[Fam].end(), Job))
			{
				cout << "Job is not found in Family:\t" << Fam << endl;
				getchar();
				exit(0);
			}
		}
	}

	//----- Check makespan---------
	int TSpan = this->GetSpan(FamSeq, JobSeqinFam);
	if (TSpan != Span)
	{
		cout << "Span is Erro!" << Span << "\t" << TSpan << endl;
		getchar();
		exit(0);
	}
	cout << "ÕýÈ·£¡" << endl;
}
void NOperator::GetRPI(vector<vector<int>> SpanArray, vector<vector<double>> &RPIArray)
{
	//Get best makespan
	vector<int> minSpanArray(SpanArray.size());
	for (int ins = 0; ins < minSpanArray.size(); ins++)
		minSpanArray[ins] = *min_element(SpanArray[ins].begin(), SpanArray[ins].end());
	
	//Get RPI for each Method
	for (int ins = 0; ins < RPIArray.size(); ins++)
		for (int Method = 0; Method < RPIArray[ins].size(); Method++)
			RPIArray[ins][Method] = (SpanArray[ins][Method] - minSpanArray[ins])*100.0 / minSpanArray[ins];

	//Get AvgRPI 
	vector<double> AvgRPI(RPIArray[0].size(), 0);
	for (int Method = 0; Method < AvgRPI.size(); Method++)
		for (int ins = 0; ins < RPIArray.size(); ins++)
			AvgRPI[Method] += RPIArray[ins][Method];
	for (int Method = 0; Method < AvgRPI.size(); Method++)
		AvgRPI[Method] /= RPIArray.size();
	for (int Method = 0; Method < AvgRPI.size(); Method++)
		cout << AvgRPI[Method] << "\t";
}

//20190425-26
void NOperator::LocalSearch_JobsInFam(vector<int> &JobSeq, int &Span, vector<int> MachReadyTime, vector<int> MachReadyTimeR)
{
	vector<int> SeqForExtracting = JobSeq;
	random_shuffle(SeqForExtracting.begin(), SeqForExtracting.end());

	vector<vector<int>> JSTime(this->m_Jobs), JCTime;
	for (int j = 0; j < JSTime.size(); j++)
		JSTime[j].resize(this->m_Machines);
	JCTime = JSTime;
	
	int nCnt = 0, CurPos = 0, bestPos;
	while (nCnt < JobSeq.size())
	{
		// Extract a job from JobSeq;
		CurPos = CurPos % SeqForExtracting.size();
		int CurJob = SeqForExtracting[CurPos];		
		vector<int>::iterator it = find(JobSeq.begin(), JobSeq.end(), CurJob);
		int OrgPos = it - JobSeq.begin();
		JobSeq.erase(it);

		//Insert the job to the best position
		GetJCTime_Forward_InJobSeq(JobSeq, MachReadyTime, JCTime);
		GetJSTime_Backward_InJobSeq(JobSeq, MachReadyTimeR, JSTime);
		int TempSpan = this->FindBestPosToInsertJob(JobSeq, JSTime, JCTime, MachReadyTime, MachReadyTimeR, CurJob, bestPos);
		if (TempSpan < Span)
		{
			JobSeq.insert(JobSeq.begin() + bestPos, CurJob);
			Span = TempSpan;
			nCnt = 0;
		}
		else //restore ogrinal JobSeq
		{
			JobSeq.insert(JobSeq.begin() + OrgPos, CurJob);
			nCnt++;
		}
		CurPos++;
	}	
}

//20190425-26 Perform LocalSearch_Job to families in a factory one by one from start to end
void NOperator::LocalSearch_JobsInFac(vector <int> FamSeqInFac, vector <vector <int>> &JobSeqinFam, int &Span)
{
	vector<vector<int>> JSTime(this->m_Jobs), JCTime;
	for (int j = 0; j < JSTime.size(); j++)
		JSTime[j].resize(this->m_Machines);
	JCTime = JSTime;

	vector<int> MachReadyTime(this->m_Machines), MachReadyTimeR(this->m_Machines);
	GetJSTime_Backward_InFactory(FamSeqInFac, JobSeqinFam, JSTime);
	for (int Fam = 0; Fam < FamSeqInFac.size(); Fam++)
	{
		int CurFam = FamSeqInFac[Fam];
		if (Fam == 0)
		{
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTime[m] = this->m_SetupTime[m][CurFam][CurFam];
		}
		else
		{
			int LastFam = FamSeqInFac[Fam - 1];
			GetJCTime_Forward_InJobSeq(JobSeqinFam[LastFam], MachReadyTime, JCTime);
			int LastJob = *(JobSeqinFam[LastFam].end() - 1);
			MachReadyTime = JCTime[LastJob];
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTime[m] += this->m_SetupTime[m][LastFam][CurFam];
		}

		if (Fam < FamSeqInFac.size() - 1)
		{
			int NextFam = FamSeqInFac[Fam + 1];
			int NextJob = JobSeqinFam[NextFam][0];
			MachReadyTimeR = JSTime[NextJob];
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTimeR[m] += this->m_SetupTime[m][CurFam][NextFam];
		}
		else
		{
			for (int m = 0; m < this->m_Machines; m++)
				MachReadyTimeR[m] = 0;
		}
		this->LocalSearch_JobsInFam(JobSeqinFam[CurFam], Span, MachReadyTime, MachReadyTimeR);
	}
}

//20190817 
int NOperator::LocalSearch_Jobs(vector<vector <int>> FamSeqInFac, vector <vector <int>> &JobSeqinFam, vector<int> &FacSpan)
{
	for (int Fac = 0; Fac < FamSeqInFac.size();Fac++)
		this->LocalSearch_JobsInFac(FamSeqInFac[Fac], JobSeqinFam, FacSpan[Fac]);
	return *max_element(FacSpan.begin(), FacSpan.end());
}

//20190426
void NOperator::LocalSearch_FamsInFac(vector <int> &FamSeqInFac, vector <vector <int>> JobSeqinFam, int &Span)
{
	vector<int> SeqForExtracting = FamSeqInFac;
	random_shuffle(SeqForExtracting.begin(), SeqForExtracting.end());

	vector<vector<int>> JSTime(this->m_Jobs), JCTime;
	for (int j = 0; j < JSTime.size(); j++)
		JSTime[j].resize(this->m_Machines);
	JCTime = JSTime;

	int nCnt = 0, CurPos = 0, bestPos;
	while (nCnt < FamSeqInFac.size())
	{
		// Extract a Fam from FamSeqInFac;
		CurPos = CurPos % SeqForExtracting.size();
		int CurFam = SeqForExtracting[CurPos];
		vector<int>::iterator it = find(FamSeqInFac.begin(), FamSeqInFac.end(), CurFam);
		int OrgPos = it - FamSeqInFac.begin();
		FamSeqInFac.erase(it);

		//Insert the Fam to the best position
		this->GetJCTime_Forward_InFactory(FamSeqInFac, JobSeqinFam, JCTime);
		this->GetJSTime_Backward_InFactory(FamSeqInFac, JobSeqinFam, JSTime);
		int TempSpan = this->FindBestPosToInsertFam_InFactory(FamSeqInFac, JobSeqinFam, JSTime, JCTime, CurFam, bestPos);

		if (TempSpan < Span)
		{
			Span = TempSpan;
			FamSeqInFac.insert(FamSeqInFac.begin() + bestPos, CurFam);
			nCnt = 0;
		}
		else
		{
			FamSeqInFac.insert(FamSeqInFac.begin() + OrgPos, CurFam);
			nCnt++;
		}
		CurPos++;
	}
}

//20200619
void NOperator::LocalSearch_FamsInFac_OptJobSeq(vector <int> &FamSeqInFac, vector <vector <int>> &JobSeqinFam, int &Span)
{
	vector<int> SeqForExtracting = FamSeqInFac;
	random_shuffle(SeqForExtracting.begin(), SeqForExtracting.end());

	vector<vector<int>> JSTime(this->m_Jobs), JCTime;
	for (int j = 0; j < JSTime.size(); j++)
		JSTime[j].resize(this->m_Machines);
	JCTime = JSTime;

	int nCnt = 0, CurPos = 0, bestPos;
	while (nCnt < FamSeqInFac.size())
	{
		// Extract a Fam from FamSeqInFac;
		CurPos = CurPos % SeqForExtracting.size();
		int CurFam = SeqForExtracting[CurPos];
		vector<int>::iterator it = find(FamSeqInFac.begin(), FamSeqInFac.end(), CurFam);
		int OrgPos = it - FamSeqInFac.begin();
		FamSeqInFac.erase(it);

		//Insert the Fam to the best position
		this->GetJCTime_Forward_InFactory(FamSeqInFac, JobSeqinFam, JCTime);
		this->GetJSTime_Backward_InFactory(FamSeqInFac, JobSeqinFam, JSTime);
		vector<int> CurJobSeq;
		int TempSpan = this->FindBestPosToInsertFam_InFactory_OptJobSeq(FamSeqInFac, JobSeqinFam, JSTime, JCTime, CurFam, bestPos, CurJobSeq);

		if (TempSpan < Span)
		{
			Span = TempSpan;
			FamSeqInFac.insert(FamSeqInFac.begin() + bestPos, CurFam);
			JobSeqinFam[CurFam] = CurJobSeq;
			nCnt = 0;
		}
		else
		{
			FamSeqInFac.insert(FamSeqInFac.begin() + OrgPos, CurFam);
			nCnt++;
		}
		CurPos++;
	}
}

//20190810-13: Extract a fam from a random factory and insert it to all the factories
int NOperator::LocalSearch_Fams(vector<vector <int>> &FamSeq, vector<vector <int>> RefFamSeq, vector <vector <int>> JobSeqinFam, vector<int> &FacSpan, int &k)
{
	vector<vector<int>> JSTime(this->m_Jobs), JCTime;
	for (int j = 0; j < JSTime.size(); j++)
		JSTime[j].resize(this->m_Machines);
	JCTime = JSTime;
	vector <bool> bComputed(this->m_Factories, false);
	int Span = *max_element(FacSpan.begin(), FacSpan.end());
	
	vector<int> newFacSpan = FacSpan;	
	int newSpan = Span;
	int nCnt = 0;
	while (nCnt < this->m_Families)
	{		
		//Find the extracted family according to RefFam
		int Fac = 0, Pos = k;
		while (RefFamSeq[Fac].size() <= Pos)
		{
			Pos -= RefFamSeq[Fac].size();
			Fac++;
		}		
		int CurFam = RefFamSeq[Fac][Pos];

		//Find CurFam in newFamSeq and erase it, recompute orgFac.
		int orgFac = -1,orgPos = 0;
		vector<int>::iterator it;
		do {
			orgFac++;
			it = find(FamSeq[orgFac].begin(), FamSeq[orgFac].end(), CurFam);
		} while (it == FamSeq[orgFac].end());
		orgPos = it - FamSeq[orgFac].begin();
		FamSeq[orgFac].erase(it);
		newFacSpan[orgFac] = this->GetJCTime_Forward_InFactory(FamSeq[orgFac], JobSeqinFam, JCTime);
		this->GetJSTime_Backward_InFactory(FamSeq[orgFac], JobSeqinFam, JSTime);
		bComputed[orgFac] = true;

		//Find the best position for CurFam in FamSeq
		int BestPos, BestPosInFac, bestFac, bestSpan = INT_MAX;
		for (Fac = 0; Fac < this->m_Factories; Fac++)
		{
			if (!bComputed[Fac])
			{
				newFacSpan[Fac] = this->GetJCTime_Forward_InFactory(FamSeq[Fac], JobSeqinFam, JCTime);
				this->GetJSTime_Backward_InFactory(FamSeq[Fac], JobSeqinFam, JSTime);
				bComputed[Fac] = true;
			}

			int TempSpan = this->FindBestPosToInsertFam_InFactory(FamSeq[Fac], JobSeqinFam, JSTime, JCTime, CurFam, BestPosInFac);
			if (TempSpan < bestSpan)
			{
				bestSpan = TempSpan;
				BestPos = BestPosInFac;
				bestFac = Fac;
			}
		}

		//Insert CurFam to BestPos
		newFacSpan[bestFac] = bestSpan;
		newSpan= *max_element(newFacSpan.begin(), newFacSpan.end());
		if (newSpan < Span)
		{
			FamSeq[bestFac].insert(FamSeq[bestFac].begin() + BestPos, CurFam);
			FacSpan = newFacSpan;
			Span = newSpan;
			nCnt = 0;
			bComputed[bestFac] = false;	
		}
		else //Insert CurFam to orgPos
		{
			FamSeq[orgFac].insert(FamSeq[orgFac].begin() + orgPos, CurFam);
			bComputed[orgFac] = false;
			newFacSpan = FacSpan;
			newSpan = Span;
			nCnt++;
		}
			
		k++;
		k %= this->m_Families;
	}
	return Span;
}




//20190518
void NOperator::SwapFaminFac(vector <int> &FamSeqInFac)
{
	if (FamSeqInFac.size() < 2) return;
	int pt1 = rand() % FamSeqInFac.size();
	int pt2;
	do {
		pt2= rand() % FamSeqInFac.size();
	} while (pt1 == pt2);

	int Fam = FamSeqInFac[pt1];
	FamSeqInFac[pt1] = FamSeqInFac[pt2];
	FamSeqInFac[pt2] = Fam;
}

//20190518
void NOperator::SwapFamBetweenFacs(vector<vector <int>> &FamSeq)
{
	int F1 = rand() % this->m_Factories;
	int F2;
	do
	{
		F2= rand() % this->m_Factories;
	} while (F1 == F2);

	if (FamSeq[F1].size() && FamSeq[F2].size())
	{
		int pt1 = rand() % FamSeq[F1].size();
		int pt2 = rand() % FamSeq[F2].size();
		int Fam = FamSeq[F1][pt1];
		FamSeq[F1][pt1] = FamSeq[F2][pt2];
		FamSeq[F2][pt2] = Fam;
	}
	else if(FamSeq[F1].size())
	{
		int pt1 = rand() % FamSeq[F1].size();
		int Fam = FamSeq[F1][pt1];
		FamSeq[F2].push_back(Fam);
		FamSeq[F1].erase(FamSeq[F1].begin() + pt1);
	}
	else if(FamSeq[F2].size())
	{
		int pt1 = rand() % FamSeq[F2].size();
		int Fam = FamSeq[F2][pt1];
		FamSeq[F1].push_back(Fam);
		FamSeq[F2].erase(FamSeq[F2].begin() + pt1);
	}
}

//20190518
void NOperator::SwapFam(vector<vector <int>> &FamSeq)
{
	if (rand() % 2)
		this->SwapFamBetweenFacs(FamSeq);
	else
		this->SwapFaminFac(FamSeq[rand() % this->m_Factories]);
}

void NOperator::ShiftFam(vector<vector <int>> &FamSeq)
{
	int r = rand() % this->m_Factories;
	if (FamSeq[r].size() > 2)
	{
		int pt1 = rand() % FamSeq[r].size();
		int pt2;
		do {
			pt2 = rand() % FamSeq[r].size();
		} while (pt1 == pt2);

		int Fam = FamSeq[r][pt1];
		FamSeq[r].erase(FamSeq[r].begin() + pt1);
		FamSeq[r].insert(FamSeq[r].begin() + pt2, Fam);
	}
	else if (FamSeq[r].size() == 2)
	{
		int Fam = FamSeq[r][0];
		FamSeq[r][0] = FamSeq[r][1];
		FamSeq[r][1] = Fam;
	}
}

//20190519
void NOperator::SwapJob(vector<vector<int>> &JobSeqinFam)
{
	int r = rand() % this->m_Families;
	if (JobSeqinFam[r].size() > 2)
	{
		int pt1 = rand() % JobSeqinFam[r].size();
		int pt2;
		do {
			pt2 = rand() % JobSeqinFam[r].size();
		} while (pt1 == pt2);

		int Job = JobSeqinFam[r][pt1];
		JobSeqinFam[r][pt1]= JobSeqinFam[r][pt2];
		JobSeqinFam[r][pt2] = Job;
	}
	else if (JobSeqinFam[r].size() == 2)
	{
		int Job = JobSeqinFam[r][0];
		JobSeqinFam[r][0] = JobSeqinFam[r][1];
		JobSeqinFam[r][1] = Job;
	}
}

void NOperator::ShiftJob(vector<vector<int>> &JobSeqinFam)
{
	int r = rand() % this->m_Families;
	if (JobSeqinFam[r].size() > 2)
	{
		int pt1 = rand() % JobSeqinFam[r].size();
		int pt2;
		do {
			pt2 = rand() % JobSeqinFam[r].size();
		} while (pt1 == pt2);

		int Job = JobSeqinFam[r][pt1];
		JobSeqinFam[r].erase(JobSeqinFam[r].begin() + pt1);
		JobSeqinFam[r].insert(JobSeqinFam[r].begin() + pt2,Job);
	}
	else if (JobSeqinFam[r].size() == 2)
	{
		int Job = JobSeqinFam[r][0];
		JobSeqinFam[r][0] = JobSeqinFam[r][1];
		JobSeqinFam[r][1] = Job;
	}
}
void NOperator::FamInsert(vector <int> &FamSeq, vector <vector <int>> JobSeqinFam, int &Span)
{
	vector<vector<int>> JSTime(this->m_Jobs), JCTime;
	for (int j = 0; j < JSTime.size(); j++)
		JSTime[j].resize(this->m_Machines);
	JCTime = JSTime;
	int  bestPos;
	for (int CurFam = 0; CurFam < m_Families; CurFam++)
	{
		// Extract a Fam from FamSeq;		
		vector<int>::iterator it = find(FamSeq.begin(), FamSeq.end(), CurFam);
		int OrgPos = it - FamSeq.begin();
		FamSeq.erase(it);
		//Insert the Fam to the best position
		this->GetJCTime_Forward_InFactory(FamSeq, JobSeqinFam, JCTime);
		this->GetJSTime_Backward_InFactory(FamSeq, JobSeqinFam, JSTime);
		int TempSpan = this->FindBestPosToInsertFam_InFactory(FamSeq, JobSeqinFam, JSTime, JCTime, CurFam, bestPos);
		if (TempSpan < Span)
		{
			Span = TempSpan;
			FamSeq.insert(FamSeq.begin() + bestPos, CurFam);
		}
		else
		{
			FamSeq.insert(FamSeq.begin() + OrgPos, CurFam);
		}

	}
}