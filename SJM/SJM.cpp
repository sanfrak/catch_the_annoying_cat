#include<stdio.h> 
#include<stdlib.h> 
#include<windows.h> 
#include<time.h> 
#include<conio.h> 
#include<MMSystem.h>
#include<graphics.h>
#include<easyx.h>
#include<WinBase.h>
#pragma comment (lib, "winmm.lib")

#define N 25
IMAGE pic[N];
MOUSEMSG m;

struct MAP_center{
	int x_value;
	int y_value;
};
struct MAP_center M[9][9];

void gotoxy(int xpos, int ypos)
{
	COORD scrn;
	HANDLE hOuput = GetStdHandle(STD_OUTPUT_HANDLE);
	scrn.X = ypos; scrn.Y = xpos;
	SetConsoleCursorPosition(hOuput, scrn);
}
enum PointType{
	CatType, WallType, FreeType
};
struct Location {
	int row;   //��ÿ�������
	int col;   //��ÿ�������
	enum PointType type;  //��ÿ��������ͣ�
	int step;  //��ÿ�����������
	int path;  //��ÿ��������·����
};
struct Location allPoint[9][9];
struct Location cat;
struct Location *clickPoint;
int trans[2];
int map[9][9];
int hasCircle = 0;

int isValid(struct Location *p);
int getMapRow(struct Location *p);
int getMapCol(struct Location *p);
int calPath(struct Location *p);
int isBoundary(struct Location *p);
int isWall(struct Location *p);
int isInCircle(struct Location *p);
int catAutoGo();
int calPath(struct Location *p);
int calStep(struct Location *p);
int getMapRow(struct Location *p);
int getMapCol(struct Location *p);
int isLessThan(struct Location *p1, struct Location *p2);
int isEqualToCat(int, int);
struct Location * getLeft(struct Location *p);
struct Location * getLeftDown(struct Location *p);
struct Location * getRightDown(struct Location *p);
struct Location * getRight(struct Location *p);
struct Location * getRightUp(struct Location *p);
struct Location * getLeftUp(struct Location *p);
void drawOnePoint(struct Location *p);
void drawAllPoints();
void initAllPoints();
void gotoxy(int row, int col);
void drawAllPoints();
void clearAllStep();
void calAllStep();
void printStep();
void updateStep(int row, int col);
//void printLocation(struct Location *p);
void loadgraph();
int menu();
int input();
int win(int step);
int lose();

int main(void)
{
	int i, j, isGameOver, count, watch;
	char step[5];
	//initialize graph and manu
	loadgraph();
	PlaySound("music\\NyanCat.wav", NULL, SND_FILENAME | SND_ASYNC);	//�񷳴��
	menu();
	//initialize font settings
	LOGFONT f;
	gettextstyle(&f);                     // ��ȡ��ǰ��������
	f.lfHeight = 40;                      // ��������߶�Ϊ 48
	strcpy_s(f.lfFaceName, _T("����"));    // ��������Ϊ�����塱
	f.lfQuality = ANTIALIASED_QUALITY;    // �������Ч��Ϊ�����  
	settextstyle(&f);                     // ����������ʽ
	setbkmode(TRANSPARENT);
	settextcolor(BLACK);

	//initialize the M-matrix
	for (i = 0; i < 9; i++){
		if (i % 2 == 0){
			for (j = 0; j < 9; j++){
				M[i][j].x_value = 30-21 + 49 * j;
				M[i][j].y_value = 208-21 + 44 * i;
			}
		}
		else{
			for (j = 0; j < 9; j++){
				M[i][j].x_value = 55-21 + 49 * j;
				M[i][j].y_value = 208-21 + 44 * i;
			}
		}
	}

	//here comes everyting after the menu
breaking:
	//game setup
	putimage(0, 0, &pic[5]);
	drawAllPoints();
	calAllStep();
	printStep();
	isGameOver = 0;
	count = 0;
	FlushMouseMsgBuffer();

	//Main Circle: Input -> Draw point -> Input ... -> Win/Lose
	while (1){
		int row, col, res;
		watch = input();	//the mouse click order
		if (watch == 2){
			goto breaking;	//Start-over
		}
		if (watch == 1){
			row = trans[0]+1;
			col = trans[1]+1;
			count++;
			updateStep(row, col);	//build the trump wall!
			res = isEqualToCat(row, col);	//in case accidentally clicked the cat
			if (isGameOver == 1 && res == 0) {
				continue;	//invalid order, back to input()
			}
			else if (isGameOver == 1) {
				break; //border point chosen, game over
			}
			sprintf_s(step, "%d", count);
			putimage(291, 136, &pic[9]);
			settextcolor(WHITE);
			outtextxy(294, 139, step);

			//catAutoGo():calculate the remaining steps & best path
			isGameOver = catAutoGo();
			if (isGameOver == -1)
				break;//cat escaped, lose
			if (isGameOver == 1)
				break;//cat captured, win
			calAllStep();
			printStep();	//draw next point of cat
		}
	}	//break out from while(input+move)

	//wining - output:steps
	if (isGameOver == 1){
		int WIN=0;
		WIN = win(count);
		if (WIN)
			goto breaking;	//Start-over
	}
	//losing - output:non
	else if (isGameOver == -1){
		int LOSE=0;
		LOSE = lose();
		if (LOSE)
			goto breaking;	//Start-over
	}
	_getch();
	closegraph();
}

