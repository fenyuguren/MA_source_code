#include "stdafx.h"
#include "Problem.h"


Problem::Problem()
{
	m_Records = 10;
}

Problem::~Problem()
{
}

void Problem::GenerateInstance()
{
	vector <int> FactoryArray = { 2,3,4,5,6,7 };//revised 20200618.  { 2,3,4 }//original version;
	vector <int> FamilyArray = { 20,40,60 };
	vector <int> MachineArray = { 2,4,6 };
	vector<int> SetupTypeArray = { 0,1,2 }; // Class 0: [1,20]; Class 1: [1,50]; Class 2: [1,100];
	int minJobsInFamily = 1;
	int maxJobsinFamily = 10;
	int minProcessingTime = 1;
	int maxProcessingTime = 10;
	int minSetupTime = 1;
	int InsRep = 5;
	vector<int> maxSetupTimeArray = { 20,50,100 };

	ofstream ofile1, ofile2;
	ofile1.open("InstanceFileNameList.txt");
	srand(20190407);
	int Counter_Files = 1;
	for (int i = 0; i < FactoryArray.size(); i++)
	{
		for (int j = 0; j < FamilyArray.size(); j++)
		{
			for (int k = 0; k < MachineArray.size(); k++)
			{
				for (int l = 0; l < SetupTypeArray.size(); l++)
				{
					for (int rep = 0; rep < InsRep; rep++)
					{
						int Factories = FactoryArray[i];
						int Families = FamilyArray[j];
						int Machines = MachineArray[k];
						int SetupType = SetupTypeArray[l];
						int maxSetupTime = maxSetupTimeArray[SetupType];

						ostringstream FileName;
						FileName << "FG_" << Factories << "_" << Families << "_" << Machines << "_" << SetupType + 1 << "_" << rep + 1 << ".txt";
						cout << Counter_Files << "\t" << FileName.str() << endl;
						ofile1 << Counter_Files << "\t" << FileName.str() << endl;
						ofile2.open(FileName.str());
						ofile2 << "Factories" << "\t" << Factories << endl;
						ofile2 << "Families" << "\t" << Families << endl;
						ofile2 << "Machines" << "\t" << Machines << endl;
						ofile2 << "SetupType" << "\t" << SetupType + 1 << endl;

						//Jobs in each Family
						ofile2 << endl << "Number of Jobs in each Family" << endl;
						vector<vector<int>> JobsinEachFamily(Families);
						int nJobs = 0;
						for (int Fam = 0; Fam < Families; Fam++)
						{
							int nJobsinFamily = rand() % (maxJobsinFamily - minJobsInFamily + 1) + minJobsInFamily;
							for (int xx = 0; xx < nJobsinFamily; xx++)
								JobsinEachFamily[Fam].push_back(nJobs + xx);
							nJobs += nJobsinFamily;
							ofile2 << nJobsinFamily << "\t";
						}
						ofile2 << endl;

						ofile2 << endl << "Jobs in each Family" << endl;
						for (int Fam = 0; Fam < Families; Fam++)
						{
							for (int Job = 0; Job < JobsinEachFamily[Fam].size(); Job++)
								ofile2 << JobsinEachFamily[Fam][Job] << "\t";
							ofile2 << endl;
						}

						//Processing time for jobs
						ofile2 << endl << "Total number of jobs" << endl << nJobs << endl;
						ofile2 << endl << "Processing times of jobs" << endl;
						for (int Job = 0; Job < nJobs; Job++)
						{
							for (int machine = 0; machine < Machines; machine++)
								ofile2 << rand() % (maxProcessingTime - minProcessingTime + 1) + minProcessingTime << "\t";
							ofile2 << endl;
						}

						//SetupTime between Families
						ofile2 << endl << "Stepup times between Families, where [f][f] represents initial setup time" << endl;
						for (int mac = 0; mac < Machines; mac++)
						{
							ofile2 << "On machine\t" << mac + 1 << endl;
							for (int Fam1 = 0; Fam1 < Families; Fam1++)
							{
								for (int Fam2 = 0; Fam2 < Families; Fam2++)
									ofile2 << rand() % (maxSetupTime - minSetupTime + 1) + minSetupTime << "\t";
								ofile2 << endl;
							}
						}

						ofile2.close();
						Counter_Files++;
					}
				}
			}
		}
	}
	ofile1.close();
}

