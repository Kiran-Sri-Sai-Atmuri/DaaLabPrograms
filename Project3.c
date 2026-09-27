#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STUDENTS 1000
#define DATA_FILE "students_fee_records.csv"

typedef struct {
    char id[20];
    char name[60];
    char branch[30];
    char year[15];
    char scholarship[25];

    double tuitionFee;
    double hostelFee;
    double messFee;
    double examFee;
    double otherFee;
    double paidFee;
} Student;

/* ---------- Function Declarations ---------- */

void clearScreen();
void pauseScreen();
void printMainHeader(int count);
void printLine(char ch, int length);
void trimNewline(char str[]);
void sanitizeInput(char str[]);
void inputString(const char *label, char value[], int size);
int inputInt(const char *label, int min, int max);
double inputDouble(const char *label, double min);

double getTotalFee(Student s);
double getPendingFee(Student s);

void loadRecords(Student students[], int *count);
void saveRecords(Student students[], int count);

int findStudentById(Student students[], int count, const char id[]);
void addStudent(Student students[], int *count);
void searchStudent(Student students[], int count);
void displayStudentDetails(Student s);
void displayAllStudents(Student students[], int count);
void updateStudent(Student students[], int count);
void deleteStudent(Student students[], int *count);
void displayPendingStudents(Student students[], int count);
void displayScholarshipStudents(Student students[], int count);
void displayNonScholarshipStudents(Student students[], int count);
void sortByPendingFee(Student students[], int count);
void sortByName(Student students[], int count);

void printTableHeader();
void printStudentRow(Student s, int serialNo);

int comparePendingDesc(const void *a, const void *b);
int compareNameAsc(const void *a, const void *b);

/* ---------- Main Function ---------- */

int main() {
    Student students[MAX_STUDENTS];
    int count = 0;
    int choice;

    loadRecords(students, &count);

    while (1) {
        clearScreen();
        printMainHeader(count);

        printf("\n");
        printf("  1. Add New Student Fee Record\n");
        printf("  2. Search Student by ID\n");
        printf("  3. Display All Student Records\n");
        printf("  4. Update Fee / Student Details\n");
        printf("  5. Delete Student Record\n");
        printf("  6. Display Pending Fee Students\n");
        printf("  7. Display Scholarship Students\n");
        printf("  8. Display Non-Scholarship Students\n");
        printf("  9. Sort Records by Pending Fee\n");
        printf("10. Sort Records by Student Name\n");
        printf("11. Save Records Manually\n");
        printf("12. Exit\n");

        printf("\n");
        printLine('-', 72);

        choice = inputInt("Enter your choice", 1, 12);

        clearScreen();

        switch (choice) {
            case 1:
                addStudent(students, &count);
                saveRecords(students, count);
                break;

            case 2:
                searchStudent(students, count);
                break;

            case 3:
                displayAllStudents(students, count);
                break;

            case 4:
                updateStudent(students, count);
                saveRecords(students, count);
                break;

            case 5:
                deleteStudent(students, &count);
                saveRecords(students, count);
                break;

            case 6:
                displayPendingStudents(students, count);
                break;

            case 7:
                displayScholarshipStudents(students, count);
                break;

            case 8:
                displayNonScholarshipStudents(students, count);
                break;

            case 9:
                sortByPendingFee(students, count);
                break;

            case 10:
                sortByName(students, count);
                break;

            case 11:
                saveRecords(students, count);
                printf("\nRecords saved successfully into '%s'.\n", DATA_FILE);
                break;

            case 12:
                saveRecords(students, count);
                printf("\nThank you. Records saved successfully.\n");
                printf("Exiting Student Fee Management System...\n\n");
                return 0;
        }

        pauseScreen();
    }

    return 0;
}

/* ---------- Utility Functions ---------- */

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseScreen() {
    char temp[10];
    printf("\nPress ENTER to continue...");
    fgets(temp, sizeof(temp), stdin);
}

void printLine(char ch, int length) {
    int i;
    for (i = 0; i < length; i++) {
        putchar(ch);
    }
    putchar('\n');
}

void printMainHeader(int count) {
    printLine('=', 72);
    printf("                 STUDENT FEE MANAGEMENT SYSTEM\n");
    printf("                       RGUKT ONGOLE CAMPUS\n");
    printLine('=', 72);
    printf(" Total Records: %d\n", count);
    printLine('-', 72);
}

