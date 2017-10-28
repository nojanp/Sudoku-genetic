#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <ctime>
using namespace std;

#define LAMBDA 20
#define MU 20
#define ITERATION 200000
#define Pm 0.06
#define Pr 0.5
#define Q 5
#define Qmin 2
#define RESET 200000

struct sudoku
{
	int table[9][9];
	int c;
	int f;
	void setC();
	void print();
};

sudoku population[MU];
sudoku children[LAMBDA];
bool qmark[MU];
int init[9][9];
bool rmark[9][9];
bool bmark[9][9];

void sudoku::setC()
{
	int rCnt[9][9], cCnt[9][9], boxCnt[9][9], cUCnt[9], boxUCnt[9];
	for(int i = 0; i < 9; i++)
		for(int j = 0; j < 9; j++)
			rCnt[i][j] = cCnt[i][j] = boxCnt[i][j] = cUCnt[i] = boxUCnt[i] = 0;

	f = c = 0;
	for(int i = 0; i < 9; i++)
		for(int j = 0; j < 9; j++)
		{
			rCnt[i][table[i][j]-1]++;
			cCnt[j][table[i][j]-1]++;
			boxCnt[(j/3*3)+(i/3)][table[i][j]-1]++;
		}


	for(int i = 0; i < 9; i++)
		for(int j = 0; j < 9; j++)
		{
			cUCnt[i] += cCnt[i][j]==0 ? 0 : 1;
			boxUCnt[i] += boxCnt[i][j]==0 ? 0 : 1;

			c += rCnt[i][j]==0 ? 0 : rCnt[i][j]-1;
			c += cCnt[i][j]==0 ? 0 : cCnt[i][j]-1;
			c += boxCnt[i][j]==0 ? 0 : boxCnt[i][j]-1;
		}

	for(int i = 0; i < 9; i++)
		f += cUCnt[i] + boxUCnt[i];
}

void sudoku::print()
{
	for(int i = 0; i < 9; i++)
	{
		if(i%3 == 0)
			cout << endl;
		for(int j = 0; j < 9; j++)
		{
			if(j%3 == 0)
				cout << "\t";
			cout << table[i][j] << " ";
		}
		cout << endl;
	}
	cout << endl << "c:\t" << c << "\nf:\t" << f << endl;
}

float frand()
{
	return ((float)rand())/((float)RAND_MAX);
}

bool checkRow(int row[9], int entry)
{
	for(int i = 0; i < 9; i++)
		if(row[i] == entry)
			return true;
	return false;
}

bool checkBox(int tab[9][9], int r, int c, int entry)
{
	int rinit = r/3*3, cinit = c/3*3;
	for(int i = rinit; i < rinit+3; i++)
		for(int j = cinit; j < cinit+3; j++)
			if(tab[i][j] == entry)
				return true;
	return false;
}

int chooseParent(int i)
{
	int randomParentF[Q], randomParentIndex[Q];
	for(int j = 0; j < Q; j++)
	{
		int r;
		do{
			r = rand()%MU;
		}while(qmark[r]);
		randomParentF[j] = population[r].f;
		randomParentIndex[j] = r;
	}

	int max;
	max = 0;
	for(int j = 0; j < Q; j++)
		if(j != i && randomParentF[j] > randomParentF[max])
			max = j;

	qmark[randomParentIndex[max]] = true;
	return randomParentIndex[max];
}

bool shouldMutate(sudoku s, int r, int c)
{
	if(init[r][c] != 0)
		return false;
	if(frand() < Pm)
		return true;
	return false;
}

sudoku mutation(sudoku s)
{
	for(int i = 0; i < 9; i++)
		for(int j = 0; j < 9; j++)
			if(shouldMutate(s, i, j))
			{
				int cnt = 0;
				int r;
				do{
					r = rand()%9;
					cnt++;
					if(cnt == 100)
						break;
				}while(r == j || init[i][r] != 0);
				int tmp = s.table[i][j];
				s.table[i][j] = s.table[i][r];
				s.table[i][r] = tmp;
			}
	s.setC();
	return s;

}

sudoku recombination(sudoku p1, sudoku p2)
{
	sudoku p[2] = {p1, p2};
	sudoku ch;

	int r;	
	for(int i = 0; i < 9; i++)
	{
		if(i%3 == 0)
			r = rand()%2;
		for(int j = 0; j < 9; j++)
			ch.table[i][j] = p[r].table[i][j];
	}

	ch.setC();
	return ch;
}