int menu()	//�˵�������ѡ��ʼ��Ϸ���鿴��Ϸ˵��
{
	FlushMouseMsgBuffer();
	initgraph(478, 599);
	putimage(0, 0, &pic[0]);
	int flag = 0;
	while (1){
		m = GetMouseMsg();
		if (m.x >= 140 && m.x <= 330 && m.y >= 500 && m.y <= 550 && flag == 0 && m.uMsg == WM_LBUTTONDOWN){
			flag++;
			putimage(0, 0, &pic[1]);	//����˵������
			}
		else if (flag >= 1 && m.uMsg == WM_LBUTTONDOWN)
			return 0;	//������Ϸ����
	}
}

int input()	//���뺯�������������λ��ѡ����ӦԲ��
{
	int i, j, k, kx,ky,r = 22;
	FlushMouseMsgBuffer();
	m = GetMouseMsg();
	switch (m.uMsg){
	case WM_LBUTTONDOWN:
		if (m.x > 195 && m.x < 220 && m.y>110 && m.y < 170)
			return 2;	//ѡ�С����桱
		for (i = 0; i < 9; i++)
			for (j = 0; j < 9; j++){
			kx = m.x - M[i][j].x_value-21;
			ky = m.y - M[i][j].y_value-21;
			k = kx*kx + ky*ky - r*r;
			if (k < 0){		//����ѡ�е�Բ������
				trans[0] = i;
				trans[1] = j;
				return 1;
			}
			}
	}
}

int win(int step)	//��������1��ʤ�������������
{
	int Num;	//Num1 is the step count
	Num = step;
	char temp1[5], temp2[5], temp3[5];
	setbkmode(TRANSPARENT);
	settextcolor(BLACK);
	putimage(0, 0, &pic[3]);
	sprintf_s(temp1, "%d", Num);
	outtextxy(115, 161, temp1);		//��ʾ����
	sprintf_s(temp2, "%d", 100-Num);
	outtextxy(215, 255, temp2);		//��ʾ�ɼ�
	sprintf_s(temp3, "%d+1", 100 - Num);
	outtextxy(30, 444, temp3);		//��ʾ��ѳɼ�
	while (1){
		m = GetMouseMsg();
		if (m.x >= 50 && m.x <= 200 && m.y >= 510 && m.y <= 560 && m.uMsg == WM_LBUTTONDOWN)
			exit(1);	//End game
		if (m.x >= 260 && m.x <= 410 && m.y >= 510 && m.y <= 560 && m.uMsg == WM_LBUTTONDOWN){
			return 1;	//Start-over
		}
	}
}

int lose()	//��������2��ʧ��
{
	putimage(0, 0, &pic[2]);
	while (1){
		m = GetMouseMsg();
		if (m.x >= 50 && m.x <= 200 && m.y >= 510 && m.y <= 560 && m.uMsg == WM_LBUTTONDOWN)
			exit(1);	//End game
		if (m.x >= 260 && m.x <= 410 && m.y >= 510 && m.y <= 560 && m.uMsg == WM_LBUTTONDOWN){
			return 1;	//Start-over
		}
	}
}

