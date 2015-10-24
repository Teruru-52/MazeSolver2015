#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <queue>

#include "Maze.h"

const uint8_t NORTH = 0x01;
const uint8_t EAST = 0x02;
const uint8_t SOUTH = 0x04;
const uint8_t WEST = 0x08;
const uint8_t DONE_NORTH = 0x10;
const uint8_t DONE_EAST = 0x20;
const uint8_t DONE_SOUTH = 0x40;
const uint8_t DONE_WEST = 0x80;

const IndexVec IndexVec::vecNorth(0,1);
const IndexVec IndexVec::vecEast(1,0);
const IndexVec IndexVec::vecSouth(0,-1);
const IndexVec IndexVec::vecWest(-1,0);
const IndexVec IndexVec::vecDir[4] = {IndexVec::vecNorth, IndexVec::vecEast, IndexVec::vecSouth, IndexVec::vecWest};

Maze::Maze()
{
	clear();
}

void Maze::clear()
{
	for (int i=0;i<N;i++) {
		for (int j=0;j<N;j++) {
			wall[i][j] = 0;
		}
	}
	for (int i=0;i<N;i++) {
		wall[N-1][i] |= NORTH | DONE_NORTH;
		wall[i][N-1] |= EAST | DONE_EAST;
		wall[0][i] |= SOUTH | DONE_SOUTH;
		wall[i][0] |= WEST | DONE_WEST;
	}
}

bool Maze::loadFromFile(const char *_filename)
{
	FILE *inputFile;
	inputFile = fopen(_filename, "r");
	if (inputFile == NULL) {
		printf("ERROR : Failed open wall data file\n");
		return false;
	}

	for (int i=0;i<3;i++) {
		int dummy;
		if (fscanf(inputFile, "%d", &dummy) == EOF) {
			printf("ERROR : Failed read wall data\n");
			fclose(inputFile);
			return false;
		}
	}

	size_t cnt = 0;
	char ch;
	while (fscanf(inputFile, "%c", &ch) != EOF) {
		if ( ('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f')) {
			uint8_t wall_bin;
			if ('0' <= ch && ch <= '9') wall_bin = ch - '0';
			else wall_bin = ch - 'a' + 10;

			size_t y = N -1 -cnt/N;
			size_t x = cnt%N;
			wall[y][x].byte = wall_bin | 0xf0;
			cnt++;
		}
	}
	fclose(inputFile);

	return true;
}


void Maze::loadFromArray(const char asciiData[N+1][N+1])
{
	for (int i=0;i<N;i++) {
		for (int j=0;j<N;j++) {
			char ch = asciiData[N-1-i][j];
			if ( ('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f')) {
				uint8_t wall_bin;
				if ('0' <= ch && ch <= '9') wall_bin = ch - '0';
				else wall_bin = ch - 'a' + 10;

				wall[i][j].byte = wall_bin | 0xf0;
			}
		}
	}
}

void Maze::printWall(const uint8_t value[N][N]) const
{
	bool printValueOn = false;
	if (value) printValueOn = true;

	for (int y=N-1;y>=0;y--) {
		for (int x=0;x<N;x++) {
			printf("+");
			if(wall[y][x].bits.North) printf("----");
			else printf("    ");
		}
		printf("+\n\r");

		for (int x=0;x<N;x++) {
			if (wall[y][x].bits.West) printf("|");
			else printf(" ");
			printf(" ");
			if (printValueOn) printf("%3u", value[y][x]);
			else printf("   ");
		}
		printf("|\n\r");
	}
	for (int i=0;i<N;i++) {
		printf("-----");
	}
	printf("+\n\r");
}



void Maze::printWall(const bool value[N][N]) const
{
	bool printValueOn = false;
	if (value) printValueOn = true;

	for (int y=N-1;y>=0;y--) {
		for (int x=0;x<N;x++) {
			printf("+");
			if(wall[y][x].bits.North) printf("----");
			else printf("    ");
		}
		printf("+\n\r");

		for (int x=0;x<N;x++) {
			if (wall[y][x].bits.West) printf("|");
			else printf(" ");
			printf("  ");
			if (printValueOn){
				if (value[y][x]) printf("* ");
				else printf("  ");
			}
			else printf("   ");
		}
		printf("|\n\r");
	}
	for (int i=0;i<N;i++) {
		printf("-----");
	}
	printf("+\n\r");
}

void Maze::printStepMap() const
{
	printWall(stepMap);
}

void Maze::updateWall(const IndexVec &cur, const Direction& newState, bool forceSetDone)
{
	if (forceSetDone) wall[cur.y][cur.x] |= newState | (uint8_t)0xf0;
	else wall[cur.y][cur.x] |= newState;

	//今のEASTをx+1のWESTに反映
	//今のNORTHをy+1のSOUTHに反映
	//今のWESTをx-1のEASTに反映
	//今のSOUTHをy-1のNORTHに反映
	for (int i=0;i<4;i++) {
		if (cur.canSum(IndexVec::vecDir[i])) {
			IndexVec neighbor(cur + IndexVec::vecDir[i]);
			//今のi番目の壁情報ビットとDoneビットを(i+2)%4番目(180度回転方向)に反映
			if (forceSetDone) wall[neighbor.y][neighbor.x] |= (0x10 | newState[i]) << (i+2)%4;
			else wall[neighbor.y][neighbor.x] |= ((newState[i+4]<<4) | newState[i]) << (i+2)%4;
		}
	}
}

void Maze::updateStepMap(const IndexVec &dist)
{
	memset(&stepMap, 0xff, sizeof(uint8_t)*N*N);
	stepMap[dist.y][dist.x] = 0;

	std::queue<IndexVec> q;
	q.push(dist);

	while (!q.empty()) {
		IndexVec cur = q.front();
		q.pop();

		Direction cur_wall = wall[cur.y][cur.x];
		for (int i=0;i<4;i++) {
			const IndexVec scanIndex = cur + IndexVec::vecDir[i];
			if (!cur_wall[i] && stepMap[scanIndex.y][scanIndex.x] > stepMap[cur.y][cur.x] +1) {
				stepMap[scanIndex.y][scanIndex.x] = stepMap[cur.y][cur.x] +1;
				q.push(scanIndex);
			}
		}
	}
}