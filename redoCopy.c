#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define TYPE_SUBJECT 1
#define TYPE_RECESS 2

const char DAYS[][12] = {"Monday", "Tuesday", "Wednesday", "Thursday",
                         "Friday"};

typedef struct {
  int id;
  char name[16];
} Subject;

typedef struct {
  int id;
  int indexPeriod;
  char name[16];
} Recess;

typedef struct {
  int id;
  int subjectId;
  int maxSessions;
  int currentSessions;
} Teacher;

typedef struct {
  int sessionType[10];
  int sessionId[10];
  int teacherId[10];
  int roomId[10]; // New: track which classroom is assigned
} Day;

typedef struct {
  char name[16];
  Day week[5];
  int *subjectSessions; // Requested sessions per subject
} YearGroup;

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
  while ((c = getchar()) != '\n' && c != EOF)
    ;
}

Subject createSubject(int id, char n[]) {
  Subject s;
  s.id = id;
  strncpy(s.name, n, 15);
  s.name[15] = '\0';
  return s;
}

Teacher createTeacher(int id, int subjectId, int maxS) {
  Teacher t;
  t.id = id;
  t.subjectId = subjectId;
  t.maxSessions = maxS;
  t.currentSessions = 0;
  return t;
}

Period *createSchedule(int startHour, int startMinute, char meridian[],
                       int numPeriods, int periodLength) {
  Period *pList = malloc(numPeriods * sizeof(Period));
  if (!pList) {
    printf("Memory allocation failed.\n");
    return NULL;
  }

  int militaryStart = startHour;
  if (strcmp(meridian, "PM") == 0 && startHour != 12)
    militaryStart += 12;
  if (strcmp(meridian, "AM") == 0 && startHour == 12)
    militaryStart = 0;

  int base = militaryStart * 60 + startMinute;
  for (int i = 0; i < numPeriods; i++) {
    int ps = base + i * periodLength, pe = ps + periodLength;
    int sh = (ps / 60) % 24, sm = ps % 60, eh = (pe / 60) % 24, em = pe % 60;
    int dsh = sh, deh = eh;
    char smr[3], emr[3];

    if (sh >= 12) {
      strcpy(smr, "PM");
      if (sh > 12)
        dsh -= 12;
    } else {
      strcpy(smr, "AM");
      if (sh == 0)
        dsh = 12;
    }

    if (eh >= 12) {
      strcpy(emr, "PM");
      if (eh > 12)
        deh -= 12;
    } else {
      strcpy(emr, "AM");
      if (eh == 0)
        deh = 12;
    }

    Period p;
    p.id = i;
    p.startH = dsh;
    p.startM = sm;
    p.endH = deh;
    p.endM = em;
    strcpy(p.startMeridian, smr);
    strcpy(p.endMeridian, emr);
    pList[i] = p;
    printf("  Period %d: %02d:%02d %s - %02d:%02d %s\n", i + 1, dsh, sm, smr,
           deh, em, emr);
  }
  return pList;
}