void trimNewline(char str[]) {
    str[strcspn(str, "\n")] = '\0';
}

void sanitizeInput(char str[]) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] == ',') {
            str[i] = ' ';
        }
    }
}

void inputString(const char *label, char value[], int size) {
    while (1) {
        printf("%s: ", label);

        if (fgets(value, size, stdin) == NULL) {
            printf("Invalid input. Try again.\n");
            continue;
        }

        trimNewline(value);
        sanitizeInput(value);

        if (strlen(value) == 0) {
            printf("This field cannot be empty. Try again.\n");
        } else {
            break;
        }
    }
}

int inputInt(const char *label, int min, int max) {
    char buffer[100];
    int value;
    char extra;

    while (1) {
        printf("%s (%d-%d): ", label, min, max);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Invalid input. Try again.\n");
            continue;
        }

        if (sscanf(buffer, "%d %c", &value, &extra) != 1) {
            printf("Enter a valid number.\n");
            continue;
        }

        if (value < min || value > max) {
            printf("Enter a number between %d and %d.\n", min, max);
            continue;
        }

        return value;
    }
}

double inputDouble(const char *label, double min) {
    char buffer[100];
    double value;
    char extra;

    while (1) {
        printf("%s: ", label);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Invalid input. Try again.\n");
            continue;
        }

        if (sscanf(buffer, "%lf %c", &value, &extra) != 1) {
            printf("Enter a valid amount.\n");
            continue;
        }

        if (value < min) {
            printf("Amount cannot be less than %.2lf.\n", min);
            continue;
        }

        return value;
    }
}

double getTotalFee(Student s) {
    return s.tuitionFee + s.hostelFee + s.messFee + s.examFee + s.otherFee;
}

double getPendingFee(Student s) {
    return getTotalFee(s) - s.paidFee;
}

/* ---------- File Handling ---------- */

void loadRecords(Student students[], int *count) {
    FILE *fp;
    char line[500];
    char *token;
    int field;

    fp = fopen(DATA_FILE, "r");

    if (fp == NULL) {
        *count = 0;
        return;
    }

    *count = 0;

    /* Skip heading line */
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp) != NULL) {
        Student s;
        field = 0;

        memset(&s, 0, sizeof(Student));

        token = strtok(line, ",\n");

        while (token != NULL) {
            switch (field) {
                case 0:
                    strncpy(s.id, token, sizeof(s.id) - 1);
                    break;
                case 1:
                    strncpy(s.name, token, sizeof(s.name) - 1);
                    break;
                case 2:
                    strncpy(s.branch, token, sizeof(s.branch) - 1);
                    break;
                case 3:
                    strncpy(s.year, token, sizeof(s.year) - 1);
                    break;
                case 4:
                    strncpy(s.scholarship, token, sizeof(s.scholarship) - 1);
                    break;
                case 5:
                    s.tuitionFee = atof(token);
                    break;
                case 6:
                    s.hostelFee = atof(token);
                    break;
                case 7:
                    s.messFee = atof(token);
                    break;
                case 8:
                    s.examFee = atof(token);
                    break;
                case 9:
                    s.otherFee = atof(token);
                    break;
                case 10:
                    s.paidFee = atof(token);
                    break;
            }

            field++;
            token = strtok(NULL, ",\n");
        }

        if (field >= 11 && *count < MAX_STUDENTS) {
            students[*count] = s;
            (*count)++;
        }
    }

    fclose(fp);
}

void saveRecords(Student students[], int count) {
    FILE *fp;
    int i;

    fp = fopen(DATA_FILE, "w");

    if (fp == NULL) {
        printf("\nError: Unable to save records.\n");
        return;
    }

    fprintf(fp, "ID,Name,Branch,Year,Scholarship,TuitionFee,HostelFee,MessFee,ExamFee,OtherFee,PaidFee\n");

    for (i = 0; i < count; i++) {
        fprintf(
            fp,
            "%s,%s,%s,%s,%s,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf\n",
            students[i].id,
            students[i].name,
            students[i].branch,
            students[i].year,
            students[i].scholarship,
            students[i].tuitionFee,
            students[i].hostelFee,
            students[i].messFee,
            students[i].examFee,
            students[i].otherFee,
            students[i].paidFee
        );
    }

    fclose(fp);
}

/* ---------- Core Operations ---------- */

int findStudentById(Student students[], int count, const char id[]) {
    int i;

    for (i = 0; i < count; i++) {
        if (strcmp(students[i].id, id) == 0) {
            return i;
        }
    }

    return -1;
}

