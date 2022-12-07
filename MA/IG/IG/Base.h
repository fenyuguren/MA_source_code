#pragma once
namespace Base
{
	template <typename T>
	struct Pair
	{
		int dim;
		T value;
	};

	template <typename T>
	struct PairGreater {
		bool operator () (Pair <T> a, Pair <T> b)
		{
			return a.value > b.value;
		}
	};

	template <typename T>
	struct PairLess {
		bool operator () (Pair <T> a, Pair <T> b)
		{
			return a.value < b.value;
		}
	};

	long GetElapsedProcessTime();//Return Process times in millionSeconds;
	vector<int> GetBestSpan(string ResultsFileName, int Instances, int ExpRep);	//GetBestSpan in a file
	vector<int> GetBestSpan(vector <string> ResultsFileNames, int Instances, int ExpRep);//GetBestSpan in a number of files
	double GetRPI(string ResultFileName, vector<int> BestSpan, int ExpRep, string RPIFileName);//GetRPI for a file
	void CombFiles(vector <string> FileNames, string MergeFileName);
	void RecordsAddConfs(vector<int> ParmLevels, int Instances, int Reps, string iFName, string oFName);
	void WriteBatFiles(int Confs, int PCs, string ExeFileName);
	void WriteBatFiles(int Confs, vector<string> IPArray, string ExeFileName);
	void FindIP(string &ip);
	void FindLostConfs(string Dir);
	void WriteRunConfs(string FileName, int Algs = 5, int Reps = 5, int CPULevels = 10);
	void GenerateResultsFileNames();
}