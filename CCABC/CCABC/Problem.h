#pragma once
class Problem
{
public:	
	void GenerateInstance();
	void ReadInstanceFileNameList(string Dir);
	void ReadInstance(int InsNo);
	Problem();
	virtual ~Problem();

protected:
	int m_Factories;
	int m_Jobs;
	int m_Machines;
	int m_Families;
	int m_Records;
	vector<vector<int>> m_JobsinEachFamily; // Jobs in each Family
	vector<vector<int>> m_JobOperPTime; // operation time for each job
	vector<vector<vector<int>>> m_SetupTime; //setup time between Families
	vector<string> m_InstanceFileNameList;
	vector<int> m_JobTotalPTime; //all the operation times for a job;
	vector<int> m_JobWeightTotalPTime; //all the operation times for a job;


	vector<int> m_FamTotalPTime; // all the operation times for the jobs in a Family
	vector<int> m_FamWeightTotalPTime; // all the operation times for the jobs in a Family
	vector<int> m_FamTotalPTimeOnFirstMachine;
	vector<int> m_FamTotalPTimeOnLastMachine;

	vector<vector<int>> m_FammaxSetupTime; //setup time between Families
	vector<double> m_FamAvgSetupTime; //average all the setup times for Families
	vector<double> m_FamAvgmaxSetupTime;//average all the max setup times for Families
	// A Solution
	vector <vector <int>> m_FamSeq; //Family sequence
	vector <vector <int>> m_JobSeqinFam;//Job sequence in each Family

	//Best Solution;
	vector <int> m_bestSeqofSet;
	vector<int> m_bestFamSeq;
	vector<vector<int>> m_bestJobSeqinFam;
	int m_bestSpan;

	void GetJobTotalPTime();
	void GetJobWeightTotalPTime();
	void GetFamTotalPTime();
	void GetFamWeightTotalPTime();
	void GetFamTotalPTimeOnFirstMachine();
	void GetFamTotalPTimeOnLastMachine();
	void GetFamAvgSetupTime();
	void GetFammaxSetupTime();
	void GetFamAvgmaxSetupTime();
};

