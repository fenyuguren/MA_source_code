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
void Heu::setdiff(vector<int> Originseq, vector<int> Removeseq, vector<int> &Restseq) {
	vector<int> diff;
	diff.resize(max(Originseq.size(), Removeseq.size()));
	sort(Originseq.begin(), Originseq.end());
	sort(Removeseq.begin(), Removeseq.end());
	vector<int>::iterator it = set_difference(Originseq.begin(), Originseq.end(), Removeseq.begin(), Removeseq.end(), diff.begin());
	int len = int(it - diff.begin());
	Restseq.clear();
	//Restseq.resize(len);
	for (int i = 0; i < len; i++) {
		Restseq.push_back(diff[i]);
	}
}
void Heu::schedulejob(vector<int>&jobseq, vector<int> &MachReadyTime) {
	int n = jobseq.size();
	vector<int>unschedulejob = jobseq;
	vector<int>schedulejob;
	schedulejob.clear();
	for (int p = 0; p < n; p++) {
		int minYPCLjk = RAND_MAX;
		vector<int> bestvalue(this->m_Machines, 0);
		int Appendjob;
		for (int k = 0; k < unschedulejob.size(); k++) {
			vector<int> TempMachReadyTime = MachReadyTime;
			int job = unschedulejob[k];
			TempMachReadyTime[0] = TempMachReadyTime[0] + m_JobOperPTime[job][0];
			//计算第一部分的和
			int ITij = 0;
			int IT = 0;
			for (int i = 1; i < m_Machines; i++) {
				if (n > 2) {
					IT = (m_Machines*max(TempMachReadyTime[i - 1] - TempMachReadyTime[i], 0)) / (i + (p*(m_Machines - i)) / (n - 2));
				}
				else {

					IT = 0;
				}
				ITij = ITij + IT;
				TempMachReadyTime[i] = max(TempMachReadyTime[i - 1], TempMachReadyTime[i]) + m_JobOperPTime[job][i];
			}
			if (unschedulejob.size() == 1) {

				Appendjob = job;
				bestvalue = TempMachReadyTime;
				break;
			}
			//计算第二部分
			vector<int> RestJob;
			vector<int>RemoveJob;
			RemoveJob.clear();
			RemoveJob.push_back(job);
			setdiff(unschedulejob, RemoveJob, RestJob);
			vector<int> artjobtime(this->m_Machines, 0);
			for (int m = 0; m < this->m_Machines; m++) {
				for (int j = 0; j < RestJob.size(); j++)
					artjobtime[m] = artjobtime[m] + m_JobOperPTime[RestJob[j]][m];
				artjobtime[m] = artjobtime[m] / RestJob.size();
			}
			vector<int> tempcopy = TempMachReadyTime;
			tempcopy[0] = tempcopy[0] + artjobtime[0];
			for (int m = 1; m < this->m_Machines; m++) {
				tempcopy[m] = max(tempcopy[m - 1], tempcopy[m]) + artjobtime[m];
			}
			int ATjk = tempcopy[m_Machines - 1] + MachReadyTime[m_Machines - 1];
			int YPCLjk = ITij + ATjk;
			if (YPCLjk < minYPCLjk) {
				minYPCLjk = YPCLjk;
				Appendjob = job;
				bestvalue = TempMachReadyTime;
			}

		}
		MachReadyTime = bestvalue;
		schedulejob.push_back(Appendjob);
		setdiff(jobseq, schedulejob, unschedulejob);

	}
	jobseq = schedulejob;
}


void Heu::LRX(vector<vector<int>> FamPop, vector<vector<vector<int>>> &JobSeqinFamPop) {
	vector<int>famseq;
	vector<vector<int>>JobSeqinFam;
	for (int i = 0; i < this->m_Families; i++) {
		//第i个解
		famseq = FamPop[i];
		JobSeqinFam = JobSeqinFamPop[i];
		vector<int> MachReadyTime(this->m_Machines, 0);
		for (int Fam = 0; Fam < this->m_Families; Fam++) {
			int CurFam = famseq[Fam];
			if (Fam == 0) {
				for (int m = 0; m < this->m_Machines; m++)
					MachReadyTime[m] = this->m_SetupTime[m][CurFam][CurFam];
			}
			else //from the second group of jobs to the end;
			{
				int PreFam = famseq[Fam - 1];
				for (int m = 0; m < this->m_Machines; m++)
					MachReadyTime[m] += this->m_SetupTime[m][PreFam][CurFam];
			}
			vector<int>jobseq = JobSeqinFam[CurFam];
			if (jobseq.size() == 1) {
				int CurJob = jobseq.back();
				MachReadyTime[0] = MachReadyTime[0] + m_JobOperPTime[CurJob][0];//on the first machine
				for (int m = 1; m < this->m_Machines; m++) //on the rest machine
					MachReadyTime[m] = max(MachReadyTime[m - 1], MachReadyTime[m]) + m_JobOperPTime[CurJob][m];
				continue;
			}
			schedulejob(jobseq, MachReadyTime);
			JobSeqinFam[CurFam] = jobseq;

		}
		JobSeqinFamPop[i] = JobSeqinFam;
	}
}
void Heu::JPA(vector<int>&InitialFamSeq, vector<vector<int>>&InitialJobinFamSeq, int &Cmax) {
	GetFamSumSetupTime();
	vector<vector<int>>D = this->m_FamSumSetupTime;
	//初始化fam种群
	vector<vector<int>> fampop(this->m_Families);
	for (int j = 0; j < this->m_Families; j++)
		fampop[j].resize(this->m_Families, 0);
	//初始化工件种群
	vector<vector<vector<int>>> JobSeqinFamPop(this->m_Families);
	for (int j = 0; j < this->m_Families; j++)
		JobSeqinFamPop[j] = this->m_JobsinEachFamily;
	//JPA被执行产生fam种群
	vector<int>permutation(this->m_Families);
	for (int j = 0; j < this->m_Families; j++)
		permutation[j] = j;
	vector<int>visited;
	for (int i = 0; i < this->m_Families; i++) {
		visited.clear();
		visited.push_back(permutation[i]);
		for (int j = 0; j < this->m_Families - 1; j++) {
			vector<int> Restseq;
			setdiff(permutation, visited, Restseq);
			int minDis = RAND_MAX;
			int fam;
			for (int p = 0; p < Restseq.size(); p++) {

				if (D[visited.back()][Restseq[p]] < minDis) {
					minDis = D[visited.back()][Restseq[p]];
					fam = Restseq[p];
				}
			}

			visited.push_back(fam);
		}

		fampop[i] = visited;
	}
	
	LRX(fampop, JobSeqinFamPop);
	//插入改进
	vector<int>SpanArray(this->m_Families);
	for (int i = 0; i < this->m_Families; i++) {
		vector<int>FamSeq = fampop[i];
		vector<vector<int>>JobSeqinFam = JobSeqinFamPop[i];
		int Span = GetSpan(FamSeq, JobSeqinFam);
		FamInsert(FamSeq, JobSeqinFam, Span);
		fampop[i] = FamSeq;
		JobSeqinFamPop[i] = JobSeqinFam;
		SpanArray[i] = Span;
	}
	int MinSpan = *min_element(SpanArray.begin(), SpanArray.end());
	int MinSpanPos = min_element(SpanArray.begin(), SpanArray.end()) - SpanArray.begin();
	Cmax = MinSpan;
	InitialFamSeq = fampop[MinSpanPos];
	InitialJobinFamSeq = JobSeqinFamPop[MinSpanPos];
}