void loadgraph()
{
	loadimage(&pic[0], "picture\\start.jpg", 478, 599, true);
	loadimage(&pic[1], "picture\\start2.jpg", 478, 599, true);
	loadimage(&pic[2], "picture\\end_f.jpg", 478, 599, true);
	loadimage(&pic[3], "picture\\end_w.jpg", 478, 599, true);
	loadimage(&pic[4], "picture\\jam.jpg", 45, 45, true);
	loadimage(&pic[5], "picture\\playground.jpg", 478, 599, true);
	loadimage(&pic[6], "picture\\cat.jpg", 45, 45, true);
	loadimage(&pic[7], "picture\\empty.jpg", 45, 45, true);
	loadimage(&pic[8], "picture\\cat2.jpg", 45, 45, true);	//flag
	loadimage(&pic[9], "picture\\package.jpg", 39, 43, true);
}

void updateStep(int row, int col)	//����������µ�ͼ�ϵ��ϰ���
{
	row--;
	col--;
	struct Location *loc = &allPoint[row][col];
	loc->type = WallType;
	map[row][col] = 1;
	clickPoint = loc;
	drawOnePoint(loc);
	clearAllStep();	//put this step unavailable
	calAllStep();
	printStep();
}

void drawOnePoint(struct Location *p)	//��������(����p)����(����+����)
{
	int i, j, px, py;
	i = p->row -1;
	j = p->col -1;
	px = M[i][j].x_value;
	py = M[i][j].y_value;
	//���ݵ�����͵Ĳ�ͬ�����Ӧ��ͼƬ(è/�ϰ���/�հ�ͼ)
	if (p->type == CatType) {
		putimage(px, py, &pic[6]);
	}
	else if (p->type == WallType) {
		putimage(px, py, &pic[4]);
	}
	else 
		putimage(px, py, &pic[7]);
}

void drawAllPoints()	//��ʼ������֮һ���������е�
{
	initAllPoints();
	int i, j;
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			drawOnePoint(&allPoint[i][j]);
		}
	}
	drawOnePoint(&cat);
}

int isEqualToCat(int row, int col)	//��ֹ��㵽è�ĳ���
{
	int catRow = getMapRow(&cat);
	int catCol = getMapCol(&cat);
	if (catRow == row - 1 && catCol == col - 1) {
		return 1;
	}
	return 0;
}

void calAllStep()		//����è��Χ�Ŀ���·����
{
	clearAllStep();		// ����ǰ�������ǰ���е���Ϣ
	int i, j, k;
	//�����·�������
	for (i = 0; i<9; i++) {
		int k = i;
		for (j = 0; j <= i; j++) {
			calPath(&allPoint[j][k]);
			calPath(&allPoint[8 - j][8 - k]);
			k--;
		}
	}
	//�����·�������
	for (i = 0; i < 9; i++) {
		k = 8 - i;
		for (j = 0; j <= i; j++) {
			calPath(&allPoint[j][k]);
			calPath(&allPoint[k][j]);
			k++;
		}
	}
	//���մ����ҡ������ĸ������������
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			calPath(&allPoint[j][i]);
			calPath(&allPoint[i][j]);
			calPath(&allPoint[j][8 - i]);
			calPath(&allPoint[8 - i][j]);
		}
	}
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			calStep(&allPoint[i][j]);
		}
	}
	
	hasCircle = isInCircle(&cat);	//�ж�è�Ƿ���·����
}

void printCatConnectedPointStep()
{
	struct Location *locationSet[6];
	int i = getMapRow(&cat);
	int j = getMapCol(&cat);
	struct Location p = allPoint[i][j];
	locationSet[0] = getLeftUp(&p);
	locationSet[1] = getLeft(&p);
	locationSet[2] = getLeftDown(&p);
	locationSet[3] = getRightDown(&p);
	locationSet[4] = getRight(&p);
	locationSet[5] = getRightUp(&p);
}

