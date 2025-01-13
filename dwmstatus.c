/*
 * Copy me if you can.
 * by 20h
 */

#define _BSD_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>


#include <X11/Xlib.h>

char *tzLA = "America/Los_Angeles";
char *tzpt = "PT";
char *tzberlin = "America/Los_Angeles";
//Interface for ip fetch
const char *iface = "wlan0";

static Display *dpy;

char *
smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

void
settz(char *tzname)
{
	setenv("TZ", tzname, 1);
}

char *
mktimes(char *fmt, char *tzname)
{
	char buf[129];
	time_t tim;
	struct tm *timtm;

	settz(tzname);
	tim = time(NULL);
	timtm = localtime(&tim);
	if (timtm == NULL)
		return smprintf("");

	if (!strftime(buf, sizeof(buf)-1, fmt, timtm)) {
		fprintf(stderr, "strftime == 0\n");
		return smprintf("");
	}

	return smprintf("%s", buf);
}

char *
getipv4addr(const char *iface) 
{
  struct ifaddrs *ifaddr, *ifa;
  char ip[INET_ADDRSTRLEN];
  
  if(getifaddrs(&ifaddr) == -1) {
	perror("getifaddrs");
	return smprintf("Call to getifaddrs failed");
  }  

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {	
	  if (ifa->ifa_addr == NULL)
	    continue;

	  int family = ifa->ifa_addr->sa_family;
	  if (family == AF_INET && strcmp(ifa->ifa_name, iface) == 0) {

		void *addr = &( (struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
		
		if(inet_ntop(family, addr, ip, sizeof(ip)) == NULL) {
		  perror("inet_ntop");
		  continue;
		}
		freeifaddrs(ifaddr);
		return smprintf("%s", ip);
	  }
  }

  freeifaddrs(ifaddr);
  return smprintf("No IP Found");
}

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

char * getmemusage(void) {
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
    /* may check feof here to make a difference between eof and io failure -- network
       timeout for instance */

    fclose(file);
    return smprintf("RAM: %.2f", percent);
  }

void
setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

char *
loadavg(void)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0)
		return smprintf("");

	return smprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}

char *
readfile(char *base, char *file)
{
	char *path, line[513];
	FILE *fd;

	memset(line, 0, sizeof(line));

	path = smprintf("%s/%s", base, file);
	fd = fopen(path, "r");
	free(path);
	if (fd == NULL)
		return NULL;

	if (fgets(line, sizeof(line)-1, fd) == NULL) {
		fclose(fd);
		return NULL;
	}
	fclose(fd);

	return smprintf("%s", line);
}

char *
getbattery(char *base)
{
	char *co, status;
	int descap, remcap;

	descap = -1;
	remcap = -1;

	co = readfile(base, "present");
	if (co == NULL)
		return smprintf("");
	if (co[0] != '1') {
		free(co);
		return smprintf("not present");
	}
	free(co);

	co = readfile(base, "charge_full_design");
	if (co == NULL) {
		co = readfile(base, "energy_full_design");
		if (co == NULL)
			return smprintf("");
	}
	sscanf(co, "%d", &descap);
	free(co);

	co = readfile(base, "charge_now");
	if (co == NULL) {
		co = readfile(base, "energy_now");
		if (co == NULL)
			return smprintf("");
	}
	sscanf(co, "%d", &remcap);
	free(co);

	co = readfile(base, "status");
	if (!strncmp(co, "Discharging", 11)) {
		status = '-';
	} else if(!strncmp(co, "Charging", 8)) {
		status = '+';
	} else {
		status = '?';
	}

	if (remcap < 0 || descap < 0)
		return smprintf("invalid");

	return smprintf("%.0f%%%c", ((float)remcap / (float)descap) * 100, status);
}

char *
gettemperature(char *base, char *sensor)
{
	char *co;

	co = readfile(base, sensor);
	if (co == NULL)
		return smprintf("");
	return smprintf("%02.0f°C", atof(co) / 1000);
}

char *
execscript(char *cmd)
{
	FILE *fp;
	char retval[1025], *rv;

	memset(retval, 0, sizeof(retval));

	fp = popen(cmd, "r");
	if (fp == NULL)
		return smprintf("");

	rv = fgets(retval, sizeof(retval), fp);
	pclose(fp);
	if (rv == NULL)
		return smprintf("");
	retval[strlen(retval)-1] = '\0';

	return smprintf("%s", retval);
}

int
main(void)
{
	char *status;
	char *avgs;
	char *bat;
	char *timeLA;
	char *dateLA;
	char *tmpt;
	char *tmbln;
	char *t0;
	char *t1;
	char *kbmap;
	char *surfs;
	char *memes;
	char *ip;
	char *memusage;

	char *battery_emoji = "\xF0\x9F\x94\x8b";
	char *clock_emoji = "\xF0\x9F\x95\x93";
	char *calendar_emoji = "\xF0\x9F\x93\x85";
	char *globe_conn_emoji = "\xF0\x9F\x8C\x90";


	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (;;sleep(30)) {
		avgs = loadavg();
		bat = getbattery("/sys/class/power_supply/BAT0");
		timeLA = mktimes("%I:%M %p", tzLA);
		dateLA = mktimes("%a %B %d, %Y", tzLA);
		tmpt = mktimes("%H:%M", tzpt);
		tmbln = mktimes("KW %W %a %d %b %H:%M %Z %Y", tzberlin);
		kbmap = execscript("setxkbmap -query | grep layout | cut -d':' -f 2- | tr -d ' '");
		surfs = execscript("surf-status");
		memes = execscript("meme-status");
		ip = getipv4addr(iface);
		memusage = getmemusage();
		t0 = gettemperature("/sys/devices/virtual/thermal/thermal_zone0", "temp");
		t1 = gettemperature("/sys/devices/virtual/thermal/thermal_zone1", "temp");

		status = smprintf(" %s%% %s %s %s TEMP:%s|%s %s %s %s %s %s %s",
				 memusage, globe_conn_emoji, ip, memes, t0, t1, battery_emoji, bat, calendar_emoji, dateLA, 
				 clock_emoji, timeLA);
		setstatus(status);

		free(surfs);
		free(memes);
		free(ip);
		free(memusage);
		free(kbmap);
		free(t0);
		free(t1);
		free(avgs);
		free(bat);
		free(timeLA);
		free(dateLA);
		free(tmpt);
		free(tmbln);
		free(status);
	}

	XCloseDisplay(dpy);

	return 0;
}

