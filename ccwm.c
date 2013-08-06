#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define LINE_MAX 100

char **global_argv;

void usage() {
	printf("Usage: %s [options]\n\n", global_argv[0]);

	puts("-h              Show help\n");

	puts("-i [interface]  Set interface (default: wlan0)");
	puts("-e [ssid]       Set SSID");
	puts("-f [freq]       Set frequency");
	puts("-w [key]        Set WEP key");
	puts("-W [passphrase] Set WPA passphrase\n");

	puts("-s              Scan access points");
	puts("-S              Scan access points verbosely");
	puts("-I              Show interface info\n");

	puts("-c              Connect to ESS access point (default if ssid is set)");
	puts("-j              Join to IBSS network");
}

void ifup(char *ifname) {
	char *cmdline = "";
	cmdline = malloc(36);
	size_t s1 = strlen(ifname);
	snprintf(cmdline, s1+16, "ip link set %s up", ifname);
	system(cmdline);
	free(cmdline);
}

void info(char *ifname) {
	char *cmdline = "";
	cmdline = malloc(43);
	size_t s1 = strlen(ifname);
	snprintf(cmdline, s1+13, "iw dev %s info", ifname);
	system(cmdline);
	snprintf(cmdline, s1+13, "iw dev %s link", ifname);
	system(cmdline);
	snprintf(cmdline, s1+23, "iw dev %s get power_save", ifname);
	system(cmdline);
	free(cmdline);
}

void scan(char *ifname, int type) {
	char *cmdline = "";
	cmdline = malloc(33);
	size_t s1 = strlen(ifname);

	if (type == 0) {
		FILE *fp;
		char line[LINE_MAX];

		snprintf(cmdline, s1+13, "iw dev %s scan", ifname);
		fp = popen(cmdline, "r");
		if (fp == NULL)
			puts("error");

		char aparray[20][5][100];
		int specnumber = 0;
		int apnumber = -1;
		int x = 0;
		while (fgets(line, LINE_MAX, fp) != NULL) {
			if (line[0] == 'B' && line[1] == 'S' && line[2] == 'S' && line[3] == ' ') {
				apnumber++;
				specnumber = 0;
				x = 0;
			}

			if (strstr(line, "\tSSID: ") || strstr(line, "\tfreq: ") ||
					strstr(line, "\tsignal: ") || strstr(line, "\tcapability: ")) {
				memmove (line, line+1, strlen (line));
				strcpy(aparray[apnumber][specnumber], line);
				specnumber++;
			}
			else if (x < 1) {
				strcpy(aparray[apnumber][4], "Encryption: Open\n");
				x++;
			}
			else if (strstr(line, "\tWPA:\t")) {
				strcpy(aparray[apnumber][specnumber], "Encryption: WPA\n");
				specnumber++;
			}
			else if (strstr(line, "\tWEP:\t")) {
				strcpy(aparray[apnumber][specnumber], "Encryption: WEP\n");
				specnumber++;
			}
		}

		pclose(fp);

		int i = 0;
		int y = 0;
		for (i = 0; i < apnumber+1; i++) {
			if (y++ > 0)
				puts("");
			printf("%s%s%s%s%s",
					aparray[i][3], aparray[i][1], aparray[i][0], aparray[i][2], aparray[i][4]);
		}

	}
	else {
		snprintf(cmdline, s1+13, "iw dev %s scan", ifname);
		system(cmdline);
	}

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
		system(cmdline);
	}
	else {
		snprintf(cmdline, s1+22, "iw dev %s set type ibss", ifname);
		system(cmdline);
	}

	if (type == 0) {
		snprintf(cmdline, s1+s2+20, "iw dev %s connect -w %s", ifname, essid);
		system(cmdline);
		snprintf(cmdline, s1+8, "dhcpcd %s", ifname);
		system(cmdline);
	}
	else if (type == 1) {
		snprintf(cmdline, s1+s2+s4+26, "iw dev %s connect -w %s key %s", ifname, essid, key);
		system(cmdline);
		snprintf(cmdline, s1+8, "dhcpcd %s", ifname);
		system(cmdline);
	}
	else if (type == 2) {
		snprintf(cmdline, s1+s2+s4+45, "wpa_supplicant -B -i %s -c <(wpa_passphrase %s %s)",
				ifname, essid, key);
		system(cmdline);
		snprintf(cmdline, s1+8, "dhcpcd %s", ifname);
		system(cmdline);
	}
	else if (type == 3) {
		snprintf(cmdline, s1+s2+s3+20, "iw dev %s ibss join %s %s", ifname, essid, freq);
		system(cmdline);
		snprintf(cmdline, s1+8, "dhcpcd %s", ifname);
		system(cmdline);
	}
	else if (type == 4) {
		snprintf(cmdline, s1+s2+s3+s4+25, "iw dev %s ibss join %s %s key %s",
				ifname, essid, freq, key);
		system(cmdline);
		snprintf(cmdline, s1+8, "dhcpcd %s", ifname);
		system(cmdline);
	}
	else if (type == 5) {
		printf("Join to WPA encrypted IBSS not yet supported!\n");
	}

	free(cmdline);
}

int main(int argc, char *argv[]) {
	global_argv = argv;
	int opt;
	int task = 0;

	char *ifname = "wlan0";
	char *essid = "";
	char *freq = "";
	char *key = "";
	char *passphrase = "";

	uid_t uid=getuid();
	if (uid != 0) {
		puts("\nThis program needs to be run with root privileges!\n");
		/* return 1; */
	}

	if (argc <= 1) {
		usage();
		return 0;
	}

	while ((opt = getopt(argc, argv, "sSIcjhi:e:f:w:W:")) != -1) {
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
			case 'w':
				key = optarg;
				break;
			case 'W':
				passphrase = optarg;
				break;
			case 's':
				task = 1;
				break;
			case 'S':
				task = 2;
				break;
			case 'I':
				task = 3;
				break;
			case 'c':
				task = 4;
				break;
			case 'j':
				task = 5;
				break;
			case 'h':
			default:
				usage();
				return 0;
				break;
		}
	}

	if (task == 0 && essid[0] != '\0') {
		task = 4;
	}

	if (task == 1) {
		ifup(ifname);
		scan(ifname, 0);
	}
	else if (task == 2) {
		ifup(ifname);
		scan(ifname, 1);
	}
	else if (task == 3) {
		ifup(ifname);
		info(ifname);
	}
	else if (task == 4) {
		if (essid[0] == '\0') {
			printf("You must specify ssid!\n");
		}
		else if (key[0] != '\0') {
			ifup(ifname);
			connect(ifname, essid, freq, key, 1);
		}
		else if (passphrase[0] != '\0') {
			ifup(ifname);
			connect(ifname, essid, freq, passphrase, 2);
		}
		else {
			ifup(ifname);
			connect(ifname, essid, freq, key, 0);
		}
	}
	else if (task == 5) {
		if (essid[0] == '\0' || freq[0] == '\0') {
			printf("You must specify ssid and freq!\n");
		}
		else if (key[0] != '\0') {
			ifup(ifname);
			connect(ifname, essid, freq, key, 4);
		}
		else if (passphrase[0] != '\0') {
			ifup(ifname);
			connect(ifname, essid, freq, key, 5);
		}
		else {
			ifup(ifname);
			connect(ifname, essid, freq, key, 3);
		}
	}

	return 0;
}
