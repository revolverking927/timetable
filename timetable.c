#include<stdio.h>
#include<stdbool.h>

typedef struct {
    int numStudents;
    //char teachersNames[6][32];
    int numTeachers;
    char subjectName[32];
} subject;

typedef struct {
    int numStudents;
    int numTeachers;
    int totalSubjects;
    subject subjectList[];
} yearGroup;

typedef struct {
    int teacherId;
} teacher;

typedef struct {
    int studentId;
} student;

void generalInfo(/*int *dayLength, int *dayStart[2],*/ int *sessionLength, bool *doubleSession) {
    char sessionCheck;
    // printf("How long does the school day last (hours): ");
    // scanf("%d", dayLength);

    // printf("At what time does a school day begin, (Hours): ");
    // scanf("%d/t", dayStart[0]);
    // printf(",(Minutes): ");
    // scanf("%d", dayStart[1]);

    printf("What is the maximum time per session (minutes): ");
    scanf("%d", sessionLength);

    printf("Are double sessions possible, Y|N: ");
    scanf(" %c", &sessionCheck);

    //If sessionCheck is Y or N, it sets double session to true or false
    *doubleSession = (sessionCheck == 'Y') ? true : false;
}

int main(void) {
    //Collecting the inputs
    int numYearGroups;

    int sessionLength;
    int dayEnd[2] = (2,15);
    int dayStart[2] = (8,05); //[Hours, Minutes]
    bool doubleSession;
    // int *_dayLength = &dayLength;
    int *_sessionLength = &sessionLength;
    // int *_dayStart[2] = sizeof(&dayStart * 2);
    bool *_doubleSession = &doubleSession;

    //Asking for general information for the school
    generalInfo(/*_dayLength, _dayStart,*/ _sessionLength, _doubleSession);

    //Prompting for the number of year groups
    printf("How many year groups are in the school: ");
    scanf("%d", &numYearGroups);

    yearGroup yearGroups[numYearGroups]; //defining the amount of year groups

    //Asking for the information of each year group
    for (int i = 0; i < numYearGroups; i++) {
        yearGroup *_curYg = &yearGroups[i]; //creating a pointer to the current yeargroup struct

        printf("\n\tYear group %d\n", i+1);
        printf("Enter the total number of students: ");
        scanf("%d", &_curYg->numStudents);

        printf("Enter the total number of teachers: ");
        scanf("%d", &_curYg->numTeachers);

        printf("\n\tSubjects\n");
        printf("Enter the total number of subjects: ");
        scanf("%d", &_curYg->totalSubjects);

        printf("The total amount of subjects in the yeargroup is: %d\n", _curYg->totalSubjects);
        //asking for the subjects
        int subjectIndex = 0; //index value for each subject
        while (subjectIndex < _curYg->totalSubjects) {
            subject *_curSub = &_curYg->subjectList[subjectIndex]; //creating a pointer to the current subject in the yeargroup
            printf("What is the subject: ");
            scanf("%s", _curSub->subjectName);

            printf("How many teachers teach this subject in the year group: ");
            scanf("%d", &_curSub->numTeachers);

            printf("How many students take this subject in the year group: ");
            scanf("%d", &_curSub->numStudents);
            subjectIndex++;
        }
    }
    

    int dayLength = 
    return 0;
}