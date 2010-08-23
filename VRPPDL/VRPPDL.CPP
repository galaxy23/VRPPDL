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

char instance[109];                        // the name of the data instance
int size;                                  // no. of the cities (excluding the depot)
pair <int, int> points[MAX_DIMENSION];     // the coordinates of the cities
pair <int, int> node[MAXN / 2 + 1];        // the tree nodes (including a pikeup point and a delivery point)
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
struct Genotype {
	int gene[MAXN + 2];
	int fitness;
	
	Genotype() {}
	
	Genotype(const Genotype &b) {
		memcpy(gene, b.gene, size * sizeof(int));
		fitness = b.fitness;
	}
	
	Genotype& operator =(const Genotype &b) {
		memcpy(gene, b.gene, size * sizeof(int));
		fitness = b.fitness;
		return (*this);
	}
};

Genotype population[POP_SIZE + 2];      // the current population
Genotype newPopulation[POP_SIZE + 2];   // the new population
Genotype src;                           // the starting solution

void readCoordinate(char *file) {
	FILE *fp;
	int i, id;
	char s[109];
	
	if ((fp = fopen(file, "r")) == NULL) {
		printf("ERROR: Can not open input file %s!\n\n", file);
		exit(1);
	}
	while (true) {
		fscanf(fp, "%s", s);
		if (strcmp("NAME", s) == 0) {
			fscanf(fp, "%s", s); // read the character ":"
			fscanf(fp, "%s", instance);
		} else if (strcmp("DIMENSION", s) == 0) {
			fscanf(fp, "%s", s); // read the character ":"
			fscanf(fp, "%d", &size);
		} else if (strcmp("NODE_COORD_SECTION", s) == 0) {
			for (i = 0; i < size; ++i) {
				fscanf(fp, "%d", &id);
				fscanf(fp, "%d%d", &points[i].first, &points[i].second);
			}
			break;
		}
	}
	fclose(fp);
}

void WriteMatching(char *file) {
	FILE *fp, *wp;
	int i, j, x, state, y, px, py;
	
	if ((fp = fopen(file, "r")) == NULL) {
		printf("ERROR: Can not open input file %s!\n\n", file);
		exit(1);
	}

	if ((wp = fopen("test.mtc", "w")) == NULL) {
		printf("ERROR: Can not open input file %s!\n\n", file);
		exit(1);
	}
	
	nVertexP = 0;
	nodeSize = 0;
	node[nodeSize++] = make_pair(0, 0);
	nodeID[0] = 0;
	while (fscanf(fp, "%d%d%d", &x, &state, &y) != EOF) {
		if (state == 1) {
			other[x] = y;
			other[y] = x;
			vertexP[nVertexP++] = x;
			isVertexP[x] = true;
			isVertexP[y] = false;
			node[nodeSize] = make_pair(x, y);
			nodeID[x] = nodeSize++;
		}
	}
}
// to read the matching file.
void readMatching(char *file) {
	FILE *fp;
	int i, j, x, state, y, px, py;
	
	if ((fp = fopen(file, "r")) == NULL) {
		printf("ERROR: Can not open input file %s!\n\n", file);
		exit(1);
	}

	nVertexP = 0;
	nodeSize = 0;
	node[nodeSize++] = make_pair(0, 0);
	nodeID[0] = 0;
	while (fscanf(fp, "%d%d%d", &x, &state, &y) != EOF) {
		if (state == 1) {
			other[x] = y;
			other[y] = x;
			vertexP[nVertexP++] = x;
			isVertexP[x] = true;
			isVertexP[y] = false;
			node[nodeSize] = make_pair(x, y);
			nodeID[x] = nodeSize++;
		}
	}
	
	// to calculate the distance matrix
	--size;
	for (i = 0; i <= size; ++i)
		for (j = 0; j <= size; ++j) {
			px = points[i].first - points[j].first;
			py = points[i].second - points[j].second;
			dis[i][j] = (int) (sqrt(px * px + py * py) + 0.5);
		}
		for (i = 0; i <= size + 1; ++i) {
			if (i == size + 1) {
				dis[0][i] = dis[i][0] = 0;
			} else {
				dis[i][size + 1] = dis[i][0];
				dis[size + 1][i] = dis[0][i];
			}
		}
		fclose(fp);
}

int main(int argc, char *argv[]) {
	srand(8);
	if (!(argc == 4 || argc == 3)) {
		printf("\nERROR: wrong number of input parameters.\n");
		printf("USAGE: exeFile coordinateFile matchingFile resultFile\n");
		exit(1);
	}
	
	int i, j, k = 0, mem, best = INF, startTime = clock();
	//readCoordinate(argv[1]);
	readMatching(argv[2]);
	// getStartingTour(argc == 4 ? argv[3] : NULL);
	

	//printf("Problem Name: %s	Dimension: %d	StartingTourCost: %d", instance, size + 1, startingTourCost);
	//printf("\nTime: %.2f seconds. Cost = %d\n", (clock() - startTime) * 1.0 / (CLOCKS_PER_SEC), best);
	//fprintf(output, "Problem Name: %s 	Dimension: %d   StartingTourCost: %d", instance, size + 1, startingTourCost);
	//fprintf(output, "\nTime: %.2f seconds. Cost = %d\n", (clock() - startTime) * 1.0 / (CLOCKS_PER_SEC), best);
	//fclose(output);
	return 0;
}