void Problem::ReadInstanceFileNameList(string Dir)
{
	m_InstanceFileNameList.clear();
	ifstream ifile;
	ifile.open(Dir + "\\" + "InstanceFileNameList.txt");
	while (true)
	{
		int x;
		string FName;
		ifile >> x >> FName;
		if (ifile.peek() != EOF)
			m_InstanceFileNameList.push_back(Dir + "\\" + FName);
		else
			break;
	}
	ifile.close();
}

void Problem::ReadInstance(int InsNo)
{
	ifstream ifile;
	ifile.open(m_InstanceFileNameList[InsNo]);
	string str;
	int data;

	//Read Configuration
	ifile >> str >> m_Factories;
	ifile >> str >> m_Families;
	ifile >> str >> m_Machines;
	ifile >> str >> data;

	//Read Number of Jobs in each Family;
	vector<int> NumbofJobsinEachFamily(m_Families);
	for (int i = 0; i < 6; i++)
		ifile >> str;
	for (int Fam = 0; Fam < m_Families; Fam++)
		ifile >> NumbofJobsinEachFamily[Fam];

	//Read Jobs in each Family;
	for (int i = 0; i < 4; i++)
		ifile >> str;
	m_JobsinEachFamily.clear();
	m_JobsinEachFamily.resize(m_Families);
	for (int Fam = 0; Fam < m_Families; Fam++)
		m_JobsinEachFamily[Fam].resize(NumbofJobsinEachFamily[Fam]);
	for (int Fam = 0; Fam < m_Families; Fam++)
		for (int i = 0; i < m_JobsinEachFamily[Fam].size(); i++)
			ifile >> m_JobsinEachFamily[Fam][i];

	//Read Total Number of Jobs
	for (int i = 0; i < 4; i++)
		ifile >> str;
	ifile >> m_Jobs;

	//Read Processing times of Jobs
	for (int i = 0; i < 4; i++)
		ifile >> str;
	m_JobOperPTime.clear();
	m_JobOperPTime.resize(m_Jobs);
	for (int j = 0; j < m_Jobs; j++)
		m_JobOperPTime[j].resize(m_Machines);
	for (int j = 0; j < m_Jobs; j++)
		for (int m = 0; m < m_Machines; m++)
			ifile >> m_JobOperPTime[j][m];

	//Read Stepup times between Families
	for (int i = 0; i < 10; i++)
		ifile >> str;
	m_SetupTime.clear();
	m_SetupTime.resize(m_Machines);
	for (int mac = 0; mac < m_Machines; mac++)
	{
		m_SetupTime[mac].resize(m_Families);
		for (int Fam = 0; Fam < m_Families; Fam++)
			m_SetupTime[mac][Fam].resize(m_Families);
	}

	for (int mac = 0; mac < m_Machines; mac++)
	{
		for (int i = 0; i < 3; i++)
			ifile >> str;		
		for (int Fam1 = 0; Fam1 < m_Families; Fam1++)
			for (int Fam2 = 0; Fam2 < m_Families; Fam2++)
				ifile >> m_SetupTime[mac][Fam1][Fam2];
	}
	

	ifile.close();
}

void Problem::GetJobTotalPTime()
{
	this->m_JobTotalPTime.clear();
	this->m_JobTotalPTime.resize(this->m_Jobs, 0);
	for (int j = 0; j < this->m_JobTotalPTime.size(); j++)
		for (int m = 0; m < this->m_Machines; m++)
			this->m_JobTotalPTime[j] += this->m_JobOperPTime[j][m];
}

void Problem::GetJobWeightTotalPTime()
{
	this->m_JobWeightTotalPTime.clear();
	this->m_JobWeightTotalPTime.resize(this->m_Jobs, 0);
	for (int j = 0; j < this->m_JobWeightTotalPTime.size(); j++)
		for (int m = 0; m < this->m_Machines; m++)
			this->m_JobWeightTotalPTime[j] += this->m_JobOperPTime[j][m]*(this->m_Machines-m);
}

