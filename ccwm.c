#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

char **global_argv;

void usage() {
	printf("Usage: %s [options] -o/-a/-w/-W\n\n", global_argv[0]);

	puts("-h              Show help\n");

	puts("-i [interface]  Set interface (default: wlan0)");
	puts("-e [ssid]       Set SSID");
	puts("-f [freq]       Set frequency");
	puts("-k [key]        Set key/passphrase\n");

	puts("-s              Scan access points");
	puts("-S              Scan access points verbosely");
	puts("-I              Show interface info\n");

	puts("-o              Connect to ESS access point");
	puts("-a              Connect to IBSS access point");
	puts("-w              Connect to WEP encrypted access point");
	puts("-W              Connect to WPA encrypted access point");
}

void ifup(char *ifname) {
	char *cmdline = "";
	cmdline = malloc(36);
	size_t s1 = strlen(ifname);
	snprintf(cmdline, s1+16, "ip link set %s up", ifname);
	printf("cmd: %s\n", cmdline);
	system(cmdline);
	free(cmdline);
}

void scan(char *ifname, int type) {
	char *cmdline = "";
	cmdline = malloc(33);
	size_t s1 = strlen(ifname);

	if (type == 0) {
		snprintf(cmdline, s1+31, "iw dev %s scan | awk -f scan.awk", ifname);
		printf("cmd: %s\n", cmdline);
		system(cmdline);
	}
	else {
		snprintf(cmdline, s1+13, "iw dev %s scan", ifname);
		printf("cmd: %s\n", cmdline);
		system(cmdline);
	}

	free(cmdline);
}

void info(char *ifname) {
	char *cmdline = "";
	cmdline = malloc(43);
	size_t s1 = strlen(ifname);
	snprintf(cmdline, s1+13, "iw dev %s info", ifname);
	printf("cmd: %s\n", cmdline);
	system(cmdline);
	snprintf(cmdline, s1+13, "iw dev %s link", ifname);
	printf("cmd: %s\n", cmdline);
	system(cmdline);
	snprintf(cmdline, s1+23, "iw dev %s get power_save", ifname);
	printf("cmd: %s\n", cmdline);
	system(cmdline);
	free(cmdline);
}

void connect(char *ifname, char *essid, char *freq, char *key, int type) {
	system("killall dhcpcd 2> /dev/null");
	char *cmdline = "";
	cmdline = malloc(100);
	size_t s1 = strlen(ifname);
	size_t s2 = strlen(essid);
	size_t s3 = strlen(freq);
	size_t s4 = strlen(key);

	if (type < 3) {
		snprintf(cmdline, s1+25, "iw dev %s set type managed", ifname);
		printf("cmd: %s\n", cmdline);
		system(cmdline);
	}
	else {
		snprintf(cmdline, s1+22, "iw dev %s set type ibss", ifname);
		printf("cmd: %s\n", cmdline);
		system(cmdline);
	}

	if (type == 0) {
		snprintf(cmdline, s1+s2+20, "iw dev %s connect -w %s", ifname, essid);
		printf("cmd: %s\n", cmdline);
		system(cmdline);
		snprintf(cmdline, s1+8, "dhcpcd %s", ifname);
		printf("cmd: %s\n", cmdline);
		system(cmdline);
	}
	else if (type == 1) {
		snprintf(cmdline, s1+s2+s4+26, "iw dev %s connect -w %s keys %s", ifname, essid, key);
		printf("cmd: %s\n", cmdline);
		system(cmdline);
		snprintf(cmdline, s1+8, "dhcpcd %s", ifname);
		printf("cmd: %s\n", cmdline);
		system(cmdline);
	}
	else if (type == 2) {
		snprintf(cmdline, s1+s2+s4+45, "wpa_supplicant -B -i %s -c <(wpa_passphrase %s %s)",
									ifname, essid, key);
		printf("cmd: %s\n", cmdline);
		system(cmdline);
		snprintf(cmdline, s1+8, "dhcpcd %s", ifname);
		printf("cmd: %s\n", cmdline);
		system(cmdline);
	}
	else {
		snprintf(cmdline, s1+s2+s3+20, "iw dev %s ibss join %s %s", ifname, essid, freq);
		printf("cmd: %s\n", cmdline);
		system(cmdline);
		snprintf(cmdline, s1+8, "dhcpcd %s", ifname);
		printf("cmd: %s\n", cmdline);
		system(cmdline);
	}

	free(cmdline);
}

int main(int argc, char *argv[]) {
	global_argv = argv;
	int opt;
	char *ifname = "wlan0";
	char *essid = "";
	char *freq = "";
	char *key = "";

	uid_t uid=getuid();
	if (uid != 0) {
		puts("\nThis program needs to be run with root privileges!\n");
		/* return 1; */
	}

	if (argc <= 1) {
		usage();
	}

	while ((opt = getopt(argc, argv, "IsSowWahi:e:f:k:")) != -1) {
		switch (opt) {
			case 'i':
				ifname = optarg;
				break;
			case 'e':
				essid = optarg;
				break;
			case 'f':
				freq = optarg;
				break;
			case 'k':
				key = optarg;
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
				if (essid[0] == '\0') {
					printf("You must specify ssid!\n");
					break;
				}
				ifup(ifname);
				connect(ifname, essid, freq, key, 0);
				break;
			case 'w':
				if (essid[0] == '\0' || key[0] == '\0') {
					printf("You must specify ssid and key!\n");
					break;
				}
				ifup(ifname);
				connect(ifname, essid, freq, key, 1);
				break;
			case 'W':
				if (essid[0] == '\0' || key[0] == '\0') {
					printf("You must specify ssid and passphrase!\n");
					break;
				}
				ifup(ifname);
				connect(ifname, essid, freq, key, 2);
				break;
			case 'a':
				if (essid[0] == '\0' || freq[0] == '\0') {
					printf("You must specify ssid and freq!\n");
					break;
				}
				ifup(ifname);
				connect(ifname, essid, freq, key, 3);
				break;
			case 'h':
			default:
				usage();
				break;
		}
	}

	return 0;
}
