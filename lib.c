
#define _GNU_SOURCE 1
#include <signal.h>
#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <link.h>
#include <string.h>
#include <limits.h>
#include <memory.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/sysinfo.h>
#include "sys/times.h"
#include "sys/vtimes.h"
#include "stdlib.h"
#include "string.h"
#define XIBUGGER_CMD_LEN 10

       
void eatendline(void)
{
    while(getchar()!='\n'){
        ;
    }
}


int procprint(const char *format, ...)
{
    va_list ap;
    fprintf(stdout, "[%d] ", getpid());
    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
    return 0;// return count of printed char maybe better
}


void cmd_help(void)
{
    printf(
        
        "p--la memoire utilisée par le processus \n"
        "f--les symbole globales \n"
        "h-- les fichiers ouverts par le processus   \n"
        "t-- les appels des fonctions du programme actuel \n"
        "i-- les infos du processus \n"
        "f--la liste des chemins des objets partagés chargé par le programme\n"
        "c -- nombre d'appel malloc et calloc \n"
    );
}

size_t cnt1 = 0;
size_t cnt = 0;
size_t cnt2 = 0;
size_t cnt3 = 0;
       
void* (*func1)(size_t) = NULL;
void free(void * add)
{
	cnt1 +=1;

	if(func1 == NULL)
	{

		func1 = (void*(*)(size_t)) dlsym( RTLD_NEXT, "free");
		if(!func1)
			abort();

	}
	
}


void* (*func)(size_t) = NULL;
void * malloc(size_t size)
{
	cnt +=1;
	static void * (* fptr)() = 0;
	char             buffer[50];

	if (fptr == 0) {
			fptr = (void * (*)())dlsym(RTLD_NEXT, "malloc");
			if (fptr == NULL) {
					(void) printf("dlopen: %s\n", dlerror());
					return (0);
			}
	}

	(void) sprintf(buffer, "malloc: %#x bytes\n", size);
	(void) write(1, buffer, strlen(buffer));
	return ((*fptr)(size));
}


/*

void* (*func2)(size_t,size_t) = NULL;
void * calloc(size_t size,size_t size2)
{
	cnt2+=1;

	if(func == NULL)
	{

		func2 = (void*(*)(size_t,size_t)) dlsym( RTLD_NEXT, "calloc");
		if(!func2)
			abort();

	}
	return func2(size,size2);
}





void* (*func3)(void(*),size_t) = NULL;
void * realloc(void *pointer,size_t size2)
{
	cnt3+=1;

	if(func3 == NULL)
	{

		func3= (void*(*)(void(*),size_t)) dlsym( RTLD_NEXT, "realloc");
		if(!func3)
			abort();

	}
	return func3(pointer, size2);
}

*/
int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getValue(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}
int getValue_2(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}
static clock_t lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;

void init2(){
    FILE* file;
    struct tms timeSample;
    char line[128];

    lastCPU = times(&timeSample);
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;

    file = fopen("/proc/cpuinfo", "r");
    numProcessors = 0;
    while(fgets(line, 128, file) != NULL){
        if (strncmp(line, "processor", 9) == 0) numProcessors++;
    }
    fclose(file);
}

double getCurrentValue_cpu(){
    struct tms timeSample;
    clock_t now;
    double percent;

    now = times(&timeSample);
    if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
        timeSample.tms_utime < lastUserCPU){
        //Overflow detection. Just skip this value.
        percent = -1.0;
    }
    else{
        percent = (timeSample.tms_stime - lastSysCPU) +
            (timeSample.tms_utime - lastUserCPU);
        percent /= (now - lastCPU);
        percent /= numProcessors;
        percent *= 100;
    }
    lastCPU = now;
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;

    return percent;
}



