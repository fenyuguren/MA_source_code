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
			int PreFam = FacFamSeq[SelFac][FacFamSeq[SelFac].size() - 1];
			for (int m = 0; m < this->m_Machines; m++)
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
			for (int m = 1; m < this->m_Machines; m++)
				JCTime[CurJob][m] = max(FacMachReadyTime[Fac][m], JCTime[CurJob][m - 1]) + this->m_JobOperPTime[CurJob][m];
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
				JCTime[CurJob][0] = TempFacMachReadyTime[Fac][0] + this->m_JobOperPTime[CurJob][0];
				for (int m = 1; m < this->m_Machines; m++)
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
int NOperator::GetJCTime_Forward(vector <int> FamSeq, vector <vector <int>> JobSeqinFam, vector<vector<int>> &JCTime, int &FacSpan)//Forward pass calculation
{

	FacSpan = GetJCTime_Forward_InFactory(FamSeq, JobSeqinFam, JCTime);
	return FacSpan;
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
	cout << "正确！"<<endl;
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
void NOperator::FamSwap(vector <int> &FamSeq, vector <vector <int>> JobSeqinFam, int &Span) {
	int x = -1, y = -1;
	for (int i = 0; i < m_Families - 1; i++) {
		for (int j = i + 1; j < m_Families; j++) {
			vector <int> Temseq = FamSeq;
			int fam = Temseq[i];
			Temseq[i] = Temseq[j];
			Temseq[j] = fam;
			int Tempspan = GetSpan(Temseq, JobSeqinFam);
			if (Tempspan < Span) {
				Span = Tempspan;
				x = i;
				y = j;
			}
		}
	}
	if (x != -1 && y != -1) {
		int fam = FamSeq[x];
		FamSeq[x] = FamSeq[y];
		FamSeq[y] = fam;
	}
}
void NOperator::JobSwap(vector <int>FamSeq, vector <vector <int>> &JobSeqinFam, int &Span) {
	for (int d = 0; d < m_Families; d++) {
		vector <int >JobSeq = JobSeqinFam[d];
		int len = JobSeq.size();
		if (len == 1) {
			continue;
		}
		int x = -1, y = -1;
		vector <vector <int>>TempJobSeqinFam = JobSeqinFam;
		for (int i = 0; i < len - 1; i++) {
			for (int j = i + 1; j < len; j++) {
				vector <int >Tempjobseq = JobSeq;

				int job = Tempjobseq[i];
				Tempjobseq[i] = Tempjobseq[j];
				Tempjobseq[j] = job;

				TempJobSeqinFam[d] = Tempjobseq;
				int TempSpan = GetSpan(FamSeq, TempJobSeqinFam);
				if (TempSpan < Span) {
					Span = TempSpan;
					x = i;
					y = j;
				}

			}
		}
		if (x != -1 && y != -1) {
			int job = JobSeq[x];
			JobSeq[x] = JobSeq[y];
			JobSeq[y] = job;
			JobSeqinFam[d] = JobSeq;
		}
	}
}

void NOperator::JobInsert(vector <int>FamSeq, vector <vector <int>> &JobSeqinFam, int &Span) {
	for (int index = 0; index < m_Families; index++) {
		int d = FamSeq[index];
		vector <int>JobSeq = JobSeqinFam[d];
		int seqlen = JobSeq.size();
		if (seqlen == 1) {
			continue;
		}
		vector <int>JobPermutation = JobSeq;
		//工件成组取出
		for (int i = 0; i < seqlen; i++) {
			int insertjob = JobPermutation[i];
			vector<int>::iterator it = find(JobSeq.begin(), JobSeq.end(), insertjob);
			JobSeq.erase(it);
			int positions = JobSeq.size() + 1;
			vector<int>fits;
			fits.clear();
			for (int pos = 0; pos < positions; pos++) {
				vector <int >TempJobSeq = JobSeq;
				TempJobSeq.insert(TempJobSeq.begin() + pos, insertjob);
				JobSeqinFam[d] = TempJobSeq;
				vector<vector<int> > JCTime(m_Jobs, vector<int>(m_Machines, 0));
				int Camx = GetJCTime_Forward_InFactory(FamSeq, JobSeqinFam, JCTime);
				fits.push_back(Camx);
			}
			int bestpos = min_element(fits.begin(), fits.end()) - fits.begin();
			JobSeq.insert(JobSeq.begin() + bestpos, insertjob);
		}
		JobSeqinFam[d] = JobSeq;
	}
	Span = GetSpan(FamSeq, JobSeqinFam);
}
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
		JobSeqinFam[r][pt1] = JobSeqinFam[r][pt2];
		JobSeqinFam[r][pt2] = Job;
	}
	else if (JobSeqinFam[r].size() == 2)
	{
		int Job = JobSeqinFam[r][0];
		JobSeqinFam[r][0] = JobSeqinFam[r][1];
		JobSeqinFam[r][1] = Job;
	}
}
void NOperator::SwapFam(vector <int>&FamSeq)
{	
	this->SwapFaminFac(FamSeq);
}
void NOperator::SwapFaminFac(vector <int> &FamSeqInFac)
{
	if (FamSeqInFac.size() < 2) return;
	int pt1 = rand() % FamSeqInFac.size();
	int pt2;
	do {
		pt2 = rand() % FamSeqInFac.size();
	} while (pt1 == pt2);

	int Fam = FamSeqInFac[pt1];
	FamSeqInFac[pt1] = FamSeqInFac[pt2];
	FamSeqInFac[pt2] = Fam;
}