int getMaxStep(struct Location **pp)
{
	if (hasCircle == 0) {
		return -1;
	}
	struct Location *locationSet[6];
	int i = getMapRow(&cat);
	int j = getMapCol(&cat);
	struct Location p = allPoint[i][j];
	locationSet[0] = getLeftUp(&p);
	locationSet[1] = getLeft(&p);
	locationSet[2] = getLeftDown(&p);
	locationSet[3] = getRightDown(&p);
	locationSet[4] = getRight(&p);
	locationSet[5] = getRightUp(&p);
	int max = locationSet[0]->step;
	int index = 0;
	int wallNum = 0;
	for (i = 0; i < 6; i++) {
		if (locationSet[i]->step < 100) {
			max = locationSet[i]->step;
			index = i;
			break;
		}
	}
	for (i = 0; i < 6; i++) {
		if (locationSet[i]->step == 100) {
			wallNum++;
			continue;
		}
		if (max < locationSet[i]->step) {
			max = locationSet[i]->step;
			index = i;
		}
	}
	if (wallNum < 6) {
		*pp = locationSet[index];
	}
	return max;
}

struct Location * getBestLocation()		//�õ���Χ�����·����
{
	struct Location *best = NULL;
	int max = getMaxStep(&best);
	if (best != NULL) {
		return best;
	}
	else if (hasCircle == 1) {
		return best;
	}
	struct Location *locationSet[6];
	int i = getMapRow(&cat);
	int j = getMapCol(&cat);
	struct Location p = allPoint[i][j];
	locationSet[0] = getLeftUp(&p);
	locationSet[1] = getLeft(&p);
	locationSet[2] = getLeftDown(&p);
	locationSet[3] = getRightDown(&p);
	locationSet[4] = getRight(&p);
	locationSet[5] = getRightUp(&p);

	for (i = 0; i < 6; i++) {
		if (isValid(locationSet[i]) == 1) {
			best = locationSet[i];
			break;
		}
	}
	for (j = i + 1; j < 6; j++) {
		if (isValid(locationSet[j]) == 1 && isLessThan(locationSet[j], best) == 1) {
			best = locationSet[j];
		}
	}
	if (best != NULL) {
		//printLocation(best);	//�ݲ���Ҫʹ��
	}
	return best;
}

int catAutoGo()	//è������Χ���ʣ�ಽ������ֵѡ�����·��
{
	struct Location *best = getBestLocation();
	if (best != NULL) {
		int i = getMapRow(&cat);
		int j = getMapCol(&cat);
		if (clickPoint->row == allPoint[i][j].row &&
			clickPoint->col == allPoint[i][j].col) {
			//do nothing
		}
		else {
			//erase last position
			putimage(M[i][j].x_value, M[i][j].y_value, &pic[7]);
			map[i][j] = 0;
			allPoint[i][j].type = FreeType;
		}
		cat.row = best->row;
		cat.col = best->col;
		i = getMapRow(&cat);
		j = getMapCol(&cat);
		map[i][j] = 1;
		allPoint[i][j].type = CatType;
		cat.path = 888;		//can't include the current position
		drawOnePoint(&cat);
		if (i == 0 || i == 8 || j == 0 || j == 8) {
			return -1; //Game Over
		}
	}
	else {
		return 1; //Only one point
	}
	return 0;
}

void printStep()	//��è����һ���н���
{
	int i, j, a, row, col;
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			a = allPoint[i][j].path;
			if (hasCircle == 1) {
				a = allPoint[i][j].step;
			}
			row = getMapRow(&cat);
			col = getMapCol(&cat);
			if (i == row && j == col) {		//��è
				putimage(M[i][j].x_value, M[i][j].y_value, &pic[6]);
			}
		}
	}
	printCatConnectedPointStep();
}

void clearAllStep()
{
	int i, j;
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			allPoint[i][j].step = -100;
			allPoint[i][j].path = -100;
		}
	}
}

int getMapRow(struct Location *p)
{
	return p->row - 1;
}

int getMapCol(struct Location *p)
{
	int row = p->row - 1;
	int col = 0;
	col = p->col - 1;
	return col;
}

