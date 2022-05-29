

#include <stdio.h>
#include <signal.h>
int gloubi = 0;

int foo(int a)
{printf("bonjour/n");
	gloubi++;
	if(a == 10)
	{
		char * p = NULL;
			*p = 1;
	
	
	}

foo(a + 1);

}
int main()
{int c=3;
int k=0;
int * p = malloc ( sizeof(int));
	int * p1 = malloc ( sizeof(int));
	int * p5 = malloc ( sizeof(int));
	int * p8 = malloc ( sizeof(int));
	int * p9 = malloc ( sizeof(int));
	free(p1);
	free(p5);
	//raise(SIGFPE);
//int t=c/k;
//raise(SIGSEGV);
foo(c);



return 0;
}
