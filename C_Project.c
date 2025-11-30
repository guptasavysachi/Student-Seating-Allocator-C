#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define Name 50
#define Rollno 10
#define FILENAME_BIN "students.dat"
#define FILENAME_LOG "allocationlog.txt"
#define Row 20
#define Columns 20

//Structure :-> student 
struct Student {
    char rollNo[Row];
    char name[Name];
    int row;
    int col;
};

//Structure :-> Seat
struct Seat 
{
    char rollNo[Rollno];
};

// Global variables
struct Student **students;
int studentCount = 0;
struct Seat hall[Row][Columns];
int hallRows, hallCols;
FILE *logFile;

//FUNCTION FOR INHALL !!
void inHall() {
    printf("Enter hall rows (1-%d): ", Row);
    scanf("%d",&hallRows);
    printf("Enter hall columns (1-%d): ",Columns);
    scanf("%d", &hallCols);

    for (int i=0;i<hallRows;i++)
        for (int j=0;j<hallCols;j++)
            strcpy(hall[i][j].rollNo, "");

    students = NULL;
    studentCount = 0;

    logFile = fopen(FILENAME_LOG, "a");

    time_t now = time(NULL);
    char timestamp[50];
    strftime(timestamp,sizeof(timestamp),"%Y-%m-%d %H:%M:%S",localtime(&now));
    fprintf(logFile,"\n--- Session started at %s ---\n", timestamp);

    printf("Hall initialized: %dx%d\n", hallRows, hallCols);
}

// ---------------------------- LOAD STUDENTS -----------------------------
void loadStudents() {
    FILE *fp = fopen(FILENAME_BIN, "rb");
    if (fp==NULL) {
        printf("No previous data.\n");
        return;
    }

    fread(&studentCount, sizeof(int), 1, fp);
    if (studentCount<=0) {
        fclose(fp);
        return;
    }

    students = realloc(students, studentCount * sizeof(struct Student*));
    for (int i=0;i<studentCount;i++) {
        students[i]=malloc(sizeof(struct Student));
        fread(students[i],sizeof(struct Student),1,fp);

        strcpy(hall[students[i]->row][students[i]->col].rollNo,
               students[i]->rollNo);
    }

    fclose(fp);
    printf("Loaded %d students from file.\n", studentCount);
}

// ---------------------------- SAVE STUDENTS -----------------------------
void saveStudents() {
    FILE *fp = fopen(FILENAME_BIN, "wb");
    if (fp==NULL) return;

    fwrite(&studentCount,sizeof(int),1,fp);
    for (int i=0;i<studentCount;i++)
        fwrite(students[i],sizeof(struct Student),1,fp);

    fclose(fp);
    printf("Saved to %s\n", FILENAME_BIN);
}

// ---------------------------- LOG ACTION -----------------------------
void logAction(const char *action, const char *details) {
    time_t now = time(NULL);
    char timestamp[50];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(logFile,"[%s] %s: %s\n",timestamp,action,details);
    fflush(logFile);
}

// ---------------------------- MENU -----------------------------
void displayMenu() {
    printf("\n=== Student Seating Allocator ===\n");
    printf("1. Allocate Seat from student choice \n");
    printf("2. Deallocate Seat\n");
    printf("3. Display Hall\n");
    printf("4. Search Student\n");
    printf("5. View Log\n");
    printf("6. Save & Exit\n");
    printf("7. Allocate Seat from u choice \n");  
    printf("0. Exit (No Save)\n");
}


// ---------------------------- ALLOCATE SEAT -----------------------------
void allocateSeat() {
    if (studentCount>=hallRows * hallCols) {
        printf("Hall full!\n");
        return;
    }

    struct Student *s = malloc(sizeof(struct Student));

    printf("Enter Roll No: ");
    scanf("%s", s->rollNo);

    printf("Enter Name: ");
    fgets(s->name,20,stdin);
    fgets(s->name,20,stdin);

    int found = 0;
    for (int r = 0; r < hallRows && !found; r++)
        for (int c = 0; c < hallCols && !found; c++)
            if (strlen(hall[r][c].rollNo) == 0) {
                s->row = r;
                s->col = c;
                strcpy(hall[r][c].rollNo, s->rollNo);
                found = 1;
            }

    students = realloc(students, (studentCount + 1) * sizeof(struct Student*));
    students[studentCount++] = s;

    char msg[200];
    sprintf(msg, "Seat %d,%d given to %s (%s)",
            s->row + 1, s->col + 1, s->name, s->rollNo);
    logAction("ALLOCATED", msg);

    printf("Seat allocated at Row %d, Col %d\n", s->row + 1, s->col + 1);
}

