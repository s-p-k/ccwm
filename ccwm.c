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
	puts("-f [freq]       Set frequency");
	puts("-k [key]        Set key/passphrase\n");

	puts("-s              Scan access points");
	puts("-S              Scan access points verbosely");
	puts("-I              Show interface info\n");

	puts("-o              Connect to ESS (open) access point");
	puts("-a              Connect to IBSS (ad-hoc) access point");
	puts("-w              Connect to WEP encrypted access point");
	puts("-W              Connect to WPA encrypted access point");
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

void scan(char *ifname, int type) {
	char *cmdline = "";
	cmdline = malloc(33);
	size_t s1 = strlen(ifname);

	if (type == 0) {
		snprintf(cmdline, s1+31, "iw dev %s scan | awk -f scan.awk", ifname);
		printf("debug: %s\n", cmdline);
		system(cmdline);
	}
	else {
		snprintf(cmdline, s1+13, "iw dev %s scan", ifname);
		printf("debug: %s\n", cmdline);
		system(cmdline);
	}

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

void connect(char *ifname, char *essid, int type) {
	system("killall dhcpcd 2> /dev/null");
	char *cmdline = "";
	cmdline = malloc(93);
	size_t s1 = strlen(ifname);
	size_t s2 = strlen(essid);

	if (type < 3) {
		snprintf(cmdline, s1+25, "iw dev %s set type managed", ifname);
		printf("debug: %s\n", cmdline);
		system(cmdline);
	}
	else {
		snprintf(cmdline, s1+22, "iw dev %s set type ibss", ifname);
		printf("debug: %s\n", cmdline);
		system(cmdline);
	}

	if (type == 0) {
		snprintf(cmdline, s1+s2+20, "iw dev %s connect -w %s", ifname, essid);
		printf("debug: %s\n", cmdline);
		system(cmdline);
		snprintf(cmdline, s1+8, "dhcpcd %s", ifname);
		printf("debug: %s\n", cmdline);
		system(cmdline);
	}
	else if (type == 1) {
		snprintf(cmdline, s1+s2+31, "iw dev %s connect -w %s keys 12345", ifname, essid);
		printf("debug: %s\n", cmdline);
		system(cmdline);
		snprintf(cmdline, s1+8, "dhcpcd %s", ifname);
		printf("debug: %s\n", cmdline);
		system(cmdline);
	}
	else if (type == 2) {
		snprintf(cmdline, s1+s2+53, "wpa_supplicant -B -i %s -c <(wpa_passphrase %s 12345678)",
									ifname, essid);
		printf("debug: %s\n", cmdline);
		system(cmdline);
		snprintf(cmdline, s1+8, "dhcpcd %s", ifname);
		printf("debug: %s\n", cmdline);
		system(cmdline);
	}
	else {
		snprintf(cmdline, s1+s2+24, "iw dev %s ibss join %s 2457", ifname, essid);
		printf("debug: %s\n", cmdline);
		system(cmdline);
		snprintf(cmdline, s1+8, "dhcpcd %s", ifname);
		printf("debug: %s\n", cmdline);
		system(cmdline);
	}

	free(cmdline);
}

int main(int argc, char *argv[]) {
	global_argv = argv;
	int opt;
	char *ifname = "wlan0";
	char *essid = "";
	char *key = "";
	char *freq = "";

	if (argc <= 1) {
		usage();
	}

	while ((opt = getopt(argc, argv, "IsSowWahi:e:k:f:")) != -1) {
		switch (opt) {
			case 'i':
				ifname = optarg;
				break;
			case 'e':
				essid = optarg;
				break;
			case 'k':
				key = optarg;
				break;
			case 'f':
				freq = optarg;
				break;
			case 'I':
				ifup(ifname);
				info(ifname);
				break;
			case 's':
				ifup(ifname);
				scan(ifname, 0);
				break;
			case 'S':
				ifup(ifname);
				scan(ifname, 1);
				break;
			case 'o':
				ifup(ifname);
				connect(ifname, essid, 0);
				break;
			case 'w':
				ifup(ifname);
				connect(ifname, essid, 1);
				break;
			case 'W':
				ifup(ifname);
				connect(ifname, essid, 2);
				break;
			case 'a':
				ifup(ifname);
				connect(ifname, essid, 3);
				break;
			case 'h':
			default:
				usage();
				break;
		}
	}

	return 0;
}
