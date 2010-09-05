#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <utility>
#include <algorithm>
#include <functional>

using namespace std;

// Change any of these parameters to match your needs

const int POP_SIZE = 10;            // population size
const int MAXN = 751 + 7;           // no. of problem variables
const int MAX_DIMENSION = 100009;   // max. dimension
const int MAX_OUTER_ITER = 50;      // max. iteration
const int MAX_INNER_ITER = 15;      // max. no. of no update
const int INF = 0x3f3f3f3f;         // max. integer
const int MAX_VEHICLE = 2;          // max. no. of vehicles
const int MAX_CAPACITY = 100;       // max. no. of capacity

char instance[109];                        // the name of the data instance
int size;                                  // no. of the cities (excluding the depot)
pair <int, int> points[MAX_DIMENSION];     // the coordinates of the cities
pair <int, int> node[MAXN / 2 + 1];        // the tree nodes (including a pickup point and a delivery point)
int demand[MAXN];                          // the demand of the city
int nodeSize;                              // the size of the tree
int nodeID[MAXN];                          // the tree id of the point
int adj[MAXN / 2 + 1][MAXN / 2 + 1];       // linked list of the tree
int deg[MAXN / 2 + 1];                     // the degree of the tree node
int dis[MAXN][MAXN];                       // the distance matrix
int other[MAXN];                           // the id of the other point in a tree node
int vertexP[MAXN];                         // to record all the pickup points
int nVertexP;                              // the no. of the pickup points
bool isVertexP[MAXN];                      // to record whether a point is a pickup point
int startingTourCost;                      // the cost of the starting tour


// genotype(GT), a member of the population
struct Genotype 
{
	int gene[MAXN + 2];
	int fitness;
	int num;

	Genotype() {}

	Genotype(const Genotype &b)
	{
		memcpy(gene, b.gene, size * sizeof(int));
		fitness = b.fitness;
		num = b.num;
	}
	
	Genotype& operator =(const Genotype &b)
	{
		memcpy(gene, b.gene, size * sizeof(int));
		fitness = b.fitness;
		num = b.num;
		return (*this);
	}
};

Genotype population[POP_SIZE + 2];      // the current population
Genotype newPopulation[POP_SIZE + 2];   // the new population
Genotype src[MAX_VEHICLE];              // the starting solution


void readCoordinate(char *file)
{
	FILE *fp;
	int i, id;
	char s[109];
	
	if ((fp = fopen(file, "r")) == NULL)
	{
		printf("ERROR: Can not open input file %s!\n\n", file);
		exit(1);
	}
	while (true)
	{
		fscanf(fp, "%s", s);
		if (strcmp("NAME", s) == 0)
		{
			fscanf(fp, "%s", s); // read the character ":"
			fscanf(fp, "%s", instance);
		} 
		else if (strcmp("DIMENSION", s) == 0)
		{
			fscanf(fp, "%s", s); // read the character ":"
			fscanf(fp, "%d", &size);
		}
		else if (strcmp("NODE_COORD_SECTION", s) == 0)
		{
			for (i = 0; i < size; ++i)
			{
				fscanf(fp, "%d", &id);
				fscanf(fp, "%d%d", &points[i].first, &points[i].second);
			}
			break;
		}
	}
	fclose(fp);
}

void writeMatching(char *file)
{
	FILE *fp, *wp;
	int i, j, x, state, y, px, py;
	
	if ((fp = fopen(file, "r")) == NULL)
	{
		printf("ERROR: Can not open input file %s!\n\n", file);
		exit(1);
	}

	if ((wp = fopen("test.mtc", "w")) == NULL)
	{
		printf("ERROR: Can not open input file %s!\n\n", file);
		exit(1);
	}
	
	nVertexP = 0;
	nodeSize = 0;
	node[nodeSize++] = make_pair(0, 0);
	nodeID[0] = 0;
	while (fscanf(fp, "%d%d%d", &x, &state, &y) != EOF)
	{
		if (state == 1)
		{
			fprintf(wp, "%d %d %d %d\n", x, state, y, rand()%30+1);
		}
		else
		{
			fprintf(wp, "%d %d %d\n", x, state, y);
		}
	}
}

// to read the matching file.
void readMatching(char *file)
{
	FILE *fp;
	int i, j, x, state, y, px, py, dem;
	
	if ((fp = fopen(file, "r")) == NULL)
	{
		printf("ERROR: Can not open input file %s!\n\n", file);
		exit(1);
	}

	nVertexP = 0;
	nodeSize = 0;
	node[nodeSize++] = make_pair(0, 0);
	nodeID[0] = 0;

	while (fscanf(fp, "%d%d%d", &x, &state, &y) != EOF)
	{
		if (state == 1)
		{
			fscanf(fp, "%d", &dem);
			other[x] = y;
			other[y] = x;
			vertexP[nVertexP++] = x;
			demand[x] = dem;
			demand[y] = -dem;
			isVertexP[x] = true;
			isVertexP[y] = false;
			node[nodeSize] = make_pair(x, y);
			nodeID[x] = nodeSize++;
		}
	}
	
	// to calculate the distance matrix
	--size;
	for (i = 0; i <= size; ++i)
	{
		for (j = 0; j <= size; ++j)
		{
			px = points[i].first - points[j].first;
			py = points[i].second - points[j].second;
			dis[i][j] = (int) (sqrt(px * px + py * py) + 0.5);
		}
	}
	for (i = 0; i <= size + 1; ++i)
	{
		if (i == size + 1)
		{
			dis[0][i] = dis[i][0] = 0;
		}
		else
		{
			dis[i][size + 1] = dis[i][0];
			dis[size + 1][i] = dis[0][i];
		}
	}

	fclose(fp);
}

// to compute the fitness of the genotype.
int calcFitness(const Genotype &gt)
{
	int i, fitness = 0;
	if (gt.num == 0)
	{
		return 0;
	}
	for (i = 1; i < gt.num * 2; ++i)
	{
		fitness += dis[gt.gene[i - 1]][gt.gene[i]];
	}
	fitness += dis[0][gt.gene[0]] + dis[gt.gene[gt.num * 2 - 1]][0];
	return -fitness;
}