void addStudent(Student students[], int *count) {
    Student s;
    char id[20];
    int index;
    int scholarshipChoice;

    printMainHeader(*count);
    printf("\nADD NEW STUDENT FEE RECORD\n");
    printLine('-', 72);

    if (*count >= MAX_STUDENTS) {
        printf("\nRecord limit reached. Cannot add more students.\n");
        return;
    }

    inputString("Enter Student ID", id, sizeof(id));

    index = findStudentById(students, *count, id);

    if (index != -1) {
        printf("\nA student with this ID already exists.\n");
        printf("Use update option to modify the existing record.\n");
        return;
    }

    memset(&s, 0, sizeof(Student));

    strcpy(s.id, id);

    inputString("Enter Student Name", s.name, sizeof(s.name));
    inputString("Enter Branch / Section", s.branch, sizeof(s.branch));
    inputString("Enter Year / Class", s.year, sizeof(s.year));

    printf("\nScholarship Status\n");
    printf("1. Scholarship\n");
    printf("2. Non-Scholarship\n");
    scholarshipChoice = inputInt("Choose status", 1, 2);

    if (scholarshipChoice == 1) {
        strcpy(s.scholarship, "Scholarship");
    } else {
        strcpy(s.scholarship, "Non-Scholarship");
    }

    printf("\nEnter Fee Details\n");
    printLine('-', 72);

    s.tuitionFee = inputDouble("Tuition Fee", 0);
    s.hostelFee = inputDouble("Hostel Fee", 0);
    s.messFee = inputDouble("Mess Fee", 0);
    s.examFee = inputDouble("Exam Fee", 0);
    s.otherFee = inputDouble("Other Fee", 0);
    s.paidFee = inputDouble("Amount Already Paid", 0);

    students[*count] = s;
    (*count)++;

    printf("\nStudent fee record added successfully.\n");
    printf("Record saved automatically.\n");
}

void searchStudent(Student students[], int count) {
    char id[20];
    int index;

    printMainHeader(count);
    printf("\nSEARCH STUDENT BY ID\n");
    printLine('-', 72);

    if (count == 0) {
        printf("\nNo records available.\n");
        return;
    }

    inputString("Enter Student ID", id, sizeof(id));

    index = findStudentById(students, count, id);

    if (index == -1) {
        printf("\nNo student found with ID: %s\n", id);
    } else {
        displayStudentDetails(students[index]);
    }
}

void displayStudentDetails(Student s) {
    double total = getTotalFee(s);
    double pending = getPendingFee(s);

    printf("\n");
    printLine('=', 72);
    printf("                         STUDENT FEE DETAILS\n");
    printLine('=', 72);

    printf(" Student ID          : %s\n", s.id);
    printf(" Student Name        : %s\n", s.name);
    printf(" Branch / Section    : %s\n", s.branch);
    printf(" Year / Class        : %s\n", s.year);
    printf(" Scholarship Status  : %s\n", s.scholarship);

    printLine('-', 72);
    printf(" Tuition Fee         : Rs. %10.2lf\n", s.tuitionFee);
    printf(" Hostel Fee          : Rs. %10.2lf\n", s.hostelFee);
    printf(" Mess Fee            : Rs. %10.2lf\n", s.messFee);
    printf(" Exam Fee            : Rs. %10.2lf\n", s.examFee);
    printf(" Other Fee           : Rs. %10.2lf\n", s.otherFee);

    printLine('-', 72);
    printf(" Total Fee           : Rs. %10.2lf\n", total);
    printf(" Paid Fee            : Rs. %10.2lf\n", s.paidFee);

    if (pending > 0) {
        printf(" Pending Fee         : Rs. %10.2lf\n", pending);
        printf(" Fee Status          : PENDING\n");
    } else if (pending == 0) {
        printf(" Pending Fee         : Rs. %10.2lf\n", 0.00);
        printf(" Fee Status          : FULLY PAID\n");
    } else {
        printf(" Pending Fee         : Rs. %10.2lf\n", 0.00);
        printf(" Extra Paid Amount   : Rs. %10.2lf\n", -pending);
        printf(" Fee Status          : EXTRA PAID / ADVANCE\n");
    }

    printLine('=', 72);
}