// Automated Timetable Generation Algorithm
void generateTimetable(YearGroup *school, int numYearGroups, int numPeriods,
                       Teacher *teachers, int numTeachers, Recess *recesses,
                       int numRecess, int numRooms) {
  srand((unsigned)time(NULL));

  // Reset day/period trackers and apply global recesses
  for (int y = 0; y < numYearGroups; y++) {
    for (int d = 0; d < 5; d++) {
      for (int p = 0; p < numPeriods; p++) {
        school[y].week[d].sessionType[p] = 0;
        school[y].week[d].sessionId[p] = -1;
        school[y].week[d].teacherId[p] = -1;
        school[y].week[d].roomId[p] = -1;

        // Check for global recess
        for (int r = 0; r < numRecess; r++) {
          if (recesses[r].indexPeriod == p) {
            school[y].week[d].sessionType[p] = TYPE_RECESS;
            school[y].week[d].sessionId[p] = r;
          }
        }
      }
    }
  }

  // Shuffle year group processing order for fairness
  int ygOrder[50];
  for (int i = 0; i < numYearGroups; i++)
    ygOrder[i] = i;
  for (int i = 0; i < numYearGroups; i++) {
    int r = i + rand() % (numYearGroups - i);
    int temp = ygOrder[i];
    ygOrder[i] = ygOrder[r];
    ygOrder[r] = temp;
  }

  // Distribution logic: Randomized scheduling with consecutive blocks
  for (int yi = 0; yi < numYearGroups; yi++) {
    int y = ygOrder[yi];
    // Collect all required sessions
    int required[500];
    int reqCount = 0;
    for (int s = 0; school[y].subjectSessions[s] != -2; s++) {
      for (int i = 0; i < school[y].subjectSessions[s]; i++) {
        required[reqCount++] = s;
      }
    }

    // Shuffle required sessions
    for (int i = 0; i < reqCount; i++) {
      int r = i + rand() % (reqCount - i);
      int temp = required[i];
      required[i] = required[r];
      required[r] = temp;
    }

    int placedSessions[500] = {0};
    int doubleBlockPlaced[500] = {
        0}; // Track how many double blocks placed per subject

    // Pass 1: Place at most 1 double block per subject
    for (int i = 0; i < reqCount; i++) {
      if (placedSessions[i])
        continue;

      int subjId = required[i];

      // Only allow 1 double block per subject
      if (doubleBlockPlaced[subjId])
        continue;

      int remainingForSubject = 0;
      for (int j = i; j < reqCount; j++) {
        if (required[j] == subjId && !placedSessions[j])
          remainingForSubject++;
      }

      if (remainingForSubject >= 2) {
        // Pass 1 now has a probability check (e.g., 70% chance) to attempt a
        // double block. This prevents EVERY subject from gobbling up the only
        // valid consecutive slot (index 0,1) in constrained schedules (like 6
        // periods with 2 breaks).
        if ((rand() % 100) < 50) {
          int placed = 0;
          int maxAttempts = 5 * numPeriods * 2;

          for (int attempt = 0; attempt < maxAttempts && !placed; attempt++) {
            int d = rand() % 5;
            int p = rand() % (numPeriods - 1);

            if (school[y].week[d].sessionId[p] == -1 &&
                school[y].week[d].sessionId[p + 1] == -1) {

              int todayCount = 0;
              for (int checkP = 0; checkP < numPeriods; checkP++) {
                if (school[y].week[d].sessionId[checkP] == subjId)
                  todayCount++;
              }

              // HARD constraint: Never allow more than 2 sessions of same
              // subject per day
              if (todayCount >= 2)
                continue;
              // Soft constraint: Prefer days with 0 sessions of this subject
              if (todayCount > 0 && attempt < maxAttempts - 10)
                continue;

              // Collect all eligible teachers into a list, then pick one
              // randomly
              int eligibleTeachers[50];
              int eligibleCount = 0;
              for (int t = 0; t < numTeachers; t++) {
                if (teachers[t].subjectId == subjId &&
                    teachers[t].maxSessions - teachers[t].currentSessions >=
                        2) {
                  int busy = 0;
                  for (int prevY = 0; prevY < numYearGroups; prevY++) {
                    if (school[prevY].week[d].teacherId[p] == teachers[t].id ||
                        school[prevY].week[d].teacherId[p + 1] ==
                            teachers[t].id) {
                      busy = 1;
                      break;
                    }
                  }
                  if (!busy)
                    eligibleTeachers[eligibleCount++] = teachers[t].id;
                }
              }
              int assignedTeacher =
                  (eligibleCount > 0) ? eligibleTeachers[rand() % eligibleCount]
                                      : -1;

              // Collect all available rooms, then pick one randomly
              int eligibleRooms[50];
              int roomCount = 0;
              if (assignedTeacher != -1) {
                for (int rm = 1; rm <= numRooms; rm++) {
                  int roomBusy = 0;
                  for (int prevY = 0; prevY < numYearGroups; prevY++) {
                    if (school[prevY].week[d].roomId[p] == rm ||
                        school[prevY].week[d].roomId[p + 1] == rm) {
                      roomBusy = 1;
                      break;
                    }
                  }
                  if (!roomBusy)
                    eligibleRooms[roomCount++] = rm;
                }
              }
              int assignedRoom =
                  (roomCount > 0) ? eligibleRooms[rand() % roomCount] : -1;

              if (assignedTeacher != -1 && assignedRoom != -1) {
                school[y].week[d].sessionType[p] = TYPE_SUBJECT;
                school[y].week[d].sessionId[p] = subjId;
                school[y].week[d].teacherId[p] = assignedTeacher;
                school[y].week[d].roomId[p] = assignedRoom;

                school[y].week[d].sessionType[p + 1] = TYPE_SUBJECT;
                school[y].week[d].sessionId[p + 1] = subjId;
                school[y].week[d].teacherId[p + 1] = assignedTeacher;
                school[y].week[d].roomId[p + 1] = assignedRoom;

                for (int t = 0; t < numTeachers; t++) {
                  if (teachers[t].id == assignedTeacher) {
                    teachers[t].currentSessions += 2;
                    break;
                  }
                }

                placedSessions[i] = 1;
                for (int j = i + 1; j < reqCount; j++) {
                  if (required[j] == subjId && !placedSessions[j]) {
                    placedSessions[j] = 1;
                    break;
                  }
                }
                doubleBlockPlaced[subjId] = 1;
                placed = 1;
              }
            }
          }
        } // Close probability check
      }
    }

    // Pass 2: Place remaining single sessions
    for (int i = 0; i < reqCount; i++) {
      if (placedSessions[i])
        continue;

      int subjId = required[i];
      int placed = 0;
      int maxAttempts = 5 * numPeriods * 2;

      for (int attempt = 0; attempt < maxAttempts && !placed; attempt++) {
        int d = rand() % 5;
        int p = rand() % numPeriods;

        if (school[y].week[d].sessionId[p] == -1) {

          int todayCount = 0;
          for (int checkP = 0; checkP < numPeriods; checkP++) {
            if (school[y].week[d].sessionId[checkP] == subjId)
              todayCount++;
          }

          // HARD constraint: Never allow more than 2 sessions of same subject
          // per day
          if (todayCount >= 2)
            continue;
          // Soft constraint: Prefer days with 0 sessions of this subject
          if (todayCount > 0 && attempt < maxAttempts - 5)
            continue;

          // Collect eligible teachers and pick randomly
          int eligibleTeachers[50];
          int eligibleCount = 0;
          for (int t = 0; t < numTeachers; t++) {
            if (teachers[t].subjectId == subjId &&
                teachers[t].currentSessions < teachers[t].maxSessions) {
              int busy = 0;
              for (int prevY = 0; prevY < numYearGroups; prevY++) {
                if (school[prevY].week[d].teacherId[p] == teachers[t].id) {
                  busy = 1;
                  break;
                }
              }
              if (!busy)
                eligibleTeachers[eligibleCount++] = teachers[t].id;
            }
          }
          int assignedTeacher = (eligibleCount > 0)
                                    ? eligibleTeachers[rand() % eligibleCount]
                                    : -1;

          // Collect available rooms and pick randomly
          int eligibleRooms[50];
          int roomCount = 0;
          if (assignedTeacher != -1) {
            for (int rm = 1; rm <= numRooms; rm++) {
              int roomBusy = 0;
              for (int prevY = 0; prevY < numYearGroups; prevY++) {
                if (school[prevY].week[d].roomId[p] == rm) {
                  roomBusy = 1;
                  break;
                }
              }
              if (!roomBusy)
                eligibleRooms[roomCount++] = rm;
            }
          }
          int assignedRoom =
              (roomCount > 0) ? eligibleRooms[rand() % roomCount] : -1;

          if (assignedTeacher != -1 && assignedRoom != -1) {
            school[y].week[d].sessionType[p] = TYPE_SUBJECT;
            school[y].week[d].sessionId[p] = subjId;
            school[y].week[d].teacherId[p] = assignedTeacher;
            school[y].week[d].roomId[p] = assignedRoom;

            for (int t = 0; t < numTeachers; t++) {
              if (teachers[t].id == assignedTeacher) {
                teachers[t].currentSessions++;
                break;
              }
            }
            placedSessions[i] = 1;
            placed = 1;
          }
        }
      }
    }
  }
}