// to compute the total fitness of the genotypes.
int calcTotalFitness(Genotype gt[])
{
	int i, fitness = 0;
	for (i = 0; i < MAX_VEHICLE; ++i)
	{
		gt[i].fitness = calcFitness(gt[i]);
		fitness += -gt[i].fitness;
	}
	return -fitness;
}

void getStartingTour(char* file)
{
    int i, j, k, n = 0, m = 1;

	if (file == NULL)
	{
		j = nVertexP / MAX_VEHICLE;
		for (k = 0; k < MAX_VEHICLE; ++k)
		{
			n = 0;
			for (i = 1; i <= j; ++i)
			{
				src[k].gene[n++] = node[m].first;
				src[k].gene[n++] = node[m].second;
				m++;
			}
			src[k].num = j;
		}

		while (m < nodeSize)
		{
			src[MAX_VEHICLE-1].gene[n++] = node[m].first;
			src[MAX_VEHICLE-1].gene[n++] = node[m].second;
			src[MAX_VEHICLE-1].num++;
			m++;
		}
	}
	else
	{
// 		FILE *fp;	
// 		if ((fp = fopen(file, "r")) == NULL) {
// 			printf("ERROR: Can not open input file %s!\n\n", file);
// 			exit(1);
// 		}
// 		fscanf(fp, "%d", &i); // i == 0
// 		for (i = 0; i < size; ++i) {
// 			fscanf(fp, "%d", &src.gene[i]);
// 		}
// 		fclose(fp);
	}

	for (k = 0; k < MAX_VEHICLE; ++k)
	{
		src[k].fitness = calcFitness(src[k]);
		startingTourCost += -src[k].fitness;
	}
}

bool checkCapacity(Genotype gt)
{
	int i;
	int curCap = 0;

	for (i = 0; i < gt.num * 2; ++i)
	{
		curCap += demand[gt.gene[i]];
		if (curCap > MAX_CAPACITY)
		{
			break;
		}
	}

	if (i < gt.num * 2)
	{
		return false;
	}

	return true;
}

bool checkCapacity(int gt[], int size)
{
	int i;
	int curCap = 0;
	
	for (i = 0; i < size; ++i)
	{
		curCap += demand[gt[i]];
		if (curCap > MAX_CAPACITY)
		{
			break;
		}
	}
	
	if (i < size)
	{
		return false;
	}
	
	return true;
}

bool checkAllCapacity(Genotype gt[])
{
	int i;
	
	for (i = 0; i < MAX_VEHICLE; ++i)
	{
		if (checkCapacity(gt[i]) == false)
		{
			break;
		}	
	}

	if (i < MAX_VEHICLE)
	{
		return false;
	}

	return true;
}

// relocate-block operator O(N^2)
bool relocateIntraBlock(Genotype& gt)
{
	int pos[MAXN], vt[MAXN], T2[MAXN];
	int i, j, n, m = 0, length, fitness;
	int x, xx, xl, xr, bestPos, bestFitness = gt.fitness;
	Genotype temp;

	vt[0] = 0, vt[gt.num * 2 + 1] = size + 1;
	memcpy(vt + 1, gt.gene, gt.num * 2 * sizeof(int));
	for (i = 0; i <= gt.num * 2 + 1; ++i)
	{
		pos[vt[i]] = i;
	}
	for (i = 0; i < gt.num * 2; ++i)
	{
		x = gt.gene[i];
		if (!isVertexP[x])
		{
			continue;
		}
		xl = pos[x], xr = pos[other[x]];
		n = 0;
		T2[0] = 0;
		++n;
		memcpy(T2 + n, vt + 1, (xl - 1) * sizeof(int));
		n += xl - 1;
		memcpy(T2 + n, vt + xr + 1, (gt.num * 2 - xr) * sizeof(int));
		n += gt.num * 2 - xr;
		T2[n++] = size + 1;
		length = -gt.fitness
			 	 - dis[vt[xl - 1]][vt[xl]]
				 - dis[vt[xr]][vt[xr + 1]]
				 + dis[vt[xl - 1]][vt[xr + 1]];
		for (j = 0; j < n - 1; ++j)
		{
			fitness = -(length
					  + dis[T2[j]][vt[xl]]
					  + dis[vt[xr]][T2[j + 1]]
					  - dis[T2[j]][T2[j + 1]]);
			m = 0;
			memcpy(temp.gene + m, T2 + 1, j * sizeof(int));
			m += j;
			memcpy(temp.gene + m, vt + xl, (xr - xl + 1) * sizeof(int));
			m += (xr - xl + 1);
			memcpy(temp.gene + m, T2 + j + 1, (n - 2 - j) * sizeof(int));
			temp.num = gt.num;

			if (fitness > bestFitness && checkCapacity(temp))
			{
				bestFitness = fitness;
				xx = x;
				bestPos = j;
			}
		}
	}
	if (bestFitness < gt.fitness)
	{
		printf("relocateBlock(Genotype& gt) is wrong\n");
		exit(1);
	}
	if (bestFitness > gt.fitness)
	{
		xl = pos[xx], xr = pos[other[xx]];
		n = 0;
		T2[0] = 0;
		++n;
		memcpy(T2 + n, vt + 1, (xl - 1) * sizeof(int));
		n += xl - 1;
		memcpy(T2 + n, vt + xr + 1, (gt.num * 2 - xr) * sizeof(int));
		n += gt.num * 2 - xr;
		T2[n++] = size + 1;
		
		m = 0;
		gt.fitness = bestFitness;
		memcpy(gt.gene + m, T2 + 1, bestPos * sizeof(int));
		m += bestPos;
		memcpy(gt.gene + m, vt + xl, (xr - xl + 1) * sizeof(int));
		m += (xr - xl + 1);
		memcpy(gt.gene + m, T2 + bestPos + 1, (n - 2 - bestPos) * sizeof(int));
		m += (n - 2 - bestPos);
		return true;
	}
	return false;
}

