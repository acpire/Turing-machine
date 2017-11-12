// ConsoleApplication3.cpp : Defines the entry point for the console application.
//

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

 struct Instr
{
	int St, StNew; // Текущее и новое состояние машины
	char ch[2], chNew[2]; // Текущий и новый символ на ленте (для двух лент)
	char TapeMove[2]; // команда перемещения ленты
};

class TTape  // Класс ленты МТ
{
private:
	 string Tape;
	 string TapeBack;
public:
	void SetCh(int ind, char ch); // Считать символ с ленты по индексу
	char OutCh(int Ind); // Считать символ с ленты по индексу
	void Load(string FileName);// Загрузить символы на ленту
	void Reset();
	string TapeOut();
};

class TTyring // Машина Тьюринга
{
private:
	TTape Tape[2];
	Instr Prog[100];// Программа
public:
	int NTapes = 1;
	int Q = 0; // Номер текущего состояния
	int Point[2]; // Указатели текущие ячейки ленты
	void LoadProg(string FileName);
	void LoadTape(string FileName, int NType);
    void Work(); // Начать работу машины Тьюренга
	string TapeOut(int NTape);
	TTyring();
};

string TTape::TapeOut()
{
	string buf="";
	for (int i = TapeBack.size() - 1; i > 0; i--)
		buf += TapeBack[i];
	return buf + Tape;
}

void TTyring::LoadProg(string FileName)
{
	ifstream InF(FileName, ios::in);
		int i;
		//	char ch;

		i = 1;

		if (InF.is_open())
			while (true)
			{
				InF >> Prog[i].St;
				if (InF.eof()) break;
				for(int j=0;j<NTapes;j++)
				  InF >> Prog[i].ch[j];

				InF >> Prog[i].StNew;

				for(int j=0;j<NTapes;j++)
				   InF >> Prog[i].chNew[j];

				for (int j = 0;j<NTapes;j++)
					InF >> Prog[i].TapeMove[j];
				i++;
			}
	InF.close();
}

void TTyring::LoadTape(string FileName, int NTape)
{
	Tape[NTape].Load(FileName);
	if (NTape > NTapes) NTapes = NTape;
}

void TTyring::Work() // Начать работу машины Тьюренга
{
		Q = 1;
		Point[0] = 0; Point[1] = 0;
		while (Q > 0)
		{
			for (;;)
			{
				int counter = 0;
				int i = 0;
				do
				{
					i++;
					counter = 0;
					for (int j = 0, count = 0;j < NTapes;j++) // Считаем количество совпадений символов на лентах
						if (Tape[j].OutCh(Point[j]) == Prog[i].ch[j]) counter++; 
				} while ((Q != Prog[i].St || counter != NTapes)&&i<100);
				if (i >= 100 || i < -1)
				{
					cout << "Program error";
					return;
				}
				else
				{
					Q = Prog[i].StNew;
					for (int j = 0;j < NTapes;j++)
					{
						Tape[j].SetCh(Point[j], Prog[i].chNew[j]);
						switch (Prog[i].TapeMove[j])
						{
						case 'R': Point[j]++; break;
						case '>': Point[j]++; break;
						case 'L': Point[j]--; break;
						case '<': Point[j]--; break;
						case 'S': break;
						case '^': break;
						default: cout << "Program error"; return;
						}
					}

					if (Q == 0)
					{
						cout << "  Assepted";
						return;
					}
					else
						if (Q < 0)
						{
							cout << "  Rejected";
							return;
						}
				}
			}
		}
}

char TTape::OutCh(int ind)
{
	if (ind >= 0)
	{
		if ((unsigned)ind < Tape.size())
			return Tape[ind];
		else
		{
			Tape.push_back('#');
			return '#';
		}
	}
	else
	{
		if ((unsigned)(-ind) < TapeBack.size())
			return TapeBack[-ind];
		else
		{
			TapeBack.push_back('#');
			return '#';
		}
	}
}

void TTape::SetCh(int ind, char ch)
{
	if (ind >= 0)
		Tape[ind] = ch;
	else
	{
		ind = -ind;
		TapeBack[ind] = ch;
	}
}

void TTape::Load(string FileName)// Загрузить символы на ленту
{
	char Buf[100];
	ifstream InF(FileName, ios::in);
	//	InF.open(FileName, ios::in);
	if (InF.is_open())
	{
	InF.getline(Buf, 100);
	Tape = Buf;
	Tape += '#';
}
	InF.close();
}

void TTape::Reset()
{
	Tape.clear();
	TapeBack.clear();
}

TTyring::TTyring()
{
	Q = 0;
	Point[0] = 0;
	Point[1] = 0;
}

string TTyring::TapeOut(int NTape)
{
	if (NTape < NTapes)
		return Tape[NTape].TapeOut();
	else
		return "";
}

int main()
{ 
	TTyring MT;

	MT.LoadTape("Tape.txt", 0);
//	MT.LoadTape("Tape.txt", 1);
	MT.LoadProg("Prog.txt");
	cout << "Start tape:\n";
	cout << MT.TapeOut(0)<<"\n";
	MT.Work();
	cout << "\n\nComplite tape:\n";
	cout<<MT.TapeOut(0);

	getchar();
	return 0;
}