void displayYearGroupTimetable(Period *periodList, int numPeriods,
                               YearGroup *yg, Subject *subjectList,
                               Recess *rList) {
  printf("\n=== TIMETABLE FOR YEAR GROUP: %s ===\n", yg->name);
  printf("+-----------+");
  for (int i = 0; i < numPeriods; i++)
    printf("------------------------+");
  printf("\n| %-9s |", "Time");
  for (int i = 0; i < numPeriods; i++) {
    char timeStr[50];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d%s-%02d:%02d%s",
             periodList[i].startH, periodList[i].startM,
             periodList[i].startMeridian, periodList[i].endH,
             periodList[i].endM, periodList[i].endMeridian);
    printf(" %-22s |", timeStr);
  }

  printf("\n+-----------+");
  for (int i = 0; i < numPeriods; i++)
    printf("------------------------+");
  printf("\n");

  for (int d = 0; d < 5; d++) {
    printf("| %-9s |", DAYS[d]);
    for (int p = 0; p < numPeriods; p++) {
      if (yg->week[d].sessionType[p] == TYPE_RECESS) {
        printf(" %-22.22s |", rList[yg->week[d].sessionId[p]].name);
      } else if (yg->week[d].sessionId[p] != -1) {
        char cell[50];
        int tid = yg->week[d].teacherId[p];
        int rid = yg->week[d].roomId[p];
        if (tid != -1 && rid != -1)
          snprintf(cell, sizeof(cell), "%s (T%d, R%d)",
                   subjectList[yg->week[d].sessionId[p]].name, tid, rid);
        else if (tid !=
                 -1) // Should not occur with new room logic, fallback safety
          snprintf(cell, sizeof(cell), "%s (T%d)",
                   subjectList[yg->week[d].sessionId[p]].name, tid);
        else
          snprintf(cell, sizeof(cell), "%s (No T)",
                   subjectList[yg->week[d].sessionId[p]].name);
        printf(" %-22.22s |", cell);
      } else {
        printf(" %-22s |", "Free session");
      }
    }
    printf("\n");
    printf("+-----------+");
    for (int i = 0; i < numPeriods; i++)
      printf("------------------------+");
    printf("\n");
  }
}