// relocate-block operator on different trees O(N^2)
bool relocateInterBlock(Genotype gt[])
{
	int posFirst[MAXN], posSecond[MAXN];
	int vtFirst[MAXN], vtSecond[MAXN], T2[MAXN], T3[MAXN];
	int h, i, j, k, nFirst, nSecond, p, n, m = 0;
	int lengthFirst, lengthSecond, fitness;
	int x, xx, y, yy;
	int xlFirst, xlSecond;
	int xrFirst, xrSecond;
	int bestPos;
	int bestFirst, bestSecond;
	int originFitness, curFitness;
	int bestFitness = 0;
	int test;

	Genotype tempFirst, tempSecond;
	Genotype tempGeno[2];

	for (i = 0; i < MAX_VEHICLE; ++i)
	{
		vtFirst[0] = 0, vtFirst[gt[i].num * 2 + 1] = size + 1;
		memcpy(vtFirst + 1, gt[i].gene, gt[i].num * 2 * sizeof(int));
		for (k = 0; k <= gt[i].num * 2 + 1; ++k)
		{
			posFirst[vtFirst[k]] = k;
		}
		for (h = 0; h < gt[i].num * 2; ++h)
		{
			x = gt[i].gene[h];
			if (!isVertexP[x])
			{
				continue;
			}
			xlFirst = posFirst[x];
			xrFirst = posFirst[other[x]];
			nFirst = 0;
			T2[0] = 0;
			++nFirst;
			memcpy(T2 + nFirst, vtFirst + 1, (xlFirst - 1) * sizeof(int));
			nFirst += xlFirst - 1;
			memcpy(T2 + nFirst, vtFirst + xrFirst + 1, (gt[i].num * 2 - xrFirst) * sizeof(int));
			nFirst += gt[i].num * 2 - xrFirst;
			T2[nFirst++] = size + 1;
			test = calcFitness(gt[i]);
			lengthFirst = - gt[i].fitness
						  - dis[vtFirst[xlFirst - 1]][vtFirst[xlFirst]]
						  - dis[vtFirst[xrFirst]][vtFirst[xrFirst + 1]]
						  + dis[vtFirst[xlFirst - 1]][vtFirst[xrFirst + 1]];
			for (j = 0; j < MAX_VEHICLE; ++j)
			{
				if (i == j)
				{
					continue;
				}
				vtSecond[0] = 0, vtSecond[gt[j].num * 2 + 1] = size + 1;
				memcpy(vtSecond + 1, gt[j].gene, gt[j].num * 2 * sizeof(int));
				for (k = 0; k <= gt[j].num * 2 + 1; ++k)
				{
					posSecond[vtSecond[k]] = k;
				}
				for (k = 0; k < gt[j].num * 2 + 1; ++k)
				{
					lengthSecond = - gt[j].fitness
								   - dis[vtSecond[k]][vtSecond[k + 1]]
								   + dis[vtSecond[k]][vtFirst[xlFirst]]
								   + dis[vtFirst[xrFirst]][vtSecond[k + 1]];
					fitness = - lengthFirst - lengthSecond;

					m = 0;
					memcpy(tempFirst.gene + m, vtFirst + 1, (xlFirst - 1) * sizeof(int));
					m += xlFirst - 1;
					memcpy(tempFirst.gene + m, vtFirst + xrFirst + 1, (gt[i].num * 2 - xrFirst ) * sizeof(int));
					tempFirst.num = gt[i].num - (xrFirst - xlFirst + 1) / 2;
					tempFirst.fitness = calcFitness(tempFirst);

					m = 0;
					memcpy(tempSecond.gene + m, vtSecond + 1, k * sizeof(int));
					m += k;
					memcpy(tempSecond.gene + m, vtFirst + xlFirst, (xrFirst - xlFirst + 1) * sizeof(int));
					m += (xrFirst - xlFirst + 1);
					memcpy(tempSecond.gene + m, vtSecond + k + 1, (gt[j].num * 2 - k) * sizeof(int));
					tempSecond.num = gt[j].num + (xrFirst - xlFirst + 1) / 2;
					tempSecond.fitness = calcFitness(tempSecond);

					if (tempSecond.fitness + tempFirst.fitness != fitness)
					{
						printf("relocateInterBlock fitness is wrong\n");
 						exit(1);
					}
					originFitness = calcFitness(gt[i]) + calcFitness(gt[j]);
					curFitness = fitness - originFitness;
					if (curFitness > bestFitness && checkCapacity(tempFirst)
						                         && checkCapacity(tempSecond))
					{
						bestFitness = curFitness;
						tempGeno[0] = tempFirst;
						tempGeno[1] = tempSecond;
						test = calcTotalFitness(tempGeno);
						bestFirst = i;
						bestSecond = j;
					}
				}
			}
		}
	}

	if (bestFitness < 0)
	{
		printf("relocateBlock(Genotype& gt) is wrong\n");
		exit(1);
	}

	if (bestFitness > 0)
	{
		gt[bestFirst] = tempGeno[0];
		gt[bestSecond] = tempGeno[1];
		return true;
	}
	return false;
}

// block-exchange operator  O(N^2)
bool blockIntraExchange(Genotype& gt)
{
	int vt[MAXN], pos[MAXN];
	int i, j, n = 0, xx, yy, bestFitness = gt.fitness;
	int newL, x, y, xl, xr, yl, yr;
	Genotype temp;
	Genotype best;

	vt[0] = 0, vt[gt.num * 2 + 1] = size + 1;
	memcpy(vt + 1, gt.gene, gt.num * 2 * sizeof(int));
	for (i = 0; i <= gt.num * 2 + 1; ++i)
	{
		pos[vt[i]] = i;
	}
	for (i = 0; i < gt.num * 2; ++i)
	{
		x = gt.gene[i];
		if (!isVertexP[x])
		{
			continue;
		}
		for (j = i + 1; j < gt.num * 2; ++j)
		{
			y = gt.gene[j];
			if (!isVertexP[y])
			{
				continue;
			}
			yl = pos[y];
			yr = pos[other[y]];
			xl = pos[x];
			xr = pos[other[x]];
			if ((xl < yl && yr < xr) ||
				(yl < xl && xr < yr))
			{
				continue;
			}
			if (xl > yl)
			{
				swap(xl, yl);
				swap(xr, yr);
			}
			if (xr + 1 == yl)
			{
				newL = - gt.fitness
					   - dis[vt[xl - 1]][vt[xl]]
					   + dis[vt[xl - 1]][vt[yl]]
					   - dis[vt[yr]][vt[yr + 1]]
					   + dis[vt[xr]][vt[yr + 1]]
					   - dis[vt[xr]][vt[yl]]
					   + dis[vt[yr]][vt[xl]];
			}
			else
			{
				newL = - gt.fitness
					   - dis[vt[xl - 1]][vt[xl]]
					   + dis[vt[xl - 1]][vt[yl]]
					   - dis[vt[yr]][vt[yr + 1]]
					   + dis[vt[xr]][vt[yr + 1]]
					   - dis[vt[xr]][vt[xr + 1]]
					   + dis[vt[yr]][vt[xr + 1]]
					   - dis[vt[yl - 1]][vt[yl]]
					   + dis[vt[yl - 1]][vt[xl]];
			}
			n = 0;
			memcpy(temp.gene + n, vt + 1, (xl - 1) * sizeof(int));
			n += xl - 1;
			memcpy(temp.gene + n, vt + yl, (yr - yl + 1) * sizeof(int));
			n += yr - yl + 1;
			memcpy(temp.gene + n, vt + xr + 1, (yl - xr - 1) * sizeof(int));
			n += yl - xr - 1;
			memcpy(temp.gene + n, vt + xl, (xr - xl + 1) * sizeof(int));
			n += xr - xl + 1;
			memcpy(temp.gene + n, vt + yr + 1, (size - yr) * sizeof(int));
			temp.num = gt.num;
			temp.fitness = calcFitness(temp);
			if (-newL > bestFitness && checkCapacity(temp))
			{
				bestFitness = -newL;
				best = temp;
			}
		}
	}
	if (bestFitness < gt.fitness)
	{
		printf("blockExchange(Genotype& gt) is wrong\n");
		exit(1);
	}
	if (bestFitness > gt.fitness)
	{
		gt = best;
		return true;
	}
	return false;
}

