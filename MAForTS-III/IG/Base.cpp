//Base.cpp
#include "stdafx.h"
#include "Base.h"

long Base::GetElapsedProcessTime()
{
	FILETIME createTime;
	FILETIME exitTime;
	FILETIME kernelTime;
	FILETIME userTime;

	long ElapsedTime;
	if (GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime) != 0)
	{
		//  Returns total user time.
		SYSTEMTIME userSystemTime;
		if (FileTimeToSystemTime(&userTime, &userSystemTime) != -1)
			ElapsedTime = (userSystemTime.wDay - 1) * 24 * 3600 * 1000
			+ userSystemTime.wHour * 3600 * 1000 +
			userSystemTime.wMinute * 60 * 1000 +
			userSystemTime.wSecond * 1000 +
			userSystemTime.wMilliseconds;
		else
			ElapsedTime = 0;
	}
	else
		ElapsedTime = 0;
	return ElapsedTime;
}

//GetBestSpan in a file. bDetail=true, file format includes more information
vector<int> Base::GetBestSpan(string ResultsFileName, int Instances, int ExpRep) //Get Best makespan by an algorithm
{
	vector<int> BestSpan(Instances, INT_MAX);
	ifstream ifile;
	ifile.open(ResultsFileName);
	if (ifile.is_open())
	{
		int Span;
		for (int i = 0; i < Instances * ExpRep; i++)
		{
			ifile >> Span;
			BestSpan[i / ExpRep] = min(BestSpan[i / ExpRep], Span);
		}
		ifile.close();
	}
	else
	{
		cout << ResultsFileName << "\t is not open" << endl;
		exit(0);
	}

	return BestSpan;
}

//GetBestSpan in a number of files
vector<int> Base::GetBestSpan(vector <string> ResultsFileNames, int Instances, int ExpRep)
{
	vector<vector<int>> SpanArray(ResultsFileNames.size());
	for (int i = 0; i < ResultsFileNames.size(); i++)
		SpanArray[i] = GetBestSpan(ResultsFileNames[i], Instances, ExpRep);

	vector<int> BestSpan = SpanArray[0];
	for (int j = 0; j < Instances; j++)
	{
		for (int i = 1; i < ResultsFileNames.size(); i++)
			BestSpan[j] = min(BestSpan[j], SpanArray[i][j]);
	}
	return BestSpan;
}

//GetRPI for a file
double Base::GetRPI(string ResultFileName, vector<int> BestSpan, int ExpRep, string RPIFileName)
{
	double AvgRPI = 0;
	ifstream ifile;
	ofstream ofile;
	ifile.open(ResultFileName);
	ofile.open(RPIFileName);
	for (int i = 0; i < BestSpan.size(); i++)
	{
		for (int j = 0; j < ExpRep; j++)
		{
			int Span;
			ifile >> Span;
			double RPI = double(Span - BestSpan[i]) * 100 / BestSpan[i];
			ofile << i << "\t" << RPI << "\t" << endl;
			AvgRPI += RPI;
		}
	}
	ifile.close();
	ofile.close();
	AvgRPI /= BestSpan.size() * ExpRep;
	return AvgRPI;
}

void Base::CombFiles(vector <string> FileNames, string MergeFileName)
{
	ofstream ofile;
	ofile.open(MergeFileName);
	ifstream ifile;
	vector <string> Temp;
	for (int i = 0; i < FileNames.size(); i++)
	{
		ifile.open(FileNames[i]);
		while (true)
		{
			char buf[1000];
			ifile.getline(buf, 1000);
			string str = buf;
			ofile << str << endl;
			Temp.push_back(str);
			if (ifile.peek() == EOF)
				break;
		}
		ifile.close();
	}
	ofile.close();
}

void Base::RecordsAddConfs(vector<int> ParmLevels, int Instances, int Reps, string iFName, string oFName)
{
	int nConf = 1;
	for (int i = 0; i < ParmLevels.size(); i++)
		nConf *= ParmLevels[i];
	ifstream ifile;
	ofstream ofile;
	ifile.open(iFName);
	ofile.open(oFName);
	for (int No = 0; No < nConf; No++)
	{
		//Get Level for each parameter
		vector<int> Level(ParmLevels.size());
		int remainder = No;
		for (int i = 0; i < Level.size(); i++)
		{
			int divisor = 1;
			for (int j = i + 1; j < ParmLevels.size(); j++)
				divisor *= ParmLevels[j];
			Level[i] = remainder / divisor;
			remainder %= divisor;
		}

		// add levels in the records
		for (int Record = 0; Record < Instances * Reps; Record++)
		{
			double RPI;
			int Ins;
			ifile >> Ins >> RPI;
			ofile << Ins << "\t";
			for (int i = 0; i < Level.size(); i++)
			{
				ofile << Level[i] << "\t";
			}
			ofile << RPI << endl;
		}
	}
	ifile.close();
	ofile.close();
}