bool tablefiller(int a[9][9], int r, int c)
{
	if(init[r][c])
	{
		if(r==8 && c==8)
			return 1;
		if(c==8)
			return tablefiller(a, r+1, 0);
		else
			return tablefiller(a, r, c+1);
	}
	vector <int> tmp;
	tmp.clear();
	for(int t=0; t < 9; t++)
	{
		if(rmark[r][t] || bmark[(c/3*3)+r/3][t])
			continue;
		tmp.push_back(t+1);
	}
	random_shuffle(tmp.begin(), tmp.end());
	for(int s=0; s < tmp.size(); s++)
	{
		a[r][c]=tmp[s];
		rmark[r][tmp[s]-1]=1;
		bmark[(c/3*3)+r/3][tmp[s]-1]=1;
		int tmpr=r;
		int tmpc=c;	
		if(tmpr==8 && tmpc==8)
		{
			return 1;
		}
		if(tmpc==8)
		{
			tmpr++;
			tmpc=0;
		}
		else
			tmpc++;
		if(tablefiller(a, tmpr, tmpc))
			return 1;
		a[r][c]=0;
		rmark[r][tmp[s]-1]=0;
		bmark[(c/3*3)+r/3][tmp[s]-1]=0;
	}
	return 0;
}

void resetPopulation()
{
	for(int l=0; l < MU; l++)
	{
		for(int i=0; i < 9; i++)
			for(int j=0; j < 9; j++)
				rmark[i][j]=bmark[i][j]=0;
		for(int i=0; i < 9; i++)
			for(int j=0; j < 9; j++)
			{
				population[l].table[i][j]=init[i][j];
				if(init[i][j]!=0)
				{
					rmark[i][init[i][j]-1]=1;
					bmark[(j/3*3)+i/3][init[i][j]-1]=1;
				}
			}
		tablefiller(population[l].table, 0, 0);
		population[l].setC();
	}
}

void nextGen(int gen)
{
	//Q tournament
	sudoku tmpPop[MU];
	bool mark[MU+LAMBDA];
	for(int j = 0; j < MU+LAMBDA; j++)
		mark[j] = false;
	for(int k = 0; k < MU; k++)
	{
		int q = Qmin;// + ((MU-Qmin)/ITERATION)*gen; //change

		sudoku *randomPop;
		randomPop = new sudoku[q];
		for(int j = 0; j < q; j++)
		{
			int r;
			do{
				r = rand()%(MU+LAMBDA);
			}while(mark[r]);
			mark[r] = true;
			randomPop[j] = (r < MU) ? population[r] : children[r%MU];
		}

		int max;
		max = 0;
		for(int j = 0; j < q; j++)
			if(randomPop[j].f > randomPop[max].f)
				max = j;

		tmpPop[k] = randomPop[max];
		delete randomPop;
	}
	for(int k = 0; k < MU; k++)
		population[k] = tmpPop[k];
}
int main()
{
	srand(time(0));
	for(int i = 0; i < 9; i++)
		for(int j = 0; j < 9; j++)
			cin >> init[i][j];

	int it;
	int minC = 1000, minIt;
	for(int i = 0; i < ITERATION; i++)
	{
		if(i%1000 == 0)
			cout << i/1000 << endl;
		for(int j = 0; j < MU; j++)
			qmark[j] = false;

		it = i;
		if(i%RESET == 0)
		{
			cout << "Generation " << i << ", reset population." << endl;
			resetPopulation();
		}

		//recombination:
		for(int j = 0; j < LAMBDA; j+=2)
		{
			int p1 = chooseParent(-1);
			int p2 = chooseParent(p1);

			if(frand() > Pr)
			{
				//				children[j] = (population[p1].c > population[p2].c) ? population[p2] : population[p1];
				children[j] = population[p1];
				children[j+1] = population[p2];
				continue;
			}

			children[j] = recombination(population[p1], population[p2]);
			children[j+1] = recombination(population[p1], population[p2]);
		}

		//mutation:
		for(int j = 0; j < MU; j++)
		{
			population[j] = mutation(population[j]);
		}

		//choose next generation:
		nextGen(i);

		int min = 0;
		for(int k = 0; k < MU; k++)
			if(population[k].c < population[min].c)
				min = k;

		if(population[min].c < minC)
		{
			minIt = i;
			minC = population[min].c;
		}

//		if(i%1000 == 0)
//			cout << population[min].c << endl;
		if(population[min].c == 0)
			break;
	}

	int min = 0;
	for(int i = 0; i < MU; i++)
		if(population[i].c < population[min].c)
			min = i;
	cout << "After " << it << " generations:\n";
	population[min].print();


	for(int i = 0; i < MU; i++)
		cout << "c:\t" << population[i].c << "\tf:\t" << population[i].f << endl;
	cout << "min C:\t\t" << minC << "\nIteration:\t" << minIt << endl;

	return 0;
}