// block-exchange operator  O(N^2)
bool blockInterExchange(Genotype gt[])
{
	int vtFirst[MAXN], vtSecond[MAXN];
	int posFirst[MAXN], posSecond[MAXN];
	int i, j, k, h, n = 0, xx, yy;
	int newL, x, y;
	int xlFirst, xrFirst, xlSecond, xrSecond;
	int xl, xr, yl, yr;
	int originFitness, curFitness;
	int bestFitness = 0;
	int bestI, bestJ;

	Genotype tempFirst, tempSecond;
	Genotype tempGeno[2];

	for (i = 0; i < MAX_VEHICLE; ++i)
	{
		vtFirst[0] = 0, vtFirst[gt[i].num * 2 + 1] = size + 1;
		memcpy(vtFirst + 1, gt[i].gene, gt[i].num * 2 * sizeof(int));
		for (k = 0; k <= gt[i].num * 2 + 1; ++k)
		{
			posFirst[vtFirst[k]] = k;
		}
		for (h = 0; h < gt[i].num * 2; ++h)
		{
			x = gt[i].gene[h];
			if (!isVertexP[x])
			{
				continue;
			}
			for (j = 0; j < MAX_VEHICLE; ++j)
			{
				if (i == j)
				{
					continue;
				}
				vtSecond[0] = 0, vtSecond[gt[j].num * 2 + 1] = size + 1;
				memcpy(vtSecond + 1, gt[j].gene, gt[j].num * 2 * sizeof(int));
				for (k = 0; k <= gt[j].num * 2 + 1; ++k)
				{
					posSecond[vtSecond[k]] = k;
				}
				for (k = 0; k < gt[j].num * 2; ++k)
				{
					y = gt[j].gene[k];
					if (!isVertexP[y])
					{
						continue;
					}
					yl = posSecond[y];
					yr = posSecond[other[y]];
					xl = posFirst[x];
					xr = posFirst[other[x]];
					originFitness = calcFitness(gt[i]) + calcFitness(gt[j]);
					newL = - originFitness
						   - dis[vtFirst[xl - 1]][vtFirst[xl]]
						   + dis[vtFirst[xl - 1]][vtSecond[yl]]
						   - dis[vtSecond[yr]][vtSecond[yr + 1]]
						   + dis[vtFirst[xr]][vtSecond[yr + 1]]
						   - dis[vtFirst[xr]][vtFirst[xr + 1]]
						   + dis[vtSecond[yr]][vtFirst[xr + 1]]
						   - dis[vtSecond[yl - 1]][vtSecond[yl]]
						   + dis[vtSecond[yl - 1]][vtFirst[xl]];
					curFitness = -newL - originFitness;

					n = 0;
					memcpy(tempGeno[0].gene + n, vtFirst + 1, (xl - 1) * sizeof(int));
					n += xl - 1;
					memcpy(tempGeno[0].gene + n, vtSecond + yl, (yr - yl + 1) * sizeof(int));
					n += yr - yl + 1;
					memcpy(tempGeno[0].gene + n, vtFirst + xr + 1, (gt[i].num * 2 - xr) * sizeof(int));
					n += gt[i].num * 2 - xr;
					tempGeno[0].num = (n + 1) / 2;
					tempGeno[0].fitness = calcFitness(tempGeno[0]);

					n = 0;
					memcpy(tempGeno[1].gene + n, vtSecond + 1, (yl - 1) * sizeof(int));
					n += yl - 1;
					memcpy(tempGeno[1].gene + n, vtFirst + xl, (xr - xl + 1) * sizeof(int));
					n += xr - xl + 1;
					memcpy(tempGeno[1].gene + n, vtSecond + yr + 1, (gt[j].num * 2 - yr) * sizeof(int));
					n += gt[j].num * 2 - yr;
					tempGeno[1].num = (n + 1) / 2;
					tempGeno[1].fitness = calcFitness(tempGeno[1]);

					if (curFitness > bestFitness && checkCapacity(tempGeno[0])
						                         && checkCapacity(tempGeno[1]))
					{
						bestFitness = curFitness;
						bestI = i;
						bestJ = j;
						tempFirst = tempGeno[0];
						tempSecond = tempGeno[1];
					}
				}
			}
		}
	}
	if (bestFitness < 0)
	{
		printf("blockExchange(Genotype& gt) is wrong\n");
		exit(1);
	}
	if (bestFitness > 0)
	{
		gt[bestI] = tempFirst;
		gt[bestJ] = tempSecond;
		return true;
	}
	return false;
}

