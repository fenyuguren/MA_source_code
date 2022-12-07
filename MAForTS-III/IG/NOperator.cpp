#include "stdafx.h"
#include "NOperator.h"
#include "gurobi_c++.h"
#include <set>
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
	int mSpan = this->GetJCTime_Forward(FamSeq, JobSeqinFam, JCTime, FacSpan);
	return mSpan;
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
	cout << "正确！";
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
	int d = rand() % m_Families;
	vector <int >JobSeq = JobSeqinFam[d];
	int len = JobSeq.size();
	while (len == 1) {
		d = rand() % m_Families;
		JobSeq = JobSeqinFam[d];
		len = JobSeq.size();
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
int NOperator::Tiebreaking(vector<vector<int>>fits) {
	int n = fits.size();//行数
	int bestpos = -1;
	int MaxTFT = 0;
	vector<int>SpanArray(n), TFTArray(n);
	for (int i = 0; i < n; i++) {
		SpanArray[i] = fits[i][0];
		TFTArray[i] = fits[i][1];
	}
	int MinSpan = *min_element(SpanArray.begin(), SpanArray.end());
	vector<int>TieNum;
	TieNum.clear();
	for (int i = 0; i < n; i++) {
		if (SpanArray[i] == MinSpan) {
			TieNum.push_back(i);
		}
	}
	if (TieNum.size() == 1) {
		bestpos = TieNum[0];
	}
	else {
		for (int i = 0; i < TieNum.size(); i++) {
			int index = TieNum[i];
			int TieTFT = TFTArray[index];
			if (TieTFT > MaxTFT) {
				MaxTFT = TieTFT;
				bestpos = index;
			}
		}
	}

	return bestpos;
}
void NOperator::JobTieInsert(vector <int>FamSeq, vector <vector <int>> &JobSeqinFam, int &Span) {

	int d = rand() % m_Families;
	vector <int >JobSeq = JobSeqinFam[d];
	int seqlen = JobSeq.size();
	while (seqlen == 1) {
		d = rand() % m_Families;
		JobSeq = JobSeqinFam[d];
		seqlen = JobSeq.size();
	}
	if (seqlen % 2 == 1) {
		seqlen--;
	}
	//工件成组
	int duishu = seqlen / 2;
	vector<vector<int>> W(duishu);
	for (int j = 0; j < duishu; j++)
		W[j].resize(2);
	int index = 0;
	for (int i = 0; i < seqlen; i = i + 2) {
		W[index][0] = JobSeq[i];
		W[index][1] = JobSeq[i + 1];
		index++;
	}
	//工件成组取出
	for (int wpos = 0; wpos < duishu; wpos++) {
		vector<int>removejob = W[wpos];
		vector<int>::iterator it = find(JobSeq.begin(), JobSeq.end(), removejob[0]);
		JobSeq.erase(it);
		it = find(JobSeq.begin(), JobSeq.end(), removejob[1]);
		JobSeq.erase(it);
		for (int i = 0; i < 2; i++) {
			int insertjob = removejob[i];
			int positions = JobSeq.size() + 1;
			vector<int>fits;
			fits.clear();
			for (int pos = 0; pos < positions; pos++) {
				vector <int >TempJobSeq = JobSeq;
				TempJobSeq.insert(TempJobSeq.begin() + pos, insertjob);
				JobSeqinFam[d] = TempJobSeq;
				vector<vector<int> > JCTime(m_Jobs, vector<int>(m_Machines, 0));
				int Camx = GetJCTime_Forward_InFactory(FamSeq, JobSeqinFam, JCTime);
				/*int TFT = 0;
				for (int j = 0; j < m_Jobs; j++)
					TFT = TFT + JCTime[j][m_Machines - 1];*/
				fits.push_back(Camx);
			}
			//int bestpos = Tiebreaking(fits);
			auto minPosition=min_element(fits.begin(), fits.end());
			int bestpos = minPosition - fits.begin();
			JobSeq.insert(JobSeq.begin() + bestpos, insertjob);
		}

	}
	JobSeqinFam[d] = JobSeq;
	Span = GetSpan(FamSeq, JobSeqinFam);

}
vector<int> NOperator::LocalSearch(vector<vector<int>>D)
{
	//初始化距离矩阵
	int n = D.size();
	vector<int>permutation, temppermutation;
	//建模求解
	GRBEnv *env = NULL;
	GRBVar **vars = NULL;
	vars = new GRBVar*[n];
	for (int i = 0; i < n; i++)
		vars[i] = new GRBVar[n];
	try
	{
		env = new GRBEnv();
		GRBModel model = GRBModel(*env);

		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				vars[i][j] = model.addVar(0.0, 1.0, D[i][j], GRB_BINARY, "xij");
			}
		}
		model.update();
		for (int i = 0; i < n; i++) {
			GRBLinExpr expr = 0;
			for (int j = 0; j < n; j++)
				expr += vars[i][j];
			model.addConstr(expr == 1, "deg1");
		}
		model.update();
		for (int i = 0; i < n; i++) {
			GRBLinExpr expr = 0;
			for (int j = 0; j < n; j++)
				expr += vars[j][i];
			model.addConstr(expr == 1, "deg2");
		}
		model.update();
		for (int i = 0; i < n; i++)
			vars[i][i].set(GRB_DoubleAttr_UB, 0);
		model.update();
		model.set(GRB_IntParam_IntegralityFocus, 1);
		model.update();
		model.optimize();
		double **sol = new double*[n];
		for (int i = 0; i < n; i++)
			sol[i] = new double[n];
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				sol[i][j] = vars[i][j].get(GRB_DoubleAttr_X);
			}
		}
		int Z = model.get(GRB_DoubleAttr_ObjVal);
		//cout << Z << endl;
		int finished = 0;
		int iteration = 0;
		while (finished == 0)
		{
			iteration++;
			//子问题模型
			GRBVar **x = new GRBVar*[n];
			GRBVar *y = new GRBVar[n];
			for (int i = 0; i < n; i++)
				x[i] = new GRBVar[n];
			GRBModel submodel = GRBModel(*env);
			for (int i = 0; i < n; i++) {
				for (int j = 0; j < n; j++) {
					x[i][j] = submodel.addVar(0.0, 1.0, 0, GRB_BINARY, "x");
				}
			}
			for (int i = 0; i < n; i++) {
				y[i] = submodel.addVar(0.0, 1.0, 0, GRB_BINARY, "y");
			}
			GRBLinExpr expr_1 = 0;
			for (int i = 0; i < n; i++) {
				expr_1 = expr_1 + y[i];
			}
			GRBLinExpr expr_2 = 0;
			for (int i = 0; i < n; i++) {
				for (int j = 0; j < n; j++) {
					expr_2 = expr_2 + x[i][j] * sol[i][j];
				}
			}
			submodel.setObjective(expr_1 - expr_2, GRB_MINIMIZE);
			submodel.update();
			GRBLinExpr expr = 0;
			for (int i = 0; i < n; i++) {
				expr = expr + y[i];
			}
			submodel.addConstr(expr <= n - 2);
			submodel.update();
			submodel.addConstr(expr >= 2);
			submodel.update();
			for (int i = 0; i < n; i++) {
				//GRBLinExpr expr = 0;			
				for (int j = 0; j < n; j++) {
					if (i != j) {
						submodel.addConstr(-y[i] + x[i][j] <= 0);
					}
				}

			}
			submodel.update();
			for (int i = 0; i < n; i++) {
				//GRBLinExpr expr = 0;			
				for (int j = 0; j < n; j++) {
					if (i != j) {
						submodel.addConstr(-y[i] + x[j][i] <= 0);
					}
				}
			}
			submodel.update();
			for (int i = 0; i < n; i++) {
				for (int j = 0; j < n; j++) {
					if (i != j) {
						submodel.addConstr(-y[i] - y[j] + x[i][j] >= -1);
					}
				}
			}
			submodel.update();
			for (int i = 0; i < n; i++)
				x[i][i].set(GRB_DoubleAttr_UB, 0);
			submodel.update();
			submodel.set(GRB_IntParam_IntegralityFocus, 1);
			submodel.update();
			submodel.optimize();
			double *yxing = new double[n];
			for (int i = 0; i < n; i++) {
				yxing[i] = y[i].get(GRB_DoubleAttr_X);
			}
			vector<int>Sstar;
			Sstar.clear();
			for (int i = 0; i < n; i++) {
				if (yxing[i] == 1) {
					Sstar.push_back(i);
				}
			}
			int len = Sstar.size();
			GRBLinExpr cut = 0;
			for (int i = 0; i < len; i++) {
				for (int j = 0; j < len; j++) {
					if (i != j) {
						cut = cut + vars[Sstar[i]][Sstar[j]];
					}
				}
			}
			delete x;
			delete y;
			delete yxing;
			if (len > 1) {
				model.addConstr(cut <= len - 1, "cut");
			}
			model.update();
			model.optimize();
			for (int i = 0; i < n; i++) {
				for (int j = 0; j < n; j++) {
					sol[i][j] = vars[i][j].get(GRB_DoubleAttr_X);
				}
			}
			Z = model.get(GRB_DoubleAttr_ObjVal);
			//cout << Z << endl;
			permutation.clear();
			permutation.push_back(0);
			for (int cishu = 0; cishu < n - 1; cishu++) {
				int lastjob = permutation.back();
				for (int i = 0; i < n; i++)
				{
					if (sol[lastjob][i] == 1) {
						permutation.push_back(i);
						break;
					}
				}
			}
			temppermutation = permutation;
			set<int>s(temppermutation.begin(), temppermutation.end());//set会自动进行排序。
			temppermutation.assign(s.begin(), s.end());
			if (temppermutation.size() == n) {
				finished = 1;
				break;
			}
			else
			{
				finished = 0;
			}
			if (iteration > 50) {
				for (int i = 0; i < n; i++)
					permutation[i] = i;
			}

		}
		delete env;
		delete vars;
		delete sol;
		/*for (int i = 0; i < n; i++)
			cout << permutation[i] << "  ";
		cout << "正确";*/
		return permutation;

	}
	catch (GRBException e)
	{
		cout << "Error code=" << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
		for (int i = 0; i < n; i++)
			permutation[i] = i;
		return permutation;

	}

}
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