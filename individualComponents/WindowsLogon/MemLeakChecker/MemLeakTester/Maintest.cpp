#include <Windows.h>

#include "..\MemLeakChecker\MainLib.h"
#include <stdlib.h>
#include <stdio.h>

void* foo(void* a, void* b)
{
	void* o = malloc(24);
	for(int i = 0 ; i < 24 ; ++i)
	{
		((unsigned char*)o)[i] = ((unsigned char*)a)[i] + ((unsigned char*)b)[i];
	}
	return o;
}

void* bar(void* a, void* b)
{
	void* o = malloc(24); malloc(8);
	return o;
}

int leakyFunction()
{
	void *a, *b, *c, *d, *e, *f, *g, *h;

	MemLeak_SetLocationID(201);
	a = malloc(15);
	MemLeak_SetLocationID(202);
	b = malloc(24);
	MemLeak_SetLocationID(203);
	c = malloc(24);
	MemLeak_SetLocationID(204);
	d = malloc(67);
	MemLeak_SetLocationID(205);
	e = malloc(54);
	MemLeak_SetLocationID(206);
	f = foo(b, c);
	MemLeak_SetLocationID(207);
	g = foo(bar(b, c), f);
	MemLeak_SetLocationID(208);
	h = bar(f, g);

	MemLeak_CheckMemChanges();

	free(a);
	free(b);
	//free(c);
	free(d);
	free(e);
	free(f);
	free(g);
	free(h);


	return 0;
}

int main()
{
	MemLeak_Init(L"\\\\.\\pipe\\MTMemDebug");

	MemLeak_SetLocationID(0);


	MemLeak_CheckMemChanges();


	void* vpC = malloc(1024);

	MemLeak_CheckMemChanges();


	free(vpC);

	MemLeak_CheckMemChanges();

	while(true)
	{
		leakyFunction();
		MemLeak_CheckMemChanges();
		Sleep(250);
	}

	getchar();

}