void displaySubjectTimetables(Period *periodList, int numPeriods,
                              YearGroup *school, int numYearGroups,
                              Subject *subjectList, int numSubjects,
                              Recess *recessList, int numRecesses) {
  for (int s = 0; s < numSubjects; s++) {
    printf("\n=== TIMETABLE FOR TEACHERS OF SUBJECT: %s ===\n",
           subjectList[s].name);
    printf("+-----------+");
    for (int i = 0; i < numPeriods; i++)
      printf("------------------------+");
    printf("\n| %-9s |", "Time");
    for (int i = 0; i < numPeriods; i++) {
      char timeStr[50];
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d%s-%02d:%02d%s",
               periodList[i].startH, periodList[i].startM,
               periodList[i].startMeridian, periodList[i].endH,
               periodList[i].endM, periodList[i].endMeridian);
      printf(" %-22s |", timeStr);
    }

    printf("\n+-----------+");
    for (int i = 0; i < numPeriods; i++)
      printf("------------------------+");
    printf("\n");

    for (int d = 0; d < 5; d++) {
      int maxLines = 1;
      for (int p = 0; p < numPeriods; p++) {
        int foundCount = 0;
        for (int y = 0; y < numYearGroups; y++) {
          if (school[y].week[d].sessionType[p] == TYPE_SUBJECT &&
              school[y].week[d].sessionId[p] == s) {
            foundCount++;
          }
        }
        if (foundCount > maxLines)
          maxLines = foundCount;
      }

      for (int line = 0; line < maxLines; line++) {
        if (line == 0)
          printf("| %-9s |", DAYS[d]);
        else
          printf("| %-9s |", "");

        for (int p = 0; p < numPeriods; p++) {
          int isRecess = 0;
          int rId = -1;
          for (int r = 0; r < numRecesses; r++) {
            if (recessList[r].indexPeriod == p) {
              isRecess = 1;
              rId = r;
              break;
            }
          }

          if (isRecess) {
            if (line == 0)
              printf(" %-22.22s |", recessList[rId].name);
            else
              printf(" %-22s |", "");
          } else {
            int currentMatch = 0;
            int printed = 0;
            for (int y = 0; y < numYearGroups; y++) {
              if (school[y].week[d].sessionType[p] == TYPE_SUBJECT &&
                  school[y].week[d].sessionId[p] == s) {
                if (currentMatch == line) {
                  char temp[50];
                  snprintf(temp, sizeof(temp), "T%d (R%d, %s)",
                           school[y].week[d].teacherId[p],
                           school[y].week[d].roomId[p], school[y].name);
                  printf(" %-22.22s |", temp);
                  printed = 1;
                  break;
                }
                currentMatch++;
              }
            }
            if (!printed) {
              if (line == 0 && currentMatch == 0)
                printf(" %-22s |", "---");
              else
                printf(" %-22s |", "");
            }
          }
        }
        printf("\n");
      }
      printf("+-----------+");
      for (int i = 0; i < numPeriods; i++)
        printf("------------------------+");
      printf("\n");
    }
  }
}

