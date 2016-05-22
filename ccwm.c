#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

void
usage(void)
{
	printf("Usage: [-h] [-i interface][-e ssid] [-f freq] [-w key]\n"
        "              [-W passphrase] [-s] [-S] [-I] [-d] [-c] [-j]\n");
}

void
ifup(char *ifname)
{
	char *cmdline = malloc(36);
	size_t s1 = strlen(ifname);

	snprintf(cmdline, s1+16, "ip link set %s up", ifname);
	system(cmdline);
	free(cmdline);
}

void
info(char *ifname)
{
	char *cmdline = malloc(43);
	size_t s1 = strlen(ifname);

	snprintf(cmdline, s1+18, "ip addr show dev %s", ifname);
	system(cmdline);
	snprintf(cmdline, s1+13, "iw dev %s link", ifname);
	system(cmdline);
	snprintf(cmdline, s1+13, "iw dev %s info", ifname);
	system(cmdline);
	snprintf(cmdline, s1+23, "iw dev %s get power_save", ifname);
	system(cmdline);
	free(cmdline);
}

void
scan(char *ifname, int type)
{
	char *cmdline = malloc(33);
	size_t s1 = strlen(ifname);
	FILE *fp;
	char line[100];
	char aparray[20][5][100];
	int apnumber = -1;
	int specnumber = 0;
	int i = 0;

	ifup(ifname);

	if (type == 0) {
		snprintf(cmdline, s1+13, "iw dev %s scan", ifname);
		fp = popen(cmdline, "r");
		if (!fp)
			puts("error");

		while (fgets(line, 100, fp) != NULL) {
			if (line[0] == 'B' && line[1] == 'S' && line[2] == 'S' && line[3] == ' ') {
				apnumber++;
				specnumber = 0;
				i = 0;
			}
			else if (strstr(line, "\tSSID: ") || strstr(line, "\tfreq: ") ||
					strstr(line, "\tsignal: ") || strstr(line, "\tcapability: ")) {
				memmove (line, line+1, strlen (line));
				strcpy(aparray[apnumber][specnumber], line);
				specnumber++;
			}
			else if (i < 1) {
				strcpy(aparray[apnumber][4], "Encryption: Open\n");
				i++;
			}
			else if (strstr(line, "\tWPA:\t")) {
				strcpy(aparray[apnumber][specnumber], "Encryption: WPA\n");
			}
			else if (strstr(line, "\tWEP:\t")) {
				strcpy(aparray[apnumber][specnumber], "Encryption: WEP\n");
			}
		}

		pclose(fp);

		for (i = 0; i < apnumber+1; i++) {
			if (i > 0)
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

void
disconnect(char *ifname)
{
	char *cmdline = malloc(53);
	size_t s1 = strlen(ifname);

	snprintf(cmdline, 28, "killall dhcpcd 2> /dev/null");
	system(cmdline);
	snprintf(cmdline, s1+36, "killall wpa_supplicant 2> /dev/null");
	system(cmdline);
	snprintf(cmdline, s1+52, "iw dev %s disconnect 2> /dev/null", ifname);
	system(cmdline);
	snprintf(cmdline, s1+52, "iw dev %s ibss leave 2> /dev/null", ifname);
	system(cmdline);
	free(cmdline);
}

void
connect(char *ifname, char *ssid, char *freq, char *key, int type)
{
	char *cmdline = malloc(100);
	size_t s1 = strlen(ifname);
	size_t s2 = strlen(ssid);
	size_t s3 = strlen(freq);
	size_t s4 = strlen(key);

	ifup(ifname);
	system("killall dhcpcd 2> /dev/null");
	system("killall wpa_supplicant 2> /dev/null");
	snprintf(cmdline, s1+32, "iw dev %s disconnect 2> /dev/null", ifname);
	system(cmdline);
	snprintf(cmdline, s1+32, "iw dev %s ibss leave 2> /dev/null", ifname);
	system(cmdline);

	if (type < 3)
		snprintf(cmdline, s1+25, "iw dev %s set type managed", ifname);
	else
		snprintf(cmdline, s1+22, "iw dev %s set type ibss", ifname);

	system(cmdline);

	if (type == 0) {
		snprintf(cmdline, s1+s2+20, "iw dev %s connect -w %s", ifname, ssid);
		system(cmdline);
	}
	else if (type == 1) {
		snprintf(cmdline, s1+s2+s4+26, "iw dev %s connect -w %s key %s", ifname, ssid, key);
		system(cmdline);
	}
	else if (type == 2) {
		snprintf(cmdline, s1+s2+s4+45, "wpa_supplicant -B -i %s -c <(wpa_passphrase %s %s)",
				ifname, ssid, key);
		system(cmdline);
	}
	else if (type == 3) {
		snprintf(cmdline, s1+s2+s3+20, "iw dev %s ibss join %s %s", ifname, ssid, freq);
		system(cmdline);
	}
	else if (type == 4) {
		snprintf(cmdline, s1+s2+s3+s4+25, "iw dev %s ibss join %s %s key %s",
				ifname, ssid, freq, key);
		system(cmdline);
	}
	else if (type == 5) {
		FILE *fp;
		char line[100];

		snprintf(cmdline, 50, "wpa_passphrase %s %s", ssid, key);
		fp = popen(cmdline, "r");
		if (fp == NULL)
			puts("error");

		char wpaarray[5][100];
		int wpaline = 0;
		while (fgets(line, 100, fp) != NULL) {
			strcpy(wpaarray[wpaline], line);
			wpaline++;
		}

		pclose(fp);

		char *wpaconf = malloc(1000);
		snprintf(wpaconf, 1000, "wpa_supplicant -B -i %s -c <(echo -e \"ap_scan=2\n%s\ttmode=1\n\ttproto=WPA\n\tkey_mgmt=WPA-NONE\n\tpairwise=NONE\n\tgroup=TKIP\n%s%s%s}\")", ifname, wpaarray[0], wpaarray[1], wpaarray[2], wpaarray[3]);

		printf("\nUsing the following command:\n%s\n\n", wpaconf);
		system(wpaconf);
	}
	else {
		free(cmdline);
		return;
	}

	snprintf(cmdline, s1+8, "dhcpcd %s", ifname);
	system(cmdline);

	free(cmdline);
}

int
main(int argc, char *argv[])
{
	int opt;
	int task = 0;
 	char *ifname = "wls3";
	char *ssid = "";
	char *freq = "";
	char *key = "";
	char *passphrase = "";
	uid_t uid=getuid();

	if (argc <= 1) {
		usage();
		return EXIT_FAILURE;
	}

	while ((opt = getopt(argc, argv, "sSIdcjhi:e:f:w:W:")) != -1) {
		switch (opt) {
			case 'i':
				ifname = optarg;
				break;
			case 'e':
				ssid = optarg;
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
			case 'd':
				task = 4;
				break;
			case 'c':
				task = 5;
				break;
			case 'j':
				task = 6;
				break;
			case 'h':
				usage();
				return EXIT_SUCCESS;
			default:
				usage();
				return EXIT_FAILURE;
		}
	}

	if (uid != 0) {
		puts("This program needs to be run with root privileges!\n");
		return EXIT_FAILURE;
	}

	if (task == 0 && ssid[0] != '\0') {
		task = 5;
		if (freq[0] != '\0' )
			task = 6;
	}

	if (task == 1) {
		scan(ifname, 0);
	}
	else if (task == 2) {
		scan(ifname, 1);
	}
	else if (task == 3) {
		info(ifname);
	}
	else if (task == 4) {
		disconnect(ifname);
	}
	else if (task == 5) {
		if (ssid[0] == '\0') {
			printf("You must specify ssid!\n");
		}
		else if (key[0] != '\0') {
			connect(ifname, ssid, freq, key, 1);
		}
		else if (passphrase[0] != '\0') {
			connect(ifname, ssid, freq, passphrase, 2);
		}
		else {
			connect(ifname, ssid, freq, key, 0);
		}
	}
	else if (task == 6) {
		if (ssid[0] == '\0' || freq[0] == '\0') {
			printf("You must specify ssid and freq!\n");
		}
		else if (key[0] != '\0') {
			connect(ifname, ssid, freq, key, 4);
		}
		else if (passphrase[0] != '\0') {
			connect(ifname, ssid, freq, passphrase, 5);
		}
		else {
			connect(ifname, ssid, freq, key, 3);
		}
	}

	return EXIT_SUCCESS;
}