void Base::WriteBatFiles(int Confs, int PCs, string ExeFileName)
{
	srand(1971);
	vector<int> RunSeq(Confs, 0);
	for (int i = 0; i < Confs; i++)
		RunSeq[i] = i;
	random_shuffle(RunSeq.begin(), RunSeq.end());
	vector <vector<int>> ConfPerPC(PCs);
	int Index_PC = 0;
	for (int i = 0; i < RunSeq.size(); i++)
	{
		Index_PC %= PCs;
		ConfPerPC[Index_PC].push_back(RunSeq[i]);
		Index_PC++;
	}

	for (int index = 0; index < ConfPerPC.size(); index++)
	{
		ostringstream FName;
		FName << "Run_" << index << ".bat";
		ofstream ofile;
		ofile.open(FName.str());
		for (int i = 0; i < ConfPerPC[index].size(); i++)
			ofile << ExeFileName << "\t" << ConfPerPC[index][i] << endl;
		ofile.close();
	}
}

void Base::WriteBatFiles(int Confs, vector<string> IPArray, string ExeFileName)
{
	srand(1971);
	vector<int> RunSeq(Confs, 0);
	for (int i = 0; i < Confs; i++)
		RunSeq[i] = i;
	random_shuffle(RunSeq.begin(), RunSeq.end());
	ofstream ofile;
	ofile.open("Run.bat");
	for (int i = 0; i < Confs; i++)
		ofile << ExeFileName << "\t" << RunSeq[i] << "\t" << IPArray[i%IPArray.size()] << endl;
	ofile.close();
}

void Base::WriteRunConfs(string FileName, int Algs, int Reps, int CPULevels)
{
	ofstream ofile;
	ofile.open(FileName);
	for (int a = 0; a < Algs; a++)
	{
		for (int rep = 0; rep < Reps; rep++)
		{
			for (int TimeLevel = 9; TimeLevel < CPULevels; TimeLevel++)
			{
				ofile << a << "\t" << (TimeLevel + 1) * 50 << "\t" << rep << endl;
			}
		}
	}
	ofile << -1 << "\t" << -1 << "\t" << -1 << endl;
	ofile.close();
}

void Base::GenerateResultsFileNames()
{
	ofstream ofile;
	ofile.open("ResultFileName.txt");
	int Algs = 5;
	int Reps = 5;
	int CPULevels = 10;
	for (int a = 0; a < Algs; a++)
	{
		for (int rep = 0; rep < Reps; rep++)
		{
			for (int TimeLevel = 0; TimeLevel < CPULevels; TimeLevel++)
			{
				ostringstream ostr;
				ostr << "CCEA" << a << "_" << (TimeLevel + 1) * 50 << "_" << rep << ".txt";
				ofile << ostr.str() << endl;
			}
		}
	}
	ofile.close();
}

void Base::FindIP(string &ip)
{
	WORD v = MAKEWORD(1, 1);
	WSADATA wsaData;
	WSAStartup(v, &wsaData); // ¼ÓÔØÌ×½Ó×Ö¿â

	struct hostent *phostinfo = gethostbyname("");
	char *p = inet_ntoa(*((struct in_addr *)(*phostinfo->h_addr_list)));
	int xx = sizeof(p);
	ip = p;
	WSACleanup();
}

void Base::FindLostConfs(string Dir)
{
	int Confs = 300;
	ofstream ofile;
	ofile.open(Dir + "Confs.txt");
	for (int i = 0; i < Confs; i++)
	{
		ostringstream str;
		str << "EA_" << i << ".txt";
		ifstream ifile;
		ifile.open(Dir + str.str());
		if (ifile.is_open())
		{
			ifile.close();
		}
		else
		{
			cout << i << endl;
			ofile << i << endl;
		}
	}
	ofile << -1 << endl;//End of the congfig file
	ofile.close();
}
