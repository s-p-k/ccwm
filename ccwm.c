#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

char **global_argv;

void usage() {
	printf("Usage: %s [options]\n\n", global_argv[0]);

	puts("-h              Show help\n");
	puts("-i [interface]  Set interface (default: wlan0)");
	puts("-e [essid]      Set essid");
	puts("-s              Scan access points\n");
	puts("-I              Show info");
}

void ifup(char *ifname) {
	char *cmdline = "";
	cmdline = malloc(36);
	size_t s1 = strlen(ifname);
	snprintf(cmdline, s1+16, "ip link set %s up", ifname);
	printf("debug: %s\n", cmdline);
	system(cmdline);
	free(cmdline);
}

void scan(char *ifname) {
	char *cmdline = "";
	cmdline = malloc(33);
	size_t s1 = strlen(ifname);
	snprintf(cmdline, s1+13, "iw dev %s scan", ifname);
	printf("debug: %s\n", cmdline);
	system(cmdline);
	free(cmdline);
}

void info(char *ifname) {
	char *cmdline = "";
	cmdline = malloc(43);
	size_t s1 = strlen(ifname);
	snprintf(cmdline, s1+13, "iw dev %s info", ifname);
	printf("debug: %s\n", cmdline);
	system(cmdline);
	snprintf(cmdline, s1+13, "iw dev %s link", ifname);
	printf("debug: %s\n", cmdline);
	system(cmdline);
	snprintf(cmdline, s1+23, "iw dev %s get power_save", ifname);
	printf("debug: %s\n", cmdline);
	system(cmdline);
	free(cmdline);
}

void connect(char *ifname, char *essid) {
	char *cmdline = "";
	cmdline = malloc(36);
	size_t s1 = strlen(ifname);
	size_t s2 = strlen(essid);
	snprintf(cmdline, s1+16, "ip link set %s up", ifname);
	printf("debug: %s\n", cmdline);
	system(cmdline);
	free(cmdline);
}

int main(int argc, char *argv[]) {
	global_argv = argv;
	int opt;
	char *ifname = "wlan0";

	if (argc <= 1) {
		usage();
	}

	while ((opt = getopt(argc, argv, "hsIi:")) != -1) {
		switch (opt) {
			case 'i':
				ifname = optarg;
				break;
			case 'I':
				ifup(ifname);
				info(ifname);
				break;
			case 's':
				ifup(ifname);
				scan(ifname);
				break;
			case 'h':
			default:
				usage();
				break;
		}
	}
	return 0;
}