// couple-exchange operator  O(N^2)
bool coupleIntraExchange(Genotype& gt)
{
	int vt[MAXN], pos[MAXN];
	int i, j, bestFitness = gt.fitness, newL;
	int x, y, xl, xr, yl, yr, xl2, xr2, yl2, yr2;
	
	vt[0] = 0;
	vt[gt.num * 2 + 1] = size + 1;
	memcpy(vt + 1, gt.gene, gt.num * 2 * sizeof(int));
	for (i = 0; i <= gt.num * 2; ++i)
	{
		pos[vt[i]] = i;
	}
	for (i = 0; i < gt.num * 2; ++i) {
		x = gt.gene[i];
		if (!isVertexP[x])
		{
			continue;
		}
		xl = pos[x];
		xr = pos[other[x]];
		for (j = i + 1; j < gt.num * 2; ++j) {
			y = gt.gene[j];
			if (!isVertexP[y])
			{
				continue;
			}
			yl = pos[y];
			yr = pos[other[y]];
			newL = - gt.fitness
				   - dis[vt[xl - 1]][vt[xl]]
				   - dis[vt[xl]][vt[xl + 1]]
				   - dis[vt[xr - 1]][vt[xr]]
				   - dis[vt[xr]][vt[xr + 1]]
				   - dis[vt[yl - 1]][vt[yl]]
				   - dis[vt[yl]][vt[yl + 1]]
				   - dis[vt[yr - 1]][vt[yr]]
				   - dis[vt[yr]][vt[yr + 1]]
				   + (xl + 1 == xr ? dis[vt[xl]][vt[xr]] : 0)
				   + (yl + 1 == yr ? dis[vt[yl]][vt[yr]] : 0)
				   + (xr + 1 == yl ? dis[vt[xr]][vt[yl]] : 0)
				   + (yr + 1 == xl ? dis[vt[yr]][vt[xl]] : 0);
			swap(vt[xl], vt[yl]);
			swap(vt[xr], vt[yr]);
			newL += dis[vt[xl - 1]][vt[xl]]
				  + dis[vt[xl]][vt[xl + 1]]
				  + dis[vt[xr - 1]][vt[xr]]
				  + dis[vt[xr]][vt[xr + 1]]
				  + dis[vt[yl - 1]][vt[yl]]
				  + dis[vt[yl]][vt[yl + 1]]
				  + dis[vt[yr - 1]][vt[yr]]
				  + dis[vt[yr]][vt[yr + 1]]
				  - (xl + 1 == xr ? dis[vt[xl]][vt[xr]] : 0)
				  - (yl + 1 == yr ? dis[vt[yl]][vt[yr]] : 0)
				  - (xr + 1 == yl ? dis[vt[xr]][vt[yl]] : 0)
				  - (yr + 1 == xl ? dis[vt[yr]][vt[xl]] : 0);
			if (-newL > bestFitness && checkCapacity(vt + 1, gt.num * 2))
			{
				bestFitness = -newL;
				xl2 = xl, xr2 = xr, yl2 = yl, yr2 = yr;
			}
			swap(vt[xl], vt[yl]);
			swap(vt[xr], vt[yr]);
		}
	}
	if (bestFitness < gt.fitness) {
		printf("coupleExchange(Genotype& gt)\n");
		exit(1);
	}
	if (bestFitness > gt.fitness) {
		gt.fitness = bestFitness;
		--xl2, --xr2, --yl2, --yr2;
		swap(gt.gene[xl2], gt.gene[yl2]);
		swap(gt.gene[xr2], gt.gene[yr2]);
		return true;
	}
	return false;
}

bool coupleInterExchange(Genotype gt[]) {
	int vtFirst[MAXN], vtSecond[MAXN];
	int posFirst[MAXN], posSecond[MAXN];
	int i, j, k, h;
	int newL;
	int x, y, xl, xr, yl, yr, xl2, xr2, yl2, yr2;
	int originFitness, curFitness;
	int bestFitness = 0;
	int bestI, bestJ;

	for (i = 0; i < MAX_VEHICLE; ++i)
	{
		vtFirst[0] = 0, vtFirst[gt[i].num * 2 + 1] = size + 1;
		memcpy(vtFirst + 1, gt[i].gene, gt[i].num * 2 * sizeof(int));
		for (k = 0; k <= gt[i].num * 2 + 1; ++k)
		{
			posFirst[vtFirst[k]] = k;
		}
		for (h = 0; h < gt[i].num * 2; ++h)
		{
			x = gt[i].gene[h];
			if (!isVertexP[x])
			{
				continue;
			}
			xl = posFirst[x];
			xr = posFirst[other[x]];
			for (j = 0; j < MAX_VEHICLE; ++j)
			{
				if (i == j)
				{
					continue;
				}
				vtSecond[0] = 0, vtSecond[gt[j].num * 2 + 1] = size + 1;
				memcpy(vtSecond + 1, gt[j].gene, gt[j].num * 2 * sizeof(int));
				for (k = 0; k <= gt[j].num * 2 + 1; ++k)
				{
					posSecond[vtSecond[k]] = k;
				}
				for (k = 0; k < gt[j].num * 2; ++k)
				{
					y = gt[j].gene[k];
					if (!isVertexP[y])
					{
						continue;
					}
					yl = posSecond[y];
					yr = posSecond[other[y]];
					originFitness = gt[i].fitness + gt[j].fitness;
					newL = - gt[i].fitness
						   - gt[j].fitness
						   - dis[vtFirst[xl - 1]][vtFirst[xl]]
						   - dis[vtFirst[xl]][vtFirst[xl + 1]]
						   - dis[vtFirst[xr - 1]][vtFirst[xr]]
						   - dis[vtFirst[xr]][vtFirst[xr + 1]]
						   - dis[vtSecond[yl - 1]][vtSecond[yl]]
						   - dis[vtSecond[yl]][vtSecond[yl + 1]]
						   - dis[vtSecond[yr - 1]][vtSecond[yr]]
						   - dis[vtSecond[yr]][vtSecond[yr + 1]];
					swap(vtFirst[xl], vtSecond[yl]);
					swap(vtFirst[xr], vtSecond[yr]);
					newL += dis[vtFirst[xl - 1]][vtFirst[xl]]
						  + dis[vtFirst[xl]][vtFirst[xl + 1]]
						  + dis[vtFirst[xr - 1]][vtFirst[xr]]
						  + dis[vtFirst[xr]][vtFirst[xr + 1]]
						  + dis[vtSecond[yl - 1]][vtSecond[yl]]
						  + dis[vtSecond[yl]][vtSecond[yl + 1]]
						  + dis[vtSecond[yr - 1]][vtSecond[yr]]
						  + dis[vtSecond[yr]][vtSecond[yr + 1]];
					curFitness = -newL - originFitness;
					if (curFitness > bestFitness
					 && checkCapacity(vtFirst + 1, gt[i].num * 2)
					 && checkCapacity(vtSecond + 1, gt[j].num * 2))
					{
						bestFitness = curFitness;
						bestI = i;
						bestJ = j;
						xl2 = xl, xr2 = xr, yl2 = yl, yr2 = yr;
					}
					swap(vtFirst[xl], vtSecond[yl]);
					swap(vtFirst[xr], vtSecond[yr]);
				}
			}
		}
	}
	if (bestFitness < 0)
	{
		printf("coupleExchange(Genotype& gt)\n");
		exit(1);
	}
	if (bestFitness > 0)
	{
		--xl2, --xr2, --yl2, --yr2;
		swap(gt[bestI].gene[xl2], gt[bestJ].gene[yl2]);
		swap(gt[bestI].gene[xr2], gt[bestJ].gene[yr2]);
		gt[bestI].fitness = calcFitness(gt[bestI]);
		gt[bestJ].fitness = calcFitness(gt[bestJ]);
		return true;
	}
	return false;
}