//����Χ����Ѱ���µ�ĺ���
struct Location * getLeft(struct Location *p)
{
	struct Location * newp = NULL;
	//get the row and col
	int row = getMapRow(p);
	int col = getMapCol(p);
	//left point
	if (col > 0) {
		newp = &allPoint[row][col - 1];
	}
	return newp;
}
struct Location * getRight(struct Location *p)
{
	struct Location *newp = NULL;
	//get the row and col
	int row = getMapRow(p);
	int col = getMapCol(p);
	//left point
	if (col < 8) {
		newp = &allPoint[row][col + 1];
	}
	return newp;
}
struct Location * getLeftDown(struct Location *p)
{
	struct Location *newp = NULL;
	//get the row and col
	int row = getMapRow(p);
	int col = getMapCol(p);
	//left down point
	if (row < 8) {
		p = &allPoint[row + 1][col];
		if (row % 2 == 0) {
			if (col == 0) {
				newp = NULL;
			}
			else {
				newp = &allPoint[row + 1][col - 1];
			}
		}
		else {
			newp = p;
		}
	}
	return newp;
}
struct Location * getRightDown(struct Location *p)
{
	struct Location *newp = NULL;
	//get the row and col
	int row = getMapRow(p);
	int col = getMapCol(p);
	//right down point
	if (row < 8) {
		p = &allPoint[row + 1][col];
		if (row % 2 == 0) {
			newp = p;
		}
		else {
			if (col == 8) {
				newp = NULL;
			}
			else {
				newp = &allPoint[row + 1][col + 1];
			}
		}
	}
	return newp;
}
struct Location * getRightUp(struct Location *p)
{
	struct Location *newp = NULL;
	//get the row and col
	int row = getMapRow(p);
	int col = getMapCol(p);
	//right up point
	if (row > 0) {
		p = &allPoint[row - 1][col];
		if (row % 2 == 0) {
			newp = p;
		}
		else {
			if (col == 8) {
				newp = NULL;
			}
			else {
				newp = &allPoint[row - 1][col + 1];
			}
		}
	}
	return newp;
}
struct Location * getLeftUp(struct Location *p)
{
	struct Location *newp = NULL;
	//get the row and col
	int row = getMapRow(p);
	int col = getMapCol(p);
	//left up point
	if (row > 0) {
		p = &allPoint[row - 1][col];
		if (row % 2 == 0) {
			if (col == 0) {
				newp = NULL;
			}
			else {
				newp = &allPoint[row - 1][col - 1];
			}
		}
		else {
			newp = p;
		}
	}
	return newp;
}

int isValid(struct Location *p)
{
	int row = getMapRow(p);
	int col = getMapCol(p);
	if (p != NULL && map[row][col] == 0) {
		return 1;
	}
	else
		return 0;
}

int calStep(struct Location *p)		//����ʣ�ಽ��
{
	int step = 0;
	//get the row and col
	int row = getMapRow(p);
	int col = getMapCol(p);
	if (map[row][col] == 1) {
		p->step = 100;
		return p->step;
	}
	if (row == 0 || col == 0 || row == 8 || col == 8) {
		p->step = 0;
		return p->step;
	}
	//left point
	struct Location *leftP = getLeft(p);
	if (isValid(leftP) == 1) {
		step++;
	}

	struct Location *rightP = getRight(p);
	if (isValid(rightP) == 1) {
		step++;
	}

	struct Location *leftDownP = getLeftDown(p);
	if (isValid(leftDownP) == 1) {
		step++;
	}

	struct Location *rightDownP = getRightDown(p);
	if (isValid(rightDownP) == 1) {
		step++;
	}

	struct Location *leftUpP = getLeftUp(p);
	if (isValid(leftUpP) == 1) {
		step++;
	}

	struct Location *rightUpP = getRightUp(p);
	if (isValid(rightUpP) == 1) {
		step++;
	}

	p->step = step;
	return p->step;
}

