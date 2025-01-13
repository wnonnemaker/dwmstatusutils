#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

long getkbs(char const* field, char* line, char* startofline) {
    int pos = strlen(field) + 1;
    char curr = line[pos];
    printf("%c \n", curr);
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
	printf("%c \n", curr);
    }
    totalint[intpos] = '\0';
    printf("%s \n", totalint);
    long kbs = atol(totalint);
    return kbs;
}

void getmemusage(void) {
    char const* const fileName = "/proc/meminfo"; /* should check that argc > 1 */
    char const* const MemTotal = "MemTotal:"; /* should check that argc > 1 */
    char const* const MemFree = "MemFree:"; /* should check that argc > 1 */
    FILE* file = fopen(fileName, "r"); /* should check the result */
    char line[256];
    char startofline[256];
    long totalkbs;
    long freekbs;
    while (fgets(line, sizeof(line), file)) {
	strncpy(startofline, line, strlen(line));	
	startofline[strlen(MemTotal)] = '\0';
	if(!strcmp(startofline, MemTotal)) {
	    printf("MATCH! \n");
	    totalkbs = getkbs(MemTotal, line, startofline);
	}
	strncpy(startofline, line, strlen(line));	
	startofline[strlen(MemFree)] = '\0';
	if(!strcmp(startofline, MemFree)) {
	    printf("MATCH!");
	    freekbs = getkbs(MemFree, line, startofline);
	}
      }
    
    printf("total kbs is %d \n", totalkbs);
    printf("free kbs is %d \n", freekbs);
    double totalmbs = totalkbs / 1024.0;
    double freembs = freekbs / 1024.0;
    double usedmbs = totalmbs - freembs;
    printf("total mbs is %.2f \n", totalmbs);
    printf("free mbs is %.2f \n", freembs);
    double percent = (usedmbs / totalmbs) * 100.0; 
    printf("Percent useage is %.2f \n", percent);
    /* may check feof here to make a difference between eof and io failure -- network
       timeout for instance */

    fclose(file);
  }



int main() {
  getmemusage();
  return 0;
}