// relocate-couple operator  O(N^3)
bool relocateIntraCouple(Genotype& gt)
{
	int vt[MAXN], pos2[MAXN], T2[MAXN];
	int i, j, k, length, newL, bestFitness = gt.fitness;
	int x, xx, lp, rp, n, m = 0;
	
	Genotype temp;

	vt[0] = 0, vt[gt.num * 2 + 1] = size + 1;
	memcpy(vt + 1, gt.gene, gt.num * 2 * sizeof(int));
	for (i = 0; i < gt.num * 2; ++i)
	{
		x = gt.gene[i];
		if (!isVertexP[x])
		{
			continue;
		}
		length = 0;
		n = 0;
		for (j = 0; j <= gt.num * 2 + 1; ++j)
		{
			if (vt[j] != x && vt[j] != other[x])
			{
				T2[n++] = vt[j];
				pos2[T2[n - 1]] = n - 1;
				if (n > 1) length += dis[T2[n - 2]][T2[n - 1]];
			}
		}
		for (j = 0; j < n - 1; ++j)
		{
			newL = length 
				 + dis[T2[j]][x]
				 + dis[x][other[x]]
				 + dis[other[x]][T2[j + 1]]
				 - dis[T2[j]][T2[j + 1]];
			m = 0;
			memcpy(temp.gene + m, T2 + 1, j * sizeof(int));
			m += j;
			temp.gene[m++] = x;
			memcpy(temp.gene + m, T2 + j + 1, (j - j) * sizeof(int));
			m += j - j;
			temp.gene[m++] = other[x];
			memcpy(temp.gene + m, T2 + j + 1, (n - 2 - j) * sizeof(int));
			m += n - 2 - j;
			temp.num = gt.num;
			temp.fitness = calcFitness(temp);
			if (-newL > bestFitness && checkCapacity(temp))
			{
				bestFitness = -newL;
				xx = x;
				lp = j;
				rp = j;
			}
			for (k = j + 1; k < n - 1; ++k)
			{
				if (isVertexP[T2[k]])
				{
					k = pos2[other[T2[k]]];
				}
				else if (pos2[other[T2[k]]] <= j)
				{
					break;
				}
				newL = length
					 + dis[T2[j]][x]
					 + dis[x][T2[j + 1]]
					 + dis[T2[k]][other[x]]
					 + dis[other[x]][T2[k + 1]]
					 - dis[T2[j]][T2[j + 1]]
					 - dis[T2[k]][T2[k + 1]];
				m = 0;
				memcpy(temp.gene + m, T2 + 1, j * sizeof(int));
				m += j;
				temp.gene[m++] = x;
				memcpy(temp.gene + m, T2 + j + 1, (k - j) * sizeof(int));
				m += k - j;
				temp.gene[m++] = other[x];
				memcpy(temp.gene + m, T2 + k + 1, (n - 2 - k) * sizeof(int));
				m += n - 2 - k;
				temp.num = gt.num;
				temp.fitness = calcFitness(temp);
				if (-newL > bestFitness && checkCapacity(temp))
				{
					bestFitness = -newL;
					xx = x;
					lp = j;
					rp = k;
				}
			}
		}
	}
	if (bestFitness < gt.fitness)
	{
		printf("relocateCouple(Genotype& gt) is wrong\n");
		exit(1);
	}
	if (bestFitness <= gt.fitness)
	{
		return false;
	}
	gt.fitness = bestFitness;
	n = 0;
	for (i = 0; i <= gt.num * 2 + 1; ++i)
	{
		if (vt[i] != xx && vt[i] != other[xx])
		{
			T2[n++] = vt[i];
		}
	}
	m = 0;
	memcpy(gt.gene + m, T2 + 1, lp * sizeof(int));
	m += lp;
	gt.gene[m++] = xx;
	memcpy(gt.gene + m, T2 + lp + 1, (rp - lp) * sizeof(int));
	m += rp - lp;
	gt.gene[m++] = other[xx];
	memcpy(gt.gene + m, T2 + rp + 1, (n - 2 - rp) * sizeof(int));
	m += n - 2 - rp;
	gt.fitness = calcFitness(gt);
	return true;
}