int calPath(struct Location *p)		//��������·��
{
	int step = 0;
	//get the row and col
	int row = getMapRow(p);
	int col = getMapCol(p);
	if (map[row][col] == 1) {
		p->path = 100;
		return p->path;
	}
	if (row == 0 || col == 0 || row == 8 || col == 8) {
		p->path = 0;
		return p->path;
	}
	//left point
	struct Location *validSet[6];

	struct Location *leftP = getLeft(p);
	validSet[step] = leftP;
	step++;

	struct Location *leftDownP = getLeftDown(p);
	validSet[step] = leftDownP;
	step++;

	struct Location *rightDownP = getRightDown(p);
	validSet[step] = rightDownP;
	step++;

	struct Location *rightP = getRight(p);
	validSet[step] = rightP;
	step++;

	struct Location *rightUpP = getRightUp(p);
	validSet[step] = rightUpP;
	step++;

	struct Location *leftUpP = getLeftUp(p);
	validSet[step] = leftUpP;
	step++;

	//��������Χ�ĵ㣬��������Ѿ���ֵ��������������ڵ��path
	// ������е�û��ֵ������Ҫ���Ժ����������ǵ�ֵ
	int i, tmp, min = 100;
	for (i = 0; i < step; i++) {
		if (validSet[i]->path >= 0) {
			tmp = validSet[i]->path;
			if (min > tmp) {
				min = tmp;
			}
		}
	}
	if (min < 100) {
		p->path = min + 1;
	}
	else {
		p->path += 1;
	}
	return p->path;
}

int isInCircle(struct Location *p)	//�ж��Ƿ��Ѿ���è��ס
{
	int step = 0;
	//left point
	struct Location *validSet[6];

	struct Location *leftP = getLeft(p);
	validSet[step] = leftP;
	step++;

	struct Location *leftDownP = getLeftDown(p);
	validSet[step] = leftDownP;
	step++;

	struct Location *rightDownP = getRightDown(p);
	validSet[step] = rightDownP;
	step++;

	struct Location *rightP = getRight(p);
	validSet[step] = rightP;
	step++;

	struct Location *rightUpP = getRightUp(p);
	validSet[step] = rightUpP;
	step++;

	struct Location *leftUpP = getLeftUp(p);
	validSet[step] = leftUpP;
	step++;

	//��������Χ�ĵ㣬���������Ƿ���ǽ������ǽ�ڵĵ㣨����ֵ�Ǹ����������ڣ�100.
	int i, num = 0;
	for (i = 0; i < step; i++) {
		if (validSet[i]->path > 100 ||//����ǽ�ڵĵ�
			(validSet[i]->path > -100 &&
			validSet[i]->path < 0) ||
			validSet[i]->path == 100) {//����ǽ
			num++;
		}
	}
	if (num == 6) {
		return 1;
	}
	else {
		return 0;
	}
}

/*void printLocation(struct Location *p)	//�ݲ���Ҫʹ�øú���
{
	int row = getMapRow(p);
	int col = getMapCol(p);
	printf("row = %d, col = %d   \n",
		row + 1, col + 1);
}*/

int isLessThan(struct Location *p1, struct Location *p2)
{
	if (p1->path >= 0 && p2->path >= 0) {
		if (p1->path < p2->path) {
			return 1;
		}
		else
			return 0;
	}
	else if (hasCircle == 1) {
		if (p1->step > p2->step) {
			return 1;
		}
		else
			return 0;
	}
	else {
		int m1 = -p1->path;
		int m2 = -p2->path;
		if (m1 < m2) {
			return 1;
		}
		else
			return 0;
	}
}

void produceGameLevel()		//��ʼ������֮һ
{
	int level = rand() % 30 + 10;
	int num = 0;
	while (num < level) {
		int row = rand() % 9;
		int col = rand() % 9;
		if (map[row][col] == 0) {
			num++;
			map[row][col] = 1;
			allPoint[row][col].type = WallType;
		}
	}
}

void initAllPoints()	//��ʼ������֮һ
{
	srand((unsigned)time(0));
	int i, j;
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			allPoint[i][j].row = i + 1;
			allPoint[i][j].col = j + 1;
			allPoint[i][j].type = FreeType;
			allPoint[i][j].step = 0;
			allPoint[i][j].path = -100;
			map[i][j] = 0;
		}
	}
	cat.row = allPoint[4][4].row;
	cat.col = allPoint[4][4].col;
	cat.type = CatType;
	allPoint[4][4].type = CatType;
	map[4][4] = 1;
	cat.path = 888;
	produceGameLevel();
}