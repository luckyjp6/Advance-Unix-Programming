/*
This file should be compile to .so file.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libunwind.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <errno.h>
#include "libpoem.h"
#include "shuffle.h"

#define errquit(m)	{ perror(m); _exit(-1); }

int init () {
	/* the order of code_xxx() in the GOT of chals. please refer to solver.py */
    static int table[] = { 606, 641, 175, 1272, 1276, 286, 249, 502, 284, 208, 280, 541, 206, 786, 1237, 1008, 788, 704, 741, 1045, 1235, 738, 706, 1239, 1152, 849, 775, 1004, 1193, 1156, 771, 1191, 200, 380, 1309, 1136, 847, 877, 808, 57, 1132, 1134, 841, 1348, 1346, 10, 1291, 884, 1293, 1385, 300, 473, 1250, 304, 949, 1254, 1217, 1446, 1409, 1256, 947, 1219, 1407, 1215, 1448, 945, 908, 906, 943, 96, 986, 1213, 180, 1211, 141, 591, 558, 108, 519, 556, 432, 1186, 597, 147, 1149, 430, 92, 1147, 182, 145, 631, 1188, 149, 186, 1284, 684, 238, 680, 511, 273, 234, 102, 271, 550, 513, 799, 711, 1019, 1421, 28, 26, 782, 1184, 1015, 1011, 1013, 391, 317, 315, 230, 866, 1332, 834, 20, 22, 1082, 871, 873, 1260, 1379, 1338, 1221, 429, 466, 1084, 1373, 427, 313, 311, 1377, 1225, 921, 1418, 1451, 1049, 1414, 953, 1160, 990, 157, 568, 1457, 1121, 155, 566, 67, 153, 114, 529, 527, 112, 562, 460, 61, 421, 1175, 639, 676, 159, 620, 637, 674, 635, 624, 1313, 1179, 261, 226, 224, 1258, 758, 720, 1026, 523, 1067, 756, 265, 754, 719, 1063, 761, 717, 269, 797, 4, 328, 326, 222, 829, 898, 1114, 369, 220, 827, 367, 1321, 825, 851, 814, 2, 1360, 823, 33, 1093, 1232, 862, 864, 1362, 457, 1230, 1097, 860, 1095, 492, 418, 416, 451, 1056, 1423, 1368, 320, 1234, 929, 498, 1099, 927, 925, 1464, 1468, 129, 960, 1110, 164, 577, 76, 538, 575, 160, 573, 70, 412, 1306, 698, 651, 1125, 628, 663, 1300, 1129, 661, 1304, 299, 254, 1266, 1264, 534, 571, 769, 776, 1074, 293, 532, 1406, 291, 1268, 47, 1072, 728, 1035, 1229, 726, 6, 1166, 1400, 374, 339, 337, 801, 1144, 1107, 1356, 857, 1354, 1317, 45, 1142, 805, 1105, 855, 43, 853, 816, 894, 896, 1103, 892, 1391, 1101, 448, 446, 409, 1399, 407, 1434, 1069, 333, 1246, 1395, 331, 989, 901, 1471, 933, 903, 1242, 1203, 489, 1436, 1473, 974, 970, 1475, 583, 133, 548, 581, 546, 84, 1140, 509, 601, 139, 687, 440, 403, 1116, 659, 1331, 642, 607, 657, 176, 1273, 692, 285, 248, 1040, 283, 244, 501, 242, 739, 785, 505, 701, 287, 787, 1279, 58, 1007, 772, 1001, 1151, 50, 1192, 240, 203, 307, 15, 201, 876, 1194, 1345, 1137, 56, 1308, 1133, 1347, 807, 842, 885, 1198, 1388, 1380, 439, 437, 470, 433, 1445, 340, 1255, 1218, 1386, 956, 946, 993, 1212, 944, 995, 97, 1251, 1253, 983, 95, 985, 144, 981, 142, 109, 590, 557, 518, 598, 185, 1148, 630, 183, 648, 632, 189, 634, 646, 1189, 1090, 609, 683, 685, 1283, 1051, 235, 512, 272, 1285, 101, 551, 710, 748, 278, 239, 712, 709, 1287, 25, 744, 707, 1016, 1185, 1012, 783, 396, 318, 355, 27, 1333, 23, 1337, 822, 1370, 1335, 872, 1374, 1085, 870, 469, 467, 351, 919, 1413, 920, 1450, 969, 1415, 999, 952, 954, 117, 567, 1120, 154, 950, 115, 1454, 1419, 152, 528, 563, 113, 111, 561, 62, 424, 461, 1351, 621, 677, 158, 1316, 1353, 569, 660, 199, 1312, 636, 1176, 1310, 262, 520, 260, 757, 1027, 1259, 1021, 718, 753, 716, 751, 1023, 760, 1064, 794, 796, 329, 5, 360, 38, 897, 368, 221, 1115, 828, 366, 1, 1363, 3, 811, 1326, 850, 1361, 1324, 1365, 30, 865, 497, 1231, 321, 1424, 499, 1055, 1422, 930, 1059, 1426, 924, 961, 963, 1469, 77, 1098, 1461, 1111, 163, 126, 539, 576, 1463, 537, 124, 574, 411, 73, 572, 75, 450, 415, 1126, 71, 1467, 169, 413, 697, 1342, 167, 650, 613, 1169, 699, 629, 1128, 662, 1263, 290, 218, 533, 120, 777, 259, 531, 766, 729, 779, 257, 1226, 1032, 764, 1071, 1034, 762, 48, 1401, 1073, 1163, 7, 1167, 1269, 1165, 338, 334, 1318, 1355, 212, 377, 800, 856, 375, 1390, 817, 854, 852, 1143, 1359, 1106, 1398, 44, 1280, 893, 1396, 1282, 1394, 891, 482, 480, 408, 332, 988, 900, 330, 1206, 902, 1247, 1068, 936, 1208, 1433, 1243, 1204, 1437, 932, 1182, 87, 977, 1202, 179, 971, 134, 584, 1180, 1200, 132, 545, 508, 543, 443, 406, 1117, 81, 173, 1476, 171, 688};
	/* the wanted order of code_xxx(), defined in "shuffle.h" */
	// static int ndat[] = { ... };

	static int num_func = sizeof(table)/sizeof(table[0]);
	static int total_func = sizeof(ndat)/sizeof(ndat[0]);
	long int addr[1500] = {0};

	setvbuf(stdout, NULL, _IONBF, 0);

	/* read and parse the file "/proc/self/maps" */
	long int proc_init;
	int fd;
	char content[20000], *cnt = content, *record;
	if ((fd = open("/proc/self/maps", O_RDONLY)) < 0) errquit("open /proc/self/maps failed");
	if (read(fd, content, 20000) < 0) errquit("read /proc/self/maps failed");
	close(fd);
	
	/* get the start point of the process */
	while ((record = strtok_r(cnt, "\n\r", &cnt)) != NULL) {
		char *tmp;
		long int offset = 0;
		
		// 55a76d701000-55a76d70c000 r-xp 0000b000 08:10 70670
		if (strstr(record, " r-xp ") == NULL) continue;
		if (strstr(record, "/chals") == NULL) continue;

		tmp = strtok_r(record, "-", &record);
		sscanf(tmp, "%lx", &proc_init);
		
        strtok_r(record, " ", &record); strtok_r(record, " ", &record);
		
		// 0000b000 08:10 70670
		tmp = strtok_r(record, " ", &record);
		sscanf(tmp, "%lx", &offset);
		proc_init -= offset;
		break;
	}

	/* store the function addresses of code_xxx */
	void* handle = dlopen("libpoem.so", RTLD_LAZY);
    if (!handle) errquit(dlerror());
	for (int i = 0; i < total_func; i++) {
		char name[15];
		memset(name, 0, 15);
		sprintf(name, "code_%d", i);

        void (*tmp)() = dlsym(handle, name); 
		if (tmp == NULL) {
			dlclose(handle);
			errquit("can't get code address");
		}
		memcpy(&addr[i], &tmp, 8);
	}
	dlclose(handle);

    /* open write previlage */
	void *GOT_addr = (void *)(proc_init + 0x17a30);
	uintptr_t ali = (uintptr_t) GOT_addr;	
	ali = ali & 0xfffffffff000;
	void *after_ali = (void *) ali;	
	if (mprotect(after_ali, 0x2000, PROT_READ | PROT_WRITE | PROT_EXEC) < 0) {
		errquit("mprotect failed");
	}
	
    /* alter GOT */
	long int GOT_idx = 0;
	for (int now_idx = 0; now_idx < num_func; GOT_idx += 8, now_idx++) {
		int fake_idx = table[now_idx];
		int real_idx = 0;
		for (; real_idx < total_func; real_idx++) {
			if (ndat[real_idx] == fake_idx) break;
		}
		if (real_idx >= total_func) {
			printf("real idx not found\n"); 
			dlclose(handle);
			return 1;
		}
		if (GOT_idx == 0x17fc0-0x17a30) GOT_idx += 8;
		if (GOT_idx == 0x18420-0x17a30) GOT_idx += 8;
		if (GOT_idx == 0x18560-0x17a30) GOT_idx += 8;
    	memcpy(GOT_addr+GOT_idx, &addr[real_idx], 8);
	}

	/* reset the write previlage */
	if (mprotect(after_ali, 0x2000, PROT_EXEC) < 0) errquit("mprotect failed");

	return 0; 
}