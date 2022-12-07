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