bool relocateInterCouple(Genotype gt[])
{
	int vtFirst[MAXN], vtSecond[MAXN];
	int posFirst[MAXN], posSecond[MAXN], pos2[MAXN];
	int T2[MAXN];
	int i, j, k, h, p, q;
	int length, newFirst, newSecond, bestFitness = 0;
	int x, xx, lp, rp, n, m = 0;
	int originFitness, curFitness;
	int newL;
	int bestI, bestJ;
	int best;

	Genotype tempFirst, tempSecond;
	Genotype tempGeno[2];

	for (i = 0; i < MAX_VEHICLE; ++i)
	{
		vtFirst[0] = 0, vtFirst[gt[i].num * 2 + 1] = size + 1;
		memcpy(vtFirst + 1, gt[i].gene, gt[i].num * 2 * sizeof(int));
		for (h = 0; h < gt[i].num * 2; ++h)
		{
			x = gt[i].gene[h];
			if (!isVertexP[x])
			{
				continue;
			}
			length = 0;
			n = 0;
			for (k = 0; k <= gt[i].num * 2 + 1; ++k)
			{
				if (vtFirst[k] != x && vtFirst[k] != other[x])
				{
					T2[n++] = vtFirst[k];
					posFirst[T2[n - 1]] = n - 1;
					if (n > 1) length += dis[T2[n - 2]][T2[n - 1]];
				}
			}
			newFirst = length;
			for (j = 0; j < MAX_VEHICLE; ++j)
			{
				if (i == j)
				{
					continue;
				}
				vtSecond[0] = 0, vtSecond[gt[j].num * 2 + 1] = size + 1;
				memcpy(vtSecond + 1, gt[j].gene, gt[j].num * 2 * sizeof(int));
				for (k = 0; k <= gt[j].num * 2 + 1; ++k)
				{
					posSecond[vtSecond[k]] = k;
				}
				originFitness = gt[i].fitness + gt[j].fitness;
				for (k = 0; k < gt[j].num * 2 + 1; ++k)
				{
					newSecond = - gt[j].fitness
								+ dis[vtSecond[k]][x]
								+ dis[x][other[x]]
								+ dis[other[x]][vtSecond[k + 1]]
								- dis[vtSecond[k]][vtSecond[k + 1]];
					newL = newFirst + newSecond;
					curFitness = -newL - originFitness;
					n = 0;
					for (q = 1; q <= gt[i].num * 2; ++q)
					{
						if (vtFirst[q] != x && vtFirst[q] != other[x])
						{
							tempGeno[0].gene[n++] = vtFirst[q];
						}
					}
					tempGeno[0].num = gt[i].num - 1;
					tempGeno[0].fitness = calcFitness(tempGeno[0]);
					m = 0;
					memcpy(tempGeno[1].gene + m, vtSecond + 1, k * sizeof(int));
					m += k;
					tempGeno[1].gene[m++] = x;
					memcpy(tempGeno[1].gene + m, vtSecond + k + 1, (k - k) * sizeof(int));
					m += k - k;
					tempGeno[1].gene[m++] = other[x];
					memcpy(tempGeno[1].gene + m, vtSecond + k + 1, (gt[j].num * 2 - k) * sizeof(int));
					m += gt[j].num * 2 - k;
					tempGeno[1].num = gt[j].num + 1;
					tempGeno[1].fitness = calcFitness(tempGeno[1]);
					if (curFitness > bestFitness && checkCapacity(tempGeno[0])
						                         && checkCapacity(tempGeno[1]))
					{
						bestFitness = curFitness;
						xx = x;
						lp = k;
						rp = k;
						bestI = i;
						bestJ = j;
						best = newL;
					}
					for (p = k + 1; p < gt[j].num * 2 + 1; ++p)
					{
						if (isVertexP[vtSecond[p]])
						{
							p = posSecond[other[vtSecond[p]]];
						}
						else if (posSecond[other[vtSecond[p]]] <= k)
						{
							break;
						}
						newSecond = - gt[j].fitness
									+ dis[vtSecond[k]][x]
									+ dis[x][vtSecond[k + 1]]
									+ dis[vtSecond[p]][other[x]]
									+ dis[other[x]][vtSecond[p + 1]]
									- dis[vtSecond[k]][vtSecond[k + 1]]
									- dis[vtSecond[p]][vtSecond[p + 1]];
						newL = newFirst + newSecond;
						curFitness = -newL - originFitness;
						n = 0;
						for (q = 1; q <= gt[i].num * 2; ++q)
						{
							if (vtFirst[q] != x && vtFirst[q] != other[x])
							{
								tempGeno[0].gene[n++] = vtFirst[q];
							}
						}
						tempGeno[0].num = gt[i].num - 1;
						tempGeno[0].fitness = calcFitness(tempGeno[0]);
						m = 0;
						memcpy(tempGeno[1].gene + m, vtSecond + 1, k * sizeof(int));
						m += k;
						tempGeno[1].gene[m++] = x;
						memcpy(tempGeno[1].gene + m, vtSecond + k + 1, (p - k) * sizeof(int));
						m += p - k;
						tempGeno[1].gene[m++] = other[x];
						memcpy(tempGeno[1].gene + m, vtSecond + p + 1, (gt[j].num * 2 - p) * sizeof(int));
						m += gt[j].num * 2 - p;
						tempGeno[1].num = gt[j].num + 1;
						tempGeno[1].fitness = calcFitness(tempGeno[1]);
						if (curFitness > bestFitness && checkCapacity(tempGeno[0])
						                             && checkCapacity(tempGeno[1]))
						{
							bestFitness = curFitness;
							xx = x;
							lp = k;
							rp = p;
							bestI = i;
							bestJ = j;
							best = newL;
						}
					}
				}
			}
		}
	}
	if (bestFitness < 0)
	{
		printf("relocateCouple(Genotype& gt) is wrong\n");
		exit(1);
	}
	if (bestFitness <= 0)
	{
		return false;
	}
	vtFirst[0] = 0, vtFirst[gt[bestI].num * 2 + 1] = size + 1;
	memcpy(vtFirst + 1, gt[bestI].gene, gt[bestI].num * 2 * sizeof(int));
	vtSecond[0] = 0, vtSecond[gt[bestJ].num * 2 + 1] = size + 1;
	memcpy(vtSecond + 1, gt[bestJ].gene, gt[bestJ].num * 2 * sizeof(int));
	n = 0;
	for (q = 1; q <= gt[bestI].num * 2; ++q)
	{
		if (vtFirst[q] != xx && vtFirst[q] != other[xx])
		{
			tempFirst.gene[n++] = vtFirst[q];
		}
	}
	tempFirst.num = gt[bestI].num - 1;
	tempFirst.fitness = calcFitness(tempFirst);
	m = 0;
	memcpy(tempSecond.gene + m, vtSecond + 1, lp * sizeof(int));
	m += lp;
	tempSecond.gene[m++] = xx;
	memcpy(tempSecond.gene + m, vtSecond + lp + 1, (rp - lp) * sizeof(int));
	m += rp - lp;
	tempSecond.gene[m++] = other[xx];
	memcpy(tempSecond.gene + m, vtSecond + rp + 1, (gt[bestJ].num * 2 - rp) * sizeof(int));
	m += gt[bestJ].num * 2 - rp;
	tempSecond.num = gt[bestJ].num + 1;
	tempSecond.fitness = calcFitness(tempSecond);
	gt[bestI] = tempFirst;
	gt[bestJ] = tempSecond;
	return true;
}