void displayAllStudents(Student students[], int count) {
    int i;

    printMainHeader(count);
    printf("\nALL STUDENT FEE RECORDS\n");
    printLine('-', 118);

    if (count == 0) {
        printf("\nNo records available.\n");
        return;
    }

    printTableHeader();

    for (i = 0; i < count; i++) {
        printStudentRow(students[i], i + 1);
    }

    printLine('-', 118);
}

void updateStudent(Student students[], int count) {
    char id[20];
    int index;
    int choice;
    int scholarshipChoice;
    double amount;

    printMainHeader(count);
    printf("\nUPDATE STUDENT FEE / DETAILS\n");
    printLine('-', 72);

    if (count == 0) {
        printf("\nNo records available.\n");
        return;
    }

    inputString("Enter Student ID", id, sizeof(id));

    index = findStudentById(students, count, id);

    if (index == -1) {
        printf("\nNo student found with ID: %s\n", id);
        return;
    }

    displayStudentDetails(students[index]);

    printf("\nWhat do you want to update?\n");
    printf("1. Add Fee Payment\n");
    printf("2. Set Paid Fee Amount\n");
    printf("3. Edit Fee Components\n");
    printf("4. Edit Student Personal Details\n");
    printf("5. Edit Scholarship Status\n");
    printf("6. Cancel Update\n");

    choice = inputInt("Enter choice", 1, 6);

    switch (choice) {
        case 1:
            amount = inputDouble("Enter additional payment amount", 0);
            students[index].paidFee += amount;
            printf("\nPayment added successfully.\n");
            break;

        case 2:
            students[index].paidFee = inputDouble("Enter new paid fee amount", 0);
            printf("\nPaid fee updated successfully.\n");
            break;

        case 3:
            printf("\nEnter Updated Fee Components\n");
            printLine('-', 72);
            students[index].tuitionFee = inputDouble("Tuition Fee", 0);
            students[index].hostelFee = inputDouble("Hostel Fee", 0);
            students[index].messFee = inputDouble("Mess Fee", 0);
            students[index].examFee = inputDouble("Exam Fee", 0);
            students[index].otherFee = inputDouble("Other Fee", 0);
            printf("\nFee components updated successfully.\n");
            break;

        case 4:
            inputString("Enter Updated Student Name", students[index].name, sizeof(students[index].name));
            inputString("Enter Updated Branch / Section", students[index].branch, sizeof(students[index].branch));
            inputString("Enter Updated Year / Class", students[index].year, sizeof(students[index].year));
            printf("\nStudent details updated successfully.\n");
            break;

        case 5:
            printf("\nScholarship Status\n");
            printf("1. Scholarship\n");
            printf("2. Non-Scholarship\n");
            scholarshipChoice = inputInt("Choose status", 1, 2);

            if (scholarshipChoice == 1) {
                strcpy(students[index].scholarship, "Scholarship");
            } else {
                strcpy(students[index].scholarship, "Non-Scholarship");
            }

            printf("\nScholarship status updated successfully.\n");
            break;

        case 6:
            printf("\nUpdate cancelled.\n");
            return;
    }

    printf("Record saved automatically.\n");
}

void deleteStudent(Student students[], int *count) {
    char id[20];
    int index;
    int confirm;
    int i;

    printMainHeader(*count);
    printf("\nDELETE STUDENT RECORD\n");
    printLine('-', 72);

    if (*count == 0) {
        printf("\nNo records available.\n");
        return;
    }

    inputString("Enter Student ID to delete", id, sizeof(id));

    index = findStudentById(students, *count, id);

    if (index == -1) {
        printf("\nNo student found with ID: %s\n", id);
        return;
    }

    displayStudentDetails(students[index]);

    printf("\nAre you sure you want to delete this record?\n");
    printf("1. Yes, delete\n");
    printf("2. No, cancel\n");

    confirm = inputInt("Enter choice", 1, 2);

    if (confirm == 2) {
        printf("\nDelete cancelled.\n");
        return;
    }

    for (i = index; i < *count - 1; i++) {
        students[i] = students[i + 1];
    }

    (*count)--;

    printf("\nStudent record deleted successfully.\n");
    printf("Records saved automatically.\n");
}

/* ---------- Reports ---------- */

void displayPendingStudents(Student students[], int count) {
    int i;
    int found = 0;

    printMainHeader(count);
    printf("\nPENDING FEE STUDENTS\n");
    printLine('-', 118);

    if (count == 0) {
        printf("\nNo records available.\n");
        return;
    }

    printTableHeader();

    for (i = 0; i < count; i++) {
        if (getPendingFee(students[i]) > 0) {
            printStudentRow(students[i], found + 1);
            found++;
        }
    }

    printLine('-', 118);

    if (found == 0) {
        printf("\nNo pending fee records found.\n");
    } else {
        printf("\nTotal Pending Students: %d\n", found);
    }
}