void Problem::GetFamTotalPTime()
{
	this->m_FamTotalPTime.clear();
	this->m_FamTotalPTime.resize(this->m_JobsinEachFamily.size(), 0);
	for(int Fam=0;Fam<this->m_JobsinEachFamily.size();Fam++)
		for (int j = 0; j < this->m_JobsinEachFamily[Fam].size(); j++)
			this->m_FamTotalPTime[Fam] += this->m_JobTotalPTime[this->m_JobsinEachFamily[Fam][j]];
}

void Problem::GetFamTotalPTimeOnFirstMachine()
{
	this->m_FamTotalPTimeOnFirstMachine.clear();
	this->m_FamTotalPTimeOnFirstMachine.resize(this->m_JobsinEachFamily.size(), 0);
	for (int Fam = 0; Fam < this->m_JobsinEachFamily.size(); Fam++)
		for (int j = 0; j < this->m_JobsinEachFamily[Fam].size(); j++)
			this->m_FamTotalPTimeOnFirstMachine[Fam] += this->m_JobOperPTime[this->m_JobsinEachFamily[Fam][j]][0];
}

void Problem::GetFamTotalPTimeOnLastMachine()
{
	this->m_FamTotalPTimeOnLastMachine.clear();
	this->m_FamTotalPTimeOnLastMachine.resize(this->m_JobsinEachFamily.size(), 0);
	for (int Fam = 0; Fam < this->m_JobsinEachFamily.size(); Fam++)
		for (int j = 0; j < this->m_JobsinEachFamily[Fam].size(); j++)
			this->m_FamTotalPTimeOnLastMachine[Fam] += this->m_JobOperPTime[this->m_JobsinEachFamily[Fam][j]][this->m_Machines - 1];
}

void Problem::GetFamWeightTotalPTime()
{
	this->m_FamWeightTotalPTime.clear();
	this->m_FamWeightTotalPTime.resize(this->m_JobsinEachFamily.size(), 0);
	for (int Fam = 0; Fam < this->m_JobsinEachFamily.size(); Fam++)
		for (int j = 0; j < this->m_JobsinEachFamily[Fam].size(); j++)
			this->m_FamWeightTotalPTime[Fam] += this->m_JobWeightTotalPTime[this->m_JobsinEachFamily[Fam][j]]; 
}

void Problem::GetFamAvgSetupTime()
{
	this->m_FamAvgSetupTime.clear();
	this->m_FamAvgSetupTime.resize(this->m_Families, 0);
	for (int CurFam = 0; CurFam < this->m_FamAvgSetupTime.size(); CurFam++)
	{
		for (int PreFam = 0; PreFam < this->m_FamAvgSetupTime.size(); PreFam++)
			for (int m = 0; m < this->m_Machines; m++)
				this->m_FamAvgSetupTime[CurFam] += this->m_SetupTime[m][PreFam][CurFam];
		this->m_FamAvgSetupTime[CurFam] /= this->m_FamAvgSetupTime.size();
	}
}

void Problem::GetFamSumSetupTime()
{
	this->m_FamSumSetupTime.clear();
	this->m_FamSumSetupTime.resize(this->m_Families);
	for (int Fam = 0; Fam < this->m_FamSumSetupTime.size(); Fam++)
		this->m_FamSumSetupTime[Fam].resize(this->m_Families);
	for (int CurFam = 0; CurFam < this->m_FamSumSetupTime.size(); CurFam++)
		for (int PreFam = 0; PreFam < this->m_FamSumSetupTime.size(); PreFam++)
		{
			if (CurFam == PreFam)
			{
				this->m_FamSumSetupTime[PreFam][CurFam] = RAND_MAX;
			}
			else {

				this->m_FamSumSetupTime[PreFam][CurFam] = 0;
				for (int m = 0; m < this->m_Machines; m++)
					this->m_FamSumSetupTime[PreFam][CurFam] = this->m_FamSumSetupTime[PreFam][CurFam] + this->m_SetupTime[m][PreFam][CurFam];
			}
		}
}
