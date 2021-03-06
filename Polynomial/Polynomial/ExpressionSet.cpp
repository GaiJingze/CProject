#include "ExpressionSet.h"
#include "command.h"
#include "IOControl.h"
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define OPERATOR_COLOR WHITE
#define NUMBER_COLOR WHITE
#define VARIABLE_COLOR WHITE
#define INDEX_COLOR YELLOW



int expressionSetsSize = sizeof(ExpressionSets);
int expressionBufferSize = sizeof(ExpresionBuffer);
int globe_ID;

void initExpressionBuffer(ExpresionBuffer *expb)
{
	globe_ID = 1;
	expb->base = (ExpresionBufferElem *)malloc(EXPRESSION_MAX_BUFFER * expressionSetsSize);
	if (expb == NULL) {
		throwError("initExpressionBuffer::fails to initialize expresion buffer. The program will end.\n", RED);
		getchar(); getchar(); getchar(); getchar();
		exit(0);
	}
	expb->top = expb->base;
	expb->maxSize = EXPRESSION_MAX_BUFFER;
	expb->currentID = 0;
	expb->offset = 0;
}

void pushExpressionBuffer(ExpresionBuffer *expb, ExpresionBufferElem exps, bool ifSave)
{
	int stackSize = expb->maxSize;
	if (ifSave && cmd_autoSave == CMD_TRUE) {
		//fwriteExpression_ID(exps, DEFAULT_BUFFER_FILE_ADDRESS);
	}
	if (expb->top - expb->base >= stackSize)
	{
		while (expb->top > expb->base) {
			expb->top--;
			freeExpressionSets(*(expb->top));
		}
	}
	expb->currentID = exps->ID;
	*(expb->top) = exps;
	expb->top++;
	expb->offset = expb->top - expb->base;
}



ExpressionSets *initExpressiosSets(bool hasID, ExpressionSets *exps,  int initSize)
{
	if (exps == NULL) {
		exps = (ExpressionSets *)malloc(expressionSetsSize);
		if (exps == NULL) {
			throwError("initExpressiosSets::fails to initialize expression set. The program will end.\n", RED);
			getchar(); getchar(); getchar(); getchar();
			exit(0);
		}
	}
	exps->base = (ExpressionSetsElem *)malloc(initSize * ExpressionSize);
	if (exps->base == NULL) {
		throwError("initExpressiosSets::fails to initialize expresion set\'s base. The program will end.\n", RED);
		getchar(); getchar(); getchar(); getchar();
		exit(0);
	}
	exps->top = exps->base;
	exps->maxSize = initSize;
	if (hasID == true) {
		exps->ID = globe_ID;
		globe_ID++;
	}
	else {
		exps->ID = 0;
	}
	*(exps->name) = '\0';
	return exps;
}

void pushExpressiosSets(ExpressionSets *exps, ExpressionSetsElem exp)
{
	int stackSize = exps->maxSize;
	if (exps->top - exps->base >= stackSize)
	{
		exps->base = (ExpressionSetsElem *)realloc(exps->base, (stackSize + EXPRESSION_SETS_INCRETMENT) * ExpressionSize);
		if (exps == NULL) {
			throwError("pushExpressiosSets::fails to realloc. The program will end.\n", RED);
			getchar(); getchar(); getchar(); getchar();
			exit(0);
		}
		exps->top = exps->base + stackSize;
		exps->maxSize = stackSize + EXPRESSION_SETS_INCRETMENT;
	}

	*(exps->top) = exp;
	(exps->top)++;
}

ExpressionSetsElem popExpressiosSets(ExpressionSets *exps, ExpressionSetsElem *exp)
{
	if (exps->top == exps->base)
		return NULL;
	if (exp != NULL)
		*exp = *--(exps->top);
	else
		--(exps->top);
	return *(exps->top);
}

void freeExpressionSets(ExpressionSets *&exps)
{
	Expressions *exp;
	if (exps == NULL) 
		return;
	((exps)->top)--;
	while ((exps)->base <= (exps)->top)
	{
		exp = *((exps)->top);
		((exps)->top)--;
		freeExpression(exp);
	}
	free(exps);
}