void cpu(){
    
     int result,result2, cpu, cpu2;
     printf("la memoire utilisée par le processus");
     result=	getValue();
     printf(" Virtual Memory currently used by current process:%d\n",result);
      //memoire totale virtuel
      
     struct sysinfo memInfo;
     sysinfo (&memInfo);
     long long totalVirtualMem = memInfo.totalram;

    totalVirtualMem += memInfo.totalswap;
    totalVirtualMem *= memInfo.mem_unit;

    //memoir utulisee

    long long virtualMemUsed = memInfo.totalram - memInfo.freeram;
 
    virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
    virtualMemUsed *= memInfo.mem_unit;
 
    printf("total virtual memory %lld\n", totalVirtualMem );
    printf(" virtual memory used%lld\n", totalVirtualMem );
    
   //Total Physical Memory (RAM):
   
   long long totalPhysMem = memInfo.totalram;
   totalPhysMem *= memInfo.mem_unit;
   printf(" total memory physic%lld\n", totalPhysMem  );


     //Physical Memory currently used:
     
     long long physMemUsed = memInfo.totalram - memInfo.freeram;
     physMemUsed *= memInfo.mem_unit;
     printf(" total memory physic currently used %lld\n", physMemUsed   );
     result2= getValue_2();
     printf(" total memory physic currently used by the current process%d \t\n", result2   );
     
     //print current cpu:

    cpu2=getCurrentValue_cpu();
    printf(" the current cpu used by the process %d \t\n", cpu2   );
}
      
      
      
      


   void fonnm()
   {
   struct stat sb;
   char *buf;
   ssize_t nbytes, bufsiz;
      if (lstat("/proc/self/exe", &sb) == -1) 
      {
        perror("lstat");
        exit(EXIT_FAILURE);
      }

    /* Ajouter un à la taille du lien, pour pouvoir déterminer si le tampon
       renvoyé par readlink() a été tronqué. */

      bufsiz = sb.st_size + 1;
 
    /* Certains liens symboliques magiques dans (par exemple) /proc et /sys
       indiquent 'st_size' comme zéro. Dans ce cas, prendre PATH_MAX
       comme estimation « acceptable ». */


       if (sb.st_size == 0)
             bufsiz = PATH_MAX;
            buf = malloc(bufsiz);


    if (buf == NULL) 
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

      nbytes = readlink("/proc/self/exe", buf, bufsiz);


     if (nbytes == -1) 
     {
        perror("readlink");
        exit(EXIT_FAILURE);
    }

    //printf("'%s' pointe vers '%.*s'\n", "/proc/self/exe", (int) nbytes, buf);
    /* Si la valeur renvoyée était égale à la taille du tampon, la cible du
       lien était plus grande que prévu (peut-être parce que la cible
       a changé entre l'appel à lstat() et l'appel à readlink()).
       Avertir l'utilisateur que la cible renvoyée peut avoir
       été tronquée. */


   if (nbytes == bufsiz)
          printf("(Il se peut que le tampon renvoyé ait été tronqué)\n");
      
              
         const char* str1 = "nm -A ";
         char buffer[sizeof(str1)+sizeof(buf)];
         strcat(strcpy(buffer, str1), buf);
         printf("%s\n", buffer);
         system(buffer);
         free(buf);
              exit(1);
      }
      
      
 void fich()
   { printf(" les fichiers ouvert par le processus :\n");
    system(" ls -l /proc/self/fd");
   }
   
   
   
 static int callback(struct dl_phdr_info *info, size_t size, void *data)
   {
    int j;
    printf("name=%s (%d segments)\n", info->dlpi_name,info->dlpi_phnum);

     for (j = 0; j < info->dlpi_phnum; j++)
     printf("\t\t header %2d: address=%10p\n", j,(void *) (info->dlpi_addr + info->dlpi_phdr[j].p_vaddr));
     return 0;
   }
      
      //etat de la pile
      
  void track (int sig)
    {
        printf("les appels des fonctions du programme actuel: \n");
        char **strings;
	void * addrs[10];
	int cnt = backtrace((void**)&addrs, 10);
	int i;

	    for(i = 0; i < cnt; i++)
	     {
		printf("%d == %p\n", i, addrs[i]);
	     }

            strings=backtrace_symbols(addrs, cnt);
  
            if (strings == NULL)
               {
               perror("backtrace_symbols");
               exit(EXIT_FAILURE);
               }

            for (int j = 0; j < cnt; j++)
              printf("%s\n", strings[j]);
              
              
                 free(strings);
        }
      
      //info processus
   
   
   void info()
   { 
    system("cat /proc/$PPID/status");
     }
     
     
     void shlib(void)
      {
        printf("la liste des chemins des objets partagés chargé par le programme\n");
   	dl_iterate_phdr(callback, NULL);
       }	
      
      
      //les signeaux d'erreurs

  
   void sign(int sig)
   { if(sig==SIGUSR1)
       {
         printf("blocked by SIGUSR1\n\n");
         
       }
      
      
      
else if(sig=SIGTRAP)
        {
        printf( " blocked by SIGTRAP ,Trace/breakpoint trap");
        }
else
   {
      printf( "blocked  by SIGUSR2\n\n"); 

} 
      char cmd[XIBUGGER_CMD_LEN]="";  
      cmd_help();

        while(1){
        procprint("");
        if( EOF == scanf("%s", cmd) ){
            procprint("done\n");
            break;
        }
        if( !strncmp(cmd, "p", XIBUGGER_CMD_LEN) ){ 
            cpu();
            eatendline();
            continue; 
        }
        
        else if( !strncmp(cmd, "f", XIBUGGER_CMD_LEN) ){ 
              fonnm();
              eatendline();
              continue;
            }
            
        else if( !strncmp(cmd, "h", XIBUGGER_CMD_LEN) ){ 
            fich();
            eatendline();
            continue; 
        }
        
        else if( !strncmp(cmd, "t", XIBUGGER_CMD_LEN) ){ 
             track(sig);
             eatendline();
             continue;
             }
             
       else if( !strncmp(cmd, "s", XIBUGGER_CMD_LEN) ){ 
            shlib();
            eatendline();
            continue; 
        }
             
      else if( !strncmp(cmd, "i", XIBUGGER_CMD_LEN) ){ 
            info();
            eatendline();
            continue; 
            }
              
     else if( !strncmp(cmd, "c", XIBUGGER_CMD_LEN) )
    { printf("\nCount malloc = %lu\n", cnt);
	printf("\nCount free = %lu\n", cnt1);
            break;}
     } 
      
   }  
   
     
     
     
     
     
      void sign1(int sig)
   { if(sig==SIGSEGV)
       {
         printf("blocked by SIGSEGV \n\n");
         
       }
      
      
      
else {
      printf( "blocked  by SIGFPE\n\n"); 

} 
      char cmd[XIBUGGER_CMD_LEN]="";  
      cmd_help();

        while(1){
        procprint("");
        if( EOF == scanf("%s", cmd) ){
            procprint("done\n");
            break;
        }
        if( !strncmp(cmd, "p", XIBUGGER_CMD_LEN) ){ 
            cpu();
            eatendline();
            continue; 
        }
        
        else if( !strncmp(cmd, "f", XIBUGGER_CMD_LEN) ){ 
              fonnm();
              eatendline();
              continue;
            }
            
        else if( !strncmp(cmd, "h", XIBUGGER_CMD_LEN) ){ 
            fich();
            eatendline();
            continue; 
        }
        
        else if( !strncmp(cmd, "t", XIBUGGER_CMD_LEN) ){ 
             track(sig);
             eatendline();
             continue;
             }
             
       else if( !strncmp(cmd, "s", XIBUGGER_CMD_LEN) ){ 
            shlib();
            eatendline();
            continue; 
        }
             
      else if( !strncmp(cmd, "i", XIBUGGER_CMD_LEN) ){ 
            info();
            eatendline();
            continue; 
            }
              
     else if( !strncmp(cmd, "c", XIBUGGER_CMD_LEN) )
    { printf("\nCount malloc = %lu\n", cnt);
	printf("\nCount free = %lu\n", cnt1);
            break;
            
     } 
      
   }  
   
 }    
     
      
 
    
int mylib () __attribute__((constructor));

   int mylib()
       {
	signal(SIGSEGV, sign1);
	signal(SIGUSR1, sign);
	signal(SIGUSR2, sign);
	signal(SIGFPE, sign1);
	signal(SIGTRAP, sign);
	return 0;
       }

/*__attribute__((destructor))

    void end()
       { 
	printf("\nCount malloc = %lu\n", cnt);
	printf("\nCount free = %lu\n", cnt1);
	//printf("Count  calloc= %lu\n", cnt2);
	//printf("Count  realloc= %lu\n", cnt3);
       }*/


