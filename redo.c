#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TYPE_SUBJECT 1
#define TYPE_RECESS  2

const char DAYS[][12] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};

typedef struct { 
    int id; 
    char name[16]; 
} Subject;
typedef struct { 
    int id; 
    char name[16]; 
} Recess;
typedef struct { 
    int id; 
    int subjectId; 
    char subjectName[16]; 
    char name[24]; 
} Teacher;
typedef struct {
    int index; 
    int sessionType[16]; 
    int sessionId[16]; 
} Day;
typedef struct {
    int id; 
    int startH; 
    int startM; 
    char startMeridian[3];
    int endH; 
    int endM; 
    char endMeridian[3]; 
} Period;

void clearBuffer(void) { 
    int c;
    while ((c = getchar()) != '\n' && c != EOF); 
}

Subject createSubject(int id, char n[]) {
    Subject s;
    s.id = id; 
    strncpy(s.name, n, 15); 
    s.name[15] = '\0'; 
    return s;
}

Recess createRecess(int id, char n[]) {
    Recess r; 
    r.id = id; 
    strncpy(r.name, n, 15); 
    r.name[15] = '\0'; 
    return r;
}

Teacher createTeacher(int id, int subjectId, char sname[], char n[]) {
    Teacher t; 
    t.id = id; 
    t.subjectId = subjectId;
    strncpy(t.subjectName, sname, 15); 
    t.subjectName[15] = '\0';
    strncpy(t.name, n, 23); 
    t.name[23] = '\0'; 
    return t;
}