bool relocateIntraBlocksOnAllTrees(Genotype gt[])
{
	int i;
	bool result;
	bool improved = false;
	
	for (i = 0; i < MAX_VEHICLE; ++i)
	{
		result = relocateIntraBlock(gt[i]);
		if (result == true)
		{
			improved = true;
		}
	}
	return improved;
}

bool blockIntraExchangeOnAllTrees(Genotype gt[])
{
	int i;
	bool result;
	bool improved = false;
	
	for (i = 0; i < MAX_VEHICLE; ++i)
	{
		result = blockIntraExchange(gt[i]);
		if (result == true)
		{
			improved = true;
		}
	}
	return improved;
}

bool relocateIntraCouplesOnAllTrees(Genotype gt[])
{
	int i;
	bool result;
	bool improved = false;
	
	for (i = 0; i < MAX_VEHICLE; ++i)
	{
		result = relocateIntraCouple(gt[i]);
		if (result == true)
		{
			improved = true;
		}
	}
	return improved;
}

bool coupleIntraExchangeOnAllTrees(Genotype gt[])
{
	int i;
	bool result;
	bool improved = false;
	
	for (i = 0; i < MAX_VEHICLE; ++i)
	{
		result = coupleIntraExchange(gt[i]);
		if (result == true)
		{
			improved = true;
		}
	}
	return improved;
}

void printAnswer(Genotype gt[])
{
	int i;
	int j;
	int curCap;
	
	for (i = 0; i < MAX_VEHICLE; ++i)
	{
		curCap = 0;
		printf("Vehicle %d\n", i);
		printf("No. 0+\t\tCapcity: 0\n");
		for (j = 0; j < gt[i].num * 2; ++j)
		{
			curCap += demand[gt[i].gene[j]];
			if ( isVertexP[gt[i].gene[j]])
			{
				printf("No. %d+\t\tCapcity: %d\n", gt[i].gene[j], curCap);
			}
			else
			{
				printf("No. %d-\t\tCapcity: %d\n", other[gt[i].gene[j]], curCap);
			}
		}
		printf("No. 0-\t\tCapcity: 0\n");
		printf("\n");
	}
	
	printf("Total Cost: %d\n", -calcTotalFitness(gt));
	return;
}

void localSearch(Genotype gt[]) {
// 	int start = clock();
	while (true) {
		if (coupleInterExchange(gt))
		{
			printAnswer(gt);
			continue;
		}
		if (coupleIntraExchangeOnAllTrees(gt))
		{
			printAnswer(gt);
			continue;
		}
		if (blockInterExchange(gt))
		{
			printAnswer(gt);
			continue;
		}
		if (blockIntraExchangeOnAllTrees(gt))
		{
			printAnswer(gt);
			continue;
		}
		if (relocateInterBlock(gt))
		{
			printAnswer(gt);
			continue;
		}
		if (relocateIntraBlocksOnAllTrees(gt))
		{
			printAnswer(gt);
			continue;
		}
		if (relocateInterCouple(gt))
		{
			printAnswer(gt);
			continue;
		}
		if (relocateIntraCouplesOnAllTrees(gt))
		{
			printAnswer(gt);
			continue;
		}
		break;
	}
}

int main(int argc, char *argv[])
{
	srand(8);
	if (!(argc == 4 || argc == 3))
	{
		printf("\nERROR: wrong number of input parameters.\n");
		printf("USAGE: exeFile coordinateFile matchingFile resultFile\n");
		exit(1);
	}
	
	int i, j, k = 0, mem, best = INF, startTime = clock();
	readCoordinate(argv[1]);
	readMatching(argv[2]);

	getStartingTour(argc == 4 ? argv[3] : NULL);
	printAnswer(src);

// 	if (checkAllCapacity(src) == true)
// 	{
// 		printf("True\n");
// 	}

	best = 0;

// 	for (i = 0; i < MAX_VEHICLE; ++i)
// 	{
// 		relocateBlock(src[i]);
// 	}

// 	for (i = 0; i < MAX_VEHICLE; ++i)
// 	{
// 		coupleIntraExchange(src[i]);
// 	}

// 	relocateBlockOnDiffTrees(src);

// 	if (checkAllCapacity(src) == true)
// 	{
// 		printf("True\n");
// 	}

// 	blockExchangeOnDiffTrees(src);

// 	coupleInterExchange(src);

// 	for (i = 0; i < MAX_VEHICLE; ++i)
// 	{
// 		relocateIntraCouple(src[i]);
// 	}

// 	blockInterExchange(src);
// 	printAnswer(src);
// 	
// 	blockIntraExchangeOnAllTrees(src);
// 	printAnswer(src);
// 
// 	relocateInterBlock(src);
// 	printAnswer(src);
// 
// 	relocateIntraBlocksOnAllTrees(src);
// 	printAnswer(src);
// 
// 	relocateIntraCouplesOnAllTrees(src);
// 	printAnswer(src);
// 
// 	relocateInterCouple(src);
// 	printAnswer(src);
// 
// 	coupleInterExchange(src);
// 	printAnswer(src);
// 
// 	coupleIntraExchangeOnAllTrees(src);
// 	printAnswer(src);

	localSearch(src);
	printAnswer(src);
	best = -calcTotalFitness(src);



	//printf("Problem Name: %s	Dimension: %d	StartingTourCost: %d", instance, size + 1, startingTourCost);
	//printf("\nTime: %.2f seconds. Cost = %d\n", (clock() - startTime) * 1.0 / (CLOCKS_PER_SEC), best);
	//fprintf(output, "Problem Name: %s 	Dimension: %d   StartingTourCost: %d", instance, size + 1, startingTourCost);
	//fprintf(output, "\nTime: %.2f seconds. Cost = %d\n", (clock() - startTime) * 1.0 / (CLOCKS_PER_SEC), best);
	//fclose(output);
	return 0;
}