void displayScholarshipStudents(Student students[], int count) {
    int i;
    int found = 0;

    printMainHeader(count);
    printf("\nSCHOLARSHIP STUDENTS\n");
    printLine('-', 118);

    if (count == 0) {
        printf("\nNo records available.\n");
        return;
    }

    printTableHeader();

    for (i = 0; i < count; i++) {
        if (strcmp(students[i].scholarship, "Scholarship") == 0) {
            printStudentRow(students[i], found + 1);
            found++;
        }
    }

    printLine('-', 118);

    if (found == 0) {
        printf("\nNo scholarship student records found.\n");
    } else {
        printf("\nTotal Scholarship Students: %d\n", found);
    }
}

void displayNonScholarshipStudents(Student students[], int count) {
    int i;
    int found = 0;

    printMainHeader(count);
    printf("\nNON-SCHOLARSHIP STUDENTS\n");
    printLine('-', 118);

    if (count == 0) {
        printf("\nNo records available.\n");
        return;
    }

    printTableHeader();

    for (i = 0; i < count; i++) {
        if (strcmp(students[i].scholarship, "Non-Scholarship") == 0) {
            printStudentRow(students[i], found + 1);
            found++;
        }
    }

    printLine('-', 118);

    if (found == 0) {
        printf("\nNo non-scholarship student records found.\n");
    } else {
        printf("\nTotal Non-Scholarship Students: %d\n", found);
    }
}

/* ---------- Sorting ---------- */

void sortByPendingFee(Student students[], int count) {
    Student temp[MAX_STUDENTS];
    int i;

    printMainHeader(count);
    printf("\nSTUDENTS SORTED BY PENDING FEE - HIGHEST FIRST\n");
    printLine('-', 118);

    if (count == 0) {
        printf("\nNo records available.\n");
        return;
    }

    for (i = 0; i < count; i++) {
        temp[i] = students[i];
    }

    qsort(temp, count, sizeof(Student), comparePendingDesc);

    printTableHeader();

    for (i = 0; i < count; i++) {
        printStudentRow(temp[i], i + 1);
    }

    printLine('-', 118);
}

void sortByName(Student students[], int count) {
    Student temp[MAX_STUDENTS];
    int i;

    printMainHeader(count);
    printf("\nSTUDENTS SORTED BY NAME - A TO Z\n");
    printLine('-', 118);

    if (count == 0) {
        printf("\nNo records available.\n");
        return;
    }

    for (i = 0; i < count; i++) {
        temp[i] = students[i];
    }

    qsort(temp, count, sizeof(Student), compareNameAsc);

    printTableHeader();

    for (i = 0; i < count; i++) {
        printStudentRow(temp[i], i + 1);
    }

    printLine('-', 118);
}

int comparePendingDesc(const void *a, const void *b) {
    Student s1 = *(Student *)a;
    Student s2 = *(Student *)b;

    double p1 = getPendingFee(s1);
    double p2 = getPendingFee(s2);

    if (p1 < p2) return 1;
    if (p1 > p2) return -1;
    return 0;
}

int compareNameAsc(const void *a, const void *b) {
    Student s1 = *(Student *)a;
    Student s2 = *(Student *)b;

    return strcmp(s1.name, s2.name);
}

/* ---------- Table Display ---------- */

void printTableHeader() {
    printf(
        "%-5s %-14s %-22s %-12s %-8s %-17s %12s %12s %12s\n",
        "S.No",
        "Student ID",
        "Name",
        "Branch",
        "Year",
        "Scholarship",
        "Total",
        "Paid",
        "Pending"
    );

    printLine('-', 118);
}

void printStudentRow(Student s, int serialNo) {
    double total = getTotalFee(s);
    double pending = getPendingFee(s);

    if (pending < 0) {
        pending = 0;
    }

    printf(
        "%-5d %-14.14s %-22.22s %-12.12s %-8.8s %-17.17s %12.2lf %12.2lf %12.2lf\n",
        serialNo,
        s.id,
        s.name,
        s.branch,
        s.year,
        s.scholarship,
        total,
        s.paidFee,
        pending
    );
}