void teacherInfo(int id, Teacher list[], char subjectName[], int subjectId) {
    char name[24];
    printf("  Teacher name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';
    list[id] = createTeacher(id, subjectId, subjectName, name);
}

Period *createSchedule(int startHour, int startMinute, char meridian[],
                       int numPeriods, int periodLength) {
    Period *pList = malloc(numPeriods * sizeof(Period));
    if (!pList) { printf("Memory allocation failed.\n"); return NULL; }

    if (strcmp(meridian, "PM") == 0 && startHour != 12) startHour += 12;
    if (strcmp(meridian, "AM") == 0 && startHour == 12) startHour  =  0;

    int base = startHour * 60 + startMinute;
    for (int i = 0; i < numPeriods; i++) {
        int ps = base + i * periodLength, pe = ps + periodLength;
        int sh = (ps/60)%24, sm = ps%60, eh = (pe/60)%24, em = pe%60;
        int dsh = sh, deh = eh;
        char smr[3], emr[3];

        if (sh >= 12) { 
            strcpy(smr,"PM"); if (sh>12) dsh-=12; 
        }
        else { 
            strcpy(smr,"AM"); if (sh==0) dsh=12;  
        }

        if (eh >= 12) { 
            strcpy(emr,"PM"); if (eh>12) deh-=12; 
        }
        else { 
            strcpy(emr,"AM"); if (eh==0) deh=12;  
        }

        Period p;
        p.id=i; 
        p.startH=dsh; 
        p.startM=sm; 
        p.endH=deh; 
        p.endM=em;
        strcpy(p.startMeridian,smr); 
        strcpy(p.endMeridian,emr);
        pList[i] = p;
        printf("  Period %d: %02d:%02d %s - %02d:%02d %s\n", i+1, dsh,sm,smr, deh,em,emr);
    }
    return pList;
}

Day *createWeekSchedule(int numPeriods, Subject *subjectList, int numSubjects,
                        Recess *recessList, int numRecess) {
    Day *week = malloc(5 * sizeof(Day));
    if (!week) { printf("Memory allocation failed.\n"); return NULL; }
    char input[256];

    for (int d = 0; d < 5; d++) {
        week[d].index = d;
        int used = 0;

        printf("\n--- %s ---\n", DAYS[d]);
        printf("Enter sessions (e.g. Math 2, Lunch 1, English 2):\n>> ");
        fflush(stdout);

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        char *token = strtok(input, ",");
        while (token != NULL && used < numPeriods) {
            while (*token == ' ') token++;

            char *lastSpace = strrchr(token, ' ');
            if (!lastSpace) {
                printf("  Invalid format: '%s'\n", token);
                token = strtok(NULL, ","); continue;
            }

            int length = atoi(lastSpace + 1);
            if (length <= 0) {
                printf("  Invalid count for '%s'\n", token);
                token = strtok(NULL, ","); continue;
            }

            int nameLen = (int)(lastSpace - token);
            char name[64];
            strncpy(name, token, nameLen); name[nameLen] = '\0';
            for (int e = nameLen-1; e >= 0 && name[e]==' '; e--) name[e]='\0';

            int found = 0;
            for (int i = 0; i < numSubjects && !found; i++) {
                if (strcasecmp(name, subjectList[i].name) == 0) {
                    if (used + length > numPeriods) {
                        printf("  Not enough periods for '%s'\n", name); break;
                    }
                    for (int k = 0; k < length; k++) {
                        week[d].sessionType[used] = TYPE_SUBJECT;
                        week[d].sessionId[used]   = i;
                        used++;
                    }
                    found = 1;
                }
            }
            for (int i = 0; i < numRecess && !found; i++) {
                if (strcasecmp(name, recessList[i].name) == 0) {
                    if (used + length > numPeriods) {
                        printf("  Not enough periods for '%s'\n", name); break;
                    }
                    for (int k = 0; k < length; k++) {
                        week[d].sessionType[used] = TYPE_RECESS;
                        week[d].sessionId[used]   = i;
                        used++;
                    }
                    found = 1;
                }
            }
            if (!found) printf("  Session '%s' not found.\n", name);
            token = strtok(NULL, ",");
        }

        while (used < numPeriods) {
            week[d].sessionType[used] = 0;
            week[d].sessionId[used]   = -1;
            used++;
        }
    }
    return week;
}

void displayStudentTimeTable(Period *periodList, int numPeriods, Day *week,
                             Subject *subjectList, Recess *recessList) {
    printf("\n+-----------+");
    for (int i = 0; i < numPeriods; i++) {
           printf("---------------------+");
    }

    printf("\n|           |");
    for (int i = 0; i < numPeriods; i++)
        printf(" %02d:%02d %s - %02d:%02d %-2s |",
               periodList[i].startH, periodList[i].startM, periodList[i].startMeridian,
               periodList[i].endH,   periodList[i].endM,   periodList[i].endMeridian);

    printf("\n+-----------+");
    for (int i = 0; i < numPeriods; i++) {
        printf("---------------------+");
    }
    printf("\n");

    for (int d = 0; d < 5; d++) {
        printf("| %-9s |", DAYS[d]);
        for (int p = 0; p < numPeriods; p++) {
            if      (week[d].sessionType[p] == TYPE_SUBJECT)
                printf(" %-19s |", subjectList[week[d].sessionId[p]].name);
            else if (week[d].sessionType[p] == TYPE_RECESS)
                printf(" %-19s |", recessList[week[d].sessionId[p]].name);
            else
                printf(" %-19s |", "---");
        }
        printf("\n");
    }

    printf("+-----------+");
    for (int i = 0; i < numPeriods; i++) {
        printf("---------------------+");
    }
    printf("\n");
}

int main(void) {
    int  startHour, startMinute, numPeriods, periodLength;
    char meridian[3];

    printf("=== Schedule Setup ===\n");
    printf("Start hour   (1-12): "); scanf("%d",  &startHour);      clearBuffer();
    printf("Start minute (0-59): "); scanf("%d",  &startMinute);    clearBuffer();
    printf("AM or PM:            "); scanf("%2s", meridian);        clearBuffer();
    printf("Number of periods:   "); scanf("%d",  &numPeriods);     clearBuffer();
    printf("Period length (min): "); scanf("%d",  &periodLength);   clearBuffer();

    printf("\n=== Periods ===\n");
    Period *periodList = createSchedule(startHour, startMinute, meridian, numPeriods, periodLength);

    int numSubjects, numRecess, numTeachers;
    printf("\nHow many subjects?  "); scanf("%d", &numSubjects);    clearBuffer();
    printf("How many recesses?  "); scanf("%d", &numRecess);        clearBuffer();
    printf("How many teachers?  "); scanf("%d", &numTeachers);      clearBuffer();

    Subject *subjectList  = malloc(numSubjects * sizeof(Subject));
    Recess  *recessList   = malloc(numRecess   * sizeof(Recess));
    Teacher *teachersList = malloc((numTeachers > 0 ? numTeachers : 1) * sizeof(Teacher));

    printf("\n=== Recesses ===\n");
    for (int i = 0; i < numRecess; i++) {
        char buf[16];
        printf("Recess %d name: ", i+1);
        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = '\0';
        recessList[i] = createRecess(i, buf);
    }

    printf("\n=== Subjects & Teachers ===\n");
    int teacherOffset = 0; 
    int teachersLeft  = numTeachers;

    for (int i = 0; i < numSubjects; i++) {
        char buf[16];
        printf("Subject %d name: ", i+1);
        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = '\0';
        subjectList[i] = createSubject(i, buf);

        if (teachersLeft > 0) {
            int perSub = 0;
            printf("  Teachers left: %d. How many teach '%s'? ", teachersLeft, buf);
            scanf("%d", &perSub); 
            clearBuffer();
            if (perSub > teachersLeft) perSub = teachersLeft;
            for (int j = 0; j < perSub; j++)
                teacherInfo(teacherOffset++, teachersList, buf, i);
            teachersLeft -= perSub;
        }
    }

    printf("\n=== Weekly Timetable ===\n");
    Day *week = createWeekSchedule(numPeriods, subjectList, numSubjects, recessList, numRecess);
    displayStudentTimeTable(periodList, numPeriods, week, subjectList, recessList);

    free(periodList); 
    free(subjectList); 
    free(recessList); 
    free(teachersList); 
    free(week);
    return 0;
}