void caluclateExpression(ExpressionSets *exps)
{
	Expressions *exp, *ptr;
	ExpressionSetsElem *moveable_base = exps->base;
	Poly *poly;
	while (moveable_base < exps->top)
	{
		ptr = exp = *moveable_base;
		while (exp != NULL) {
			poly = exp->son;
			while (poly) {
				if (varValueConfig[poly->var]) {
					exp->coeff *= PolyType (pow(varValue[poly->var], poly->degree));
					if (exp->coeff == 0) {
						//it only has a head
						if (ptr->next == NULL) {
							freeExpression(ptr);
						}
						else {
							ptr->next = exp->next;
							freePolyList((exp->son));
							free(exp);
							exp = ptr->next;

						}
						break;
					}
				}
				poly = poly->son;
			}
			if (exp == NULL) {
				break;
			}
			ptr = exp;
			exp = exp->next;
		}
		moveable_base++;
	}
}


PolyType caluclateExpressionResult(ExpressionSets *exps) {
	Expressions *exp;
	ExpressionSetsElem *moveable_base = exps->base;
	Poly *poly;
	PolyType res = 0, temp;

	while (moveable_base < exps->top)
	{
		exp = *moveable_base;
		while (exp != NULL) {
			poly = exp->son;
			temp = 1;
			while (poly) {
				temp *= PolyType(pow(varValue[poly->var], poly->degree));
				if (temp == 0)
					break;
				poly = poly->son;
			}
			res += temp;
			exp = exp->next;
		}
		moveable_base++;

	}
	return res;
}

void clearVarValue()
{
	for (int i = 0; i < MAXVAR; i++) {
		varValue[i] = 0;
		varValueConfig[i] = false;
	}
}


void printBufferCurrentOffset(ExpresionBuffer *expb)
{
	printExpressionSet(*(expb->base + expb->offset - 1));
}

void printExpressionSet(ExpressionSets *exps)
{
	HANDLE hwdl = GetStdHandle(STD_OUTPUT_HANDLE);
	Expressions *exp = *exps->base;
	Poly *ptr;
	while (exp != NULL) {
		if (cmd_color == CMD_TRUE)
			SetConsoleTextAttribute(hwdl, NUMBER_COLOR);
		if(exp->coeff != 1 || exp->son == NULL)
			printf("%d", exp->coeff);
		ptr = exp->son;
		
		while (ptr) {
			if (cmd_color == CMD_TRUE)
				SetConsoleTextAttribute(hwdl, VARIABLE_COLOR);
			printf("%c", varTable[ptr->var]);
			if (ptr->degree != 1) {
				if (cmd_color == CMD_TRUE)
					SetConsoleTextAttribute(hwdl,INDEX_COLOR);
				printf("^%d", ptr->degree);
			}
			ptr = ptr->son;
		}
		if (cmd_color == CMD_TRUE)
			SetConsoleTextAttribute(hwdl, OPERATOR_COLOR);
		if (exp->next != NULL) {
			if (exp->next->coeff > 0) {
				printf("+");
			}
			else {
				;
			}
		}
			
		exp = exp->next;
	}
	printf("\n");
	if (cmd_color == CMD_TRUE)
		SetConsoleTextAttribute(hwdl, WHITE);
}

void printExpression(Expressions *exp)
{
	HANDLE hwdl = GetStdHandle(STD_OUTPUT_HANDLE);
	Poly *ptr;
	while (exp != NULL) {
		if (cmd_color == CMD_TRUE)
			SetConsoleTextAttribute(hwdl, NUMBER_COLOR);
		if (exp->coeff != 1 || exp->son == NULL)
			printf("%d", exp->coeff);
		ptr = exp->son;

		while (ptr) {
			if (cmd_color == CMD_TRUE)
				SetConsoleTextAttribute(hwdl, VARIABLE_COLOR);
			printf("%c", varTable[ptr->var]);
			if (ptr->degree != 1) {
				if (cmd_color == CMD_TRUE)
					SetConsoleTextAttribute(hwdl, INDEX_COLOR);
				printf("^%d", ptr->degree);
			}
			ptr = ptr->son;
		}
		if (cmd_color == CMD_TRUE)
			SetConsoleTextAttribute(hwdl, OPERATOR_COLOR);
		if (exp->next != NULL) {
			if (exp->next->coeff > 0) {
				printf("+");
			}
			else {
				;
			}
		}

		exp = exp->next;
	}
	printf("\n");
	if (cmd_color == CMD_TRUE)
		SetConsoleTextAttribute(hwdl, WHITE);
}

void printExpressionSet_all(ExpressionSets *exps)
{
	ExpressionSetsElem *movealbe_base = exps->base;
	int i = 0;
	while (movealbe_base < exps->top) {
		printf("base + %d: ", i);
		printExpression(*movealbe_base);
		movealbe_base++;
		i++;
	}
}