int main(void) {
  int startHour, startMinute, endHour, endMinute, numSessions;
  char startMeridian[3], endMeridian[3];

  printf("=== School Timetable Generator ===\n");
  printf("--- Workday Schedule ---\n");
  printf("Start time (Hour 1-12): ");
  scanf("%d", &startHour);
  clearBuffer();
  printf("Start time (Minute 0-59): ");
  scanf("%d", &startMinute);
  clearBuffer();
  printf("Start AM or PM: ");
  scanf("%s", startMeridian);
  clearBuffer();

  printf("\nEnd time (Hour 1-12): ");
  scanf("%d", &endHour);
  clearBuffer();
  printf("End time (Minute 0-59): ");
  scanf("%d", &endMinute);
  clearBuffer();
  printf("End AM or PM: ");
  scanf("%s", endMeridian);
  clearBuffer();

  printf("\nNumber of sessions per day: ");
  scanf("%d", &numSessions);
  clearBuffer();

  // Convert to military minutes for calculation
  int mStart = startHour;
  if (strcmp(startMeridian, "PM") == 0 && startHour != 12)
    mStart += 12;
  if (strcmp(startMeridian, "AM") == 0 && startHour == 12)
    mStart = 0;
  int startTotal = mStart * 60 + startMinute;

  int mEnd = endHour;
  if (strcmp(endMeridian, "PM") == 0 && endHour != 12)
    mEnd += 12;
  if (strcmp(endMeridian, "AM") == 0 && endHour == 12)
    mEnd = 0;
  int endTotal = mEnd * 60 + endMinute;

  if (endTotal <= startTotal) {
    printf("Error: End time must be after start time.\n");
    return 1;
  }

  int totalMinutes = endTotal - startTotal;
  int sessionLength = totalMinutes / numSessions;

  printf("\nGenerated %d sessions of %d minutes each.\n", numSessions,
         sessionLength);
  Period *periodList = createSchedule(startHour, startMinute, startMeridian,
                                      numSessions, sessionLength);

  int numSubjects;
  printf("\n--- Course Information ---\n");
  printf("How many different subjects are there? ");
  scanf("%d", &numSubjects);
  clearBuffer();
  Subject *subjectList = malloc(numSubjects * sizeof(Subject));
  int *teachersPerSubject = malloc(numSubjects * sizeof(int));

  int totalTeacherCount = 0;
  for (int i = 0; i < numSubjects; i++) {
    char name[16];
    printf("\nSubject ID %d Name: ", i);
    fgets(name, sizeof(name), stdin);
    char *newline = strchr(name, '\n');
    if (newline)
      *newline = '\0';
    subjectList[i] = createSubject(i, name);
    printf("  Number of teachers assigned to '%s': ", name);
    scanf("%d", &teachersPerSubject[i]);
    clearBuffer();
    totalTeacherCount += teachersPerSubject[i];
  }

  Teacher *teachersList = malloc(totalTeacherCount * sizeof(Teacher));
  int tIdx = 0;
  for (int i = 0; i < numSubjects; i++) {
    for (int j = 0; j < teachersPerSubject[i]; j++) {
      teachersList[tIdx] = createTeacher(tIdx, i, 25);
      tIdx++;
    }
  }

  int numRecesses;
  printf("\n--- Recess / Break configuration ---\n");
  printf("How many daily breaks (e.g., 1 for Lunch, 2 for Recess & Lunch)? ");
  scanf("%d", &numRecesses);
  clearBuffer();
  Recess *recessList = malloc(numRecesses * sizeof(Recess));
  for (int i = 0; i < numRecesses; i++) {
    char name[16];
    int idx;
    printf("\nBreak %d Name (e.g., Lunch): ", i + 1);
    fgets(name, sizeof(name), stdin);
    char *newline = strchr(name, '\n');
    if (newline)
      *newline = '\0';
    printf("Which session index (1 to %d) is %s? ", numSessions, name);
    scanf("%d", &idx);
    clearBuffer();

    recessList[i].id = i;
    recessList[i].indexPeriod = idx - 1; // 0-indexed internally
    strncpy(recessList[i].name, name, 15);
    recessList[i].name[15] = '\0';
  }

  int numYearGroups;
  int numRooms;
  printf("\n--- Facilities Configuration ---\n");
  printf("How many classrooms are available in total? ");
  scanf("%d", &numRooms);
  clearBuffer();

  printf("\n--- Student Configuration ---\n");
  printf("How many Year Groups (e.g., Year 7, Year 8)? ");
  scanf("%d", &numYearGroups);
  clearBuffer();
  YearGroup *school = malloc(numYearGroups * sizeof(YearGroup));

  for (int i = 0; i < numYearGroups; i++) {
    printf("\nName of Year Group %d: ", i + 1);
    fgets(school[i].name, 15, stdin);
    char *newline = strchr(school[i].name, '\n');
    if (newline)
      *newline = '\0';

    school[i].subjectSessions = malloc((numSubjects + 1) * sizeof(int));
    printf("  Allocation for %s:\n", school[i].name);
    for (int s = 0; s < numSubjects; s++) {
      printf("    Weekly sessions for %s (ID %d): ", subjectList[s].name, s);
      scanf("%d", &school[i].subjectSessions[s]);
      clearBuffer();
    }
    school[i].subjectSessions[numSubjects] = -2; // Sentinel
  }

  printf("\n--- Automating Realistic Timetable Generation ---\n");
  generateTimetable(school, numYearGroups, numSessions, teachersList,
                    totalTeacherCount, recessList, numRecesses, numRooms);

  for (int i = 0; i < numYearGroups; i++) {
    displayYearGroupTimetable(periodList, numSessions, &school[i], subjectList,
                              recessList);
  }

  printf("\n--- Automating Teacher Timetables ---\n");
  displaySubjectTimetables(periodList, numSessions, school, numYearGroups,
                           subjectList, numSubjects, recessList, numRecesses);

  // Free memory
  for (int i = 0; i < numYearGroups; i++)
    free(school[i].subjectSessions);
  free(school);
  free(teachersList);
  free(subjectList);
  free(teachersPerSubject);
  free(periodList);
  free(recessList);

  printf("\nProcess complete. All timetables generated successfully.\n");
  return 0;
}
