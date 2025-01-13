#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

long getkbs(char const* field, char* line, char* startofline) {
    int pos = strlen(field) + 1;
    char curr = line[pos];
    while (curr == ' ') {
	pos ++;
	curr = line[pos];
    }
    char totalint[256]; 
    int intpos = 0;
    curr = line[pos];
    while (curr != ' ') {
	totalint[intpos] = curr;
	intpos ++;
	pos ++;
	curr = line[pos];
    }
    totalint[intpos] = '\0';
    long kbs = atol(totalint);
    return kbs;
}

void getmemusage(void) {
    char const* const fileName = "/proc/meminfo";
    char const* const MemTotal = "MemTotal:";
    char const* const MemFree = "MemFree:";
    FILE* file = fopen(fileName, "r");
    char line[256];
    char startofline[256];
    long totalkbs;
    long freekbs;
    while (fgets(line, sizeof(line), file)) {
	strncpy(startofline, line, strlen(line));	
	startofline[strlen(MemTotal)] = '\0';
	if(!strcmp(startofline, MemTotal)) {
	    totalkbs = getkbs(MemTotal, line, startofline);
	}
	strncpy(startofline, line, strlen(line));	
	startofline[strlen(MemFree)] = '\0';
	if(!strcmp(startofline, MemFree)) {
	    freekbs = getkbs(MemFree, line, startofline);
	}
      }
    
    double totalmbs = totalkbs / 1024.0;
    double freembs = freekbs / 1024.0;
    double usedmbs = totalmbs - freembs;
    double percent = (usedmbs / totalmbs) * 100.0; 

    fclose(file);
    printf("RAM: %.2f \n", percent);
  }

int main() {
  getmemusage();
  return 0;
}
