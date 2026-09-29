# Timetable Generator Pseudocode

## 1. Data Structures

**Subject**: Contains ID and Subject Name.
**Recess**: Contains ID, Session Index (the period it occupies), and Name.
**Teacher**: Contains ID, Subject ID they teach, Maximum Sessions they can teach per week, and Current Sessions assigned.
**Day**: Represents a single day for a year group. 
  - Contains arrays (length = periods per day) for:
    - Session Type (Subject or Recess or Free)
    - Session ID (which Subject or Recess)
    - Teacher ID
    - Room ID

**YearGroup**: Contains Year Group Name, an array of 5 `Day` structures for the week, and an array detailing how many sessions of each subject are required per week.
**Period**: Contains the literal clock time information (Start Hour, Start Minute, End Hour, End Minute, and AM/PM strings) for a specific session index.

## 2. Helper Functions

**readInt(prompt, min, max)**:
  - Loop infinitely:
    - Prompt the user and read input string.
    - If input is "exit", terminate program.
    - Attempt to parse as an integer.
    - If valid and between min and max, return the integer.
    - Else, print error message and loop again.

**readChoice(prompt, options list)**:
  - Loop infinitely:
    - Prompt user and read input string.
    - If input is "exit", terminate program.
    - If input matches any option in the list (case-insensitive), return the matched string.
    - Else, print error message and loop again.

**createSchedule(startTime, periods, length)**:
  - Calculate total minutes from midnight for the start time.
  - Create a list of `Period` structs.
  - For each period from 1 to `periods`:
    - Calculate start time = base + (i * length).
    - Calculate end time = start time + length.
    - Convert times and calculate AM/PM limits.
    - Save to `Period` list.
  - Return the `Period` list.

## 3. Algorithm: `generateTimetable`

**Inputs**: School (List of YearGroups), Number of YearGroups, Number of Sessions, Teachers list, Recesses list, Number of Rooms.

**Initialization**:
  - For each Year Group, for each Day, for each Session:
    - Set Session to empty (-1).
    - If the current session index matches a Recess index, assign it as a RECESS session type.

**Randomization**:
  - Create a shuffled list of the Year Groups to process them in a random order (improves fairness).

**Distribution Pass 1 (Double Blocks)**:
  - For each Year Group (in shuffled order):
    - Collect a flattened list of all `required sessions` from `subjectSessions`.
    - Shuffle the `required sessions` list.
    - For each required session:
      - If already placed or its subject already has a double block placed, skip.
      - Check if the subject has at least 2 remaining required sessions.
      - If yes, with a 50% probability, try to place a double block:
        - Try multiple random attempts (random Day, random Period p):
          - If Period `p` and `p+1` are both empty:
            - Ensure the subject isn't already scheduled > 2 times on this day.
            - Find all eligible teachers (teach this subject, have >= 2 free session slots, and aren't busy with another year group at `p` or `p+1`).
            - Pick a random eligible teacher.
            - Find all eligible rooms (not used by any other year group at `p` or `p+1`).
            - Pick a random eligible room.
            - If both a teacher and a room are found:
              - Assign `p` and `p+1` to this Subject, Teacher, and Room.
              - Increment teacher's current session counter by 2.
              - Mark 2 sessions of this subject as placed.
              - Break out of attempt loop.

**Distribution Pass 2 (Single Blocks)**:
  - For each Year Group (in shuffled order):
    - For each required session that is not yet placed:
      - Try multiple random attempts (random Day, random Period p):
        - If Period `p` is empty:
          - Ensure the subject isn't already scheduled >= 2 times on this day.
          - Find all eligible teachers (teach this subject, not at max sessions, not busy at Period `p`).
          - Pick a random eligible teacher.
          - Find all eligible rooms (not used at Period `p`).
          - Pick a random eligible room.
          - If both a teacher and a room are found:
            - Assign `p` to this Subject, Teacher, and Room.
            - Increment teacher's current session counter by 1.
            - Mark session as placed.
            - Break out of attempt loop.

## 4. Display Functions

**displayYearGroupTimetable**:
  - Calculate maximum column width dynamically based on string lengths of time headers, recesses, and "Subject (Teacher, Room)" strings formatting.
  - Print the Time header row with the specific `Period` start/end times.
  - For each Day (Monday to Friday):
    - For each Period:
      - If Recess, print the Recess name.
      - If Subject, format and print "Subject Name (Teacher ID, Room ID)".
      - Else, print "Free session".

**displaySubjectTimetables**:
  - Calculate maximum column width dynamically based on teacher cell data.
  - For each Subject:
    - Print the Time header row.
    - For each Day:
      - Determine the maximum number of simultaneous classes happening across all year groups for this specific subject at any given time (this becomes `maxLines` rows).
      - For `line = 0` to `maxLines`:
        - For each Period:
          - If Recess, print Recess name on the first line.
          - Else, find all year groups having this subject right now. Print the `n-th` match on the `n-th` line in format "Teacher ID (Room ID, YearGroup Name)".
          - If no matches and the period isn't recess, print "---".

## 5. Main Control Flow

1. Loop until a valid Workday start and end time is entered by the user.
2. Read the number of sessions, calculate session length, and call `createSchedule`.
3. Read the number of subjects.
4. For each subject, read its name and the number of teachers teaching it.
5. Setup the `teachersList` based on the subjects and required teachers.
6. Read the number of recesses, and for each recess, read its name and which session index it replaces.
7. Read the number of total available classrooms/rooms in the school.
8. Read the number of Year Groups.
9. For each Year Group:
  - Read its name.
  - For each subject, read how many weekly sessions this year group needs for that subject.
10. Call `generateTimetable` to automatically allocate all subjects, teachers, and rooms across the week.
11. Loop through all Year Groups and call `displayYearGroupTimetable` for each.
12. Call `displaySubjectTimetables` to show the schedules from the teachers' perspective.
13. Free allocated memory and Exit.