// ---------------------------- DEALLOCATE SEAT -----------------------------
void deallocateSeat() {
    char roll[Rollno];
    printf("Enter RollNo: ");
    scanf("%s", roll);

    int i;
    for (i = 0; i < studentCount; i++)
        if (strcmp(students[i]->rollNo, roll) == 0)
            break;

    if (i == studentCount) {
        printf("Not found.\n");
        return;
    }

    char msg[200];
    sprintf(msg, "Removed %s (%s)", students[i]->name, students[i]->rollNo);
    logAction("DEALLOCATED", msg);

    strcpy(hall[students[i]->row][students[i]->col].rollNo, "");

    free(students[i]);

    for (int j = i; j < studentCount - 1; j++)
        students[j] = students[j + 1];

    studentCount--;

    if (studentCount > 0)
        students = realloc(students, studentCount * sizeof(struct Student*));
    else {
        free(students);
        students = NULL;
    }

    printf("Student removed.\n");
}

// ---------------------------- DISPLAY HALL -----------------------------
void displayHall() {
    printf("\nHall (%dx%d):\n", hallRows, hallCols);

    for (int r = 0; r < hallRows; r++) {
        for (int c = 0; c < hallCols; c++)
            printf("%c ", strlen(hall[r][c].rollNo) ? 'O' : '-');
        printf("\n");
    }

    printf("Total students: %d\n", studentCount);
}

// ---------------------------- SEARCH STUDENT -----------------------------
void searchStudent() {
    char roll[Rollno];
    printf("Enter Roll No: ");
    scanf("%s", roll);

    for (int i = 0; i < studentCount; i++)
        if (strcmp(students[i]->rollNo, roll) == 0) {
            printf("Found: %s - %s at Row %d, Col %d\n",
                   students[i]->rollNo,
                   students[i]->name,
                   students[i]->row + 1,
                   students[i]->col + 1);
            return;
        }

    printf("Not found.\n");
}

// ---------------------------- VIEW LOG -----------------------------
void viewLog() {
    FILE *fp = fopen(FILENAME_LOG, "r");
    if (!fp) {
        printf("No log.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp))
        printf("%s", line);

    fclose(fp);
}

// ---------------------------- FREE MEMORY -----------------------------
void freeResources() {
    for (int i = 0; i < studentCount; i++)
        free(students[i]);

    free(students);

    time_t now = time(NULL);
    char timestamp[50];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(logFile, "--- Session Ended %s ---\n", timestamp);

    fclose(logFile);
}

// ---------------------------- ALLOCATE SEAT from Teacher choice  -----------------------------
void allocateSeatCustom() {
    struct Student *s = malloc(sizeof(struct Student));

    printf("Enter Roll No: ");
    scanf("%s", s->rollNo);

    printf("Enter Name: ");
    fgets(s->name,20,stdin);
   fgets(s->name,20,stdin);

    int r, c;

    printf("Enter Row Number (1-%d): ", hallRows);
    scanf("%d", &r);
    printf("Enter Column Number (1-%d): ", hallCols);
    scanf("%d", &c);

    // Convert to 0-index
    r--;  
    c--;

    // ---- VALIDATION CHECKS ----
    if (r < 0 || r >= hallRows || c < 0 || c >= hallCols) {
        printf("Invalid seat position!\n");
        free(s);
        return;
    }

    if (strlen(hall[r][c].rollNo) != 0) {
        printf("Seat already occupied by %s\n", hall[r][c].rollNo);
        free(s);
        return;
    }

    // ---- ALLOCATE SEAT ----
    s->row = r;
    s->col = c;
    strcpy(hall[r][c].rollNo, s->rollNo);

    students = realloc(students, (studentCount + 1) * sizeof(struct Student*));
    students[studentCount++] = s;

    char msg[200];
    sprintf(msg, "Manual seat %d,%d to %s (%s)", r + 1, c + 1, s->name, s->rollNo);
    logAction("MANUAL-ALLOCATED", msg);

    printf("Seat allocated manually at Row %d, Column %d\n", r + 1, c + 1);
}
// ---------------------------- MAIN -----------------------------
int main() {
    inHall();
    loadStudents();

    int choice;

    do {
        displayMenu();
        printf("Enter: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: allocateSeat(); break;
            case 2: deallocateSeat(); break;
            case 3: displayHall(); break;
            case 4: searchStudent(); break;
            case 5: viewLog(); break;
            case 6: saveStudents(); break;
            case 7: allocateSeatCustom(); break;   
            case 0: printf("Exited Successfully !!");break;
            default: printf("Invalid.\n");
        }
    } while (choice != 0);
    freeResources();
    return 0;
}