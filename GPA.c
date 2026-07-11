#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_COURSES 200
#define MAX_NAME_LEN 50
#define COURSE_NAME_LEN 50
#define DATA_FILE "../other/student_data.dat"
#define OUTPUT_DIR "../other/"
#define MAX_SEMESTER 8

// Grade table - Tsinghua University standard
typedef struct {
    char letter[4];
    float point;
} GradeMap;

GradeMap grade_table[] = {
    {"A+", 4.0}, {"A", 4.0}, {"A-", 4.0},
    {"B+", 3.6}, {"B", 3.3}, {"B-", 3.0},
    {"C+", 2.6}, {"C", 2.3}, {"C-", 2.0},
    {"D+", 1.6}, {"D", 1.3}, {"F", 0.0}
};
int grade_count = 12;

typedef struct {
    char name[COURSE_NAME_LEN];
    int credits;
    int semester;
    float grade;
    int is_major_course;
    int is_pf;  // Pass/Fail course: 1=Yes, 0=No
} Course;

typedef struct {
    char name[MAX_NAME_LEN];
    char id[20];
    char major[50];
    int required_credits;
    int major_credits_required;
    Course courses[MAX_COURSES];
    int course_count;
} Student;

// ========== UTILITY FUNCTIONS ==========

void clear_input() {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

int my_strcasecmp(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 1;
        a++;
        b++;
    }
    return (*a || *b) ? 1 : 0;
}

char *my_strcasestr(const char *haystack, const char *needle) {
    if (!*needle) return (char*)haystack;
    for (; *haystack; haystack++) {
        const char *h = haystack;
        const char *n = needle;
        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            h++;
            n++;
        }
        if (!*n) return (char*)haystack;
    }
    return NULL;
}

// ========== GRADE CONVERSION ==========

float letter_to_grade(char *letter) {
    for (int i = 0; i < grade_count; i++) {
        if (my_strcasecmp(grade_table[i].letter, letter) == 0)
            return grade_table[i].point;
    }
    return -1;
}

void grade_to_letter(float grade, char *letter) {
    for (int i = 0; i < grade_count; i++) {
        if (grade == grade_table[i].point) {
            strcpy(letter, grade_table[i].letter);
            return;
        }
    }
    strcpy(letter, "F");
}

int is_valid_grade(float grade) {
    float allowed[] = {4.0, 3.6, 3.3, 3.0, 2.6, 2.3, 2.0, 1.6, 1.3, 0.0};
    for (int i = 0; i < 10; i++) {
        if (fabs(grade - allowed[i]) < 0.01) return 1;
    }
    return 0;
}

// ========== DATA PERSISTENCE ==========

void save_data(Student *s) {
    // Auto-create the output directory if it doesn't exist
    system("if not exist \"..\\other\" mkdir \"..\\other\"");
    FILE *f = fopen(DATA_FILE, "wb");
    if (f) {
        fwrite(s, sizeof(Student), 1, f);
        fclose(f);
        printf("[OK] Data saved successfully!\n");
    } else {
        printf("[ERROR] Cannot save data!\n");
    }
}

int load_data(Student *s) {
    FILE *f = fopen(DATA_FILE, "rb");
    if (f) {
        if (fread(s, sizeof(Student), 1, f) != 1) {
            printf("[ERROR] Corrupted file! Starting fresh.\n");
            fclose(f);
            return 0;
        }
        fclose(f);
        printf("[OK] Data loaded successfully!\n");
        return 1;
    }
    return 0;
}

// ========== STUDENT PROFILE ==========

void input_student_profile(Student *s) {
    printf("\n========================================\n");
    printf("         STUDENT PROFILE\n");
    printf("========================================\n");
    printf("Enter student name: ");
    fgets(s->name, MAX_NAME_LEN, stdin);
    s->name[strcspn(s->name, "\n")] = '\0';
    s->name[strcspn(s->name, "\r")] = '\0';

    printf("Enter student ID: ");
    fgets(s->id, 20, stdin);
    s->id[strcspn(s->id, "\n")] = '\0';
    s->id[strcspn(s->id, "\r")] = '\0';

    printf("Enter major: ");
    fgets(s->major, 50, stdin);
    s->major[strcspn(s->major, "\n")] = '\0';
    s->major[strcspn(s->major, "\r")] = '\0';

    printf("Enter total required credits for graduation: ");
    scanf("%d", &s->required_credits);
    printf("Enter major credits required: ");
    scanf("%d", &s->major_credits_required);
    clear_input();
    save_data(s);
}

void show_student_profile(Student *s) {
    printf("\n========================================\n");
    printf("         STUDENT PROFILE\n");
    printf("========================================\n");
    printf("Name: %s\n", s->name);
    printf("ID: %s\n", s->id);
    printf("Major: %s\n", s->major);
    printf("Required Credits: %d\n", s->required_credits);
    printf("Major Credits Required: %d\n", s->major_credits_required);
    printf("========================================\n");
}

// ========== COURSE MANAGEMENT ==========

int course_exists(Student *s, char *name) {
    for (int i = 0; i < s->course_count; i++) {
        if (my_strcasecmp(s->courses[i].name, name) == 0)
            return 1;
    }
    return 0;
}

void add_course(Student *s) {
    if (s->course_count >= MAX_COURSES) {
        printf("Cannot add more courses (max %d).\n", MAX_COURSES);
        return;
    }

    Course *c = &s->courses[s->course_count];
    char grade_input[10];

    printf("\n=== ADD NEW COURSE ===\n");
    printf("Course name: ");
    fgets(c->name, COURSE_NAME_LEN, stdin);
    c->name[strcspn(c->name, "\n")] = '\0';
    c->name[strcspn(c->name, "\r")] = '\0';

    if (course_exists(s, c->name)) {
        printf("[ERROR] Course '%s' already exists!\n", c->name);
        return;
    }

    printf("Credits (1-6): ");
    scanf("%d", &c->credits);
    while (c->credits < 1 || c->credits > 6) {
        printf("Invalid credits. Enter again (1-6): ");
        scanf("%d", &c->credits);
    }

    printf("Semester (1-%d): ", MAX_SEMESTER);
    scanf("%d", &c->semester);
    while (c->semester < 1 || c->semester > MAX_SEMESTER) {
        printf("Invalid semester! Enter again (1-%d): ", MAX_SEMESTER);
        scanf("%d", &c->semester);
    }

    printf("Is this a P/F course? (1=Yes, 0=No): ");
    int temp;
    scanf("%d", &temp);
    c->is_pf = (temp != 0);

    if (c->is_pf) {
        c->grade = 0.0;  // P/F courses have no GPA grade
    } else {
        printf("Grade (4.0,3.6,3.3,3.0,2.6,2.3,2.0,1.6,1.3,0.0 or A+,A,A-,B+,B,B-,C+,C,C-,D+,D,F): ");
        scanf("%s", grade_input);

        if (isalpha(grade_input[0])) {
            c->grade = letter_to_grade(grade_input);
            if (c->grade < 0) {
                printf("Invalid grade letter. Using 0.0\n");
                c->grade = 0.0;
            }
        } else {
            c->grade = atof(grade_input);
        }

        while (!is_valid_grade(c->grade)) {
            printf("Invalid grade! Allowed: 4.0,3.6,3.3,3.0,2.6,2.3,2.0,1.6,1.3,0.0\n");
            printf("Enter grade again: ");
            scanf("%s", grade_input);
            if (isalpha(grade_input[0])) {
                c->grade = letter_to_grade(grade_input);
            } else {
                c->grade = atof(grade_input);
            }
        }
    }

    printf("Is this a major course? (1=Yes, 0=No): ");
    scanf("%d", &temp);
    c->is_major_course = (temp != 0);

    s->course_count++;
    printf("\n[OK] Course added successfully!\n");
    if (c->is_pf)
        printf("     Note: This P/F course counts toward credits but NOT toward GPA.\n");
    save_data(s);
}

void delete_course(Student *s) {
    if (s->course_count == 0) {
        printf("No courses to delete!\n");
        return;
    }

    printf("\n=== DELETE COURSE ===\n");
    for (int i = 0; i < s->course_count; i++) {
        char letter[4];
        grade_to_letter(s->courses[i].grade, letter);
        printf("%d. %s (Sem %d, %.2f - %s)\n",
               i+1, s->courses[i].name,
               s->courses[i].semester,
               s->courses[i].grade, letter);
    }

    int index;
    printf("Enter course number to delete (1-%d): ", s->course_count);
    scanf("%d", &index);
    clear_input();

    if (index < 1 || index > s->course_count) {
        printf("Invalid selection!\n");
        return;
    }

    printf("Are you sure you want to delete '%s'? (y/n): ", s->courses[index-1].name);
    char confirm[4];
    fgets(confirm, sizeof(confirm), stdin);
    if (confirm[0] != 'y' && confirm[0] != 'Y') {
        printf("Cancelled.\n");
        return;
    }

    for (int i = index-1; i < s->course_count - 1; i++) {
        s->courses[i] = s->courses[i+1];
    }
    s->course_count--;
    save_data(s);
    printf("[OK] Course deleted successfully!\n");
}

void edit_course(Student *s) {
    if (s->course_count == 0) {
        printf("No courses to edit!\n");
        return;
    }

    printf("\n=== EDIT COURSE ===\n");
    for (int i = 0; i < s->course_count; i++) {
        char letter[4];
        grade_to_letter(s->courses[i].grade, letter);
        printf("%d. %s | Sem %d | %.2f (%s) | %d credits | %s%s\n",
               i+1, s->courses[i].name, s->courses[i].semester,
               s->courses[i].grade, letter, s->courses[i].credits,
               s->courses[i].is_major_course ? "Major" : "Elective",
               s->courses[i].is_pf ? " [P/F]" : "");
    }

    int index;
    printf("Enter course number to edit (1-%d): ", s->course_count);
    scanf("%d", &index);

    if (index < 1 || index > s->course_count) {
        printf("Invalid selection!\n");
        return;
    }

    Course *c = &s->courses[index-1];
    char grade_input[10];

    clear_input();

    printf("Current name: %s\n", c->name);
    printf("Enter new name (or press Enter to keep): ");
    char new_name[COURSE_NAME_LEN];
    fgets(new_name, COURSE_NAME_LEN, stdin);
    new_name[strcspn(new_name, "\n")] = '\0';
    new_name[strcspn(new_name, "\r")] = '\0';
    if (strlen(new_name) > 0) {
        if (course_exists(s, new_name) && my_strcasecmp(new_name, c->name) != 0) {
            printf("[ERROR] Course '%s' already exists!\n", new_name);
            return;
        }
        strcpy(c->name, new_name);
    }

    printf("Current credits: %d\n", c->credits);
    printf("Enter new credits (1-6, or 0 to keep): ");
    int new_credits;
    scanf("%d", &new_credits);
    if (new_credits >= 1 && new_credits <= 6) {
        c->credits = new_credits;
    }

    printf("Current semester: %d\n", c->semester);
    printf("Enter new semester (1-%d, or 0 to keep): ", MAX_SEMESTER);
    int new_sem;
    scanf("%d", &new_sem);
    if (new_sem >= 1 && new_sem <= MAX_SEMESTER) {
        c->semester = new_sem;
    }

    printf("Current grade: %.2f\n", c->grade);
    printf("Enter new grade (4.0,3.6,3.3,3.0,2.6,2.3,2.0,1.6,1.3,0.0): ");
    scanf("%s", grade_input);

    if (isalpha(grade_input[0])) {
        float new_grade = letter_to_grade(grade_input);
        if (new_grade >= 0 && is_valid_grade(new_grade)) {
            c->grade = new_grade;
        }
    } else {
        float new_grade = atof(grade_input);
        if (is_valid_grade(new_grade)) {
            c->grade = new_grade;
        }
    }

    printf("Is major course? (1=Yes, 0=No, current: %d): ", c->is_major_course);
    int temp;
    scanf("%d", &temp);
    if (temp == 0 || temp == 1) {
        c->is_major_course = temp;
    }

    printf("Is this a P/F course? (1=Yes, 0=No, current: %d): ", c->is_pf);
    scanf("%d", &temp);
    if (temp == 0 || temp == 1) {
        c->is_pf = temp;
    }

    save_data(s);
    printf("[OK] Course updated successfully!\n");
}

// ========== SORT FUNCTIONS using qsort ==========

int compare_by_semester(const void *a, const void *b) {
    Course *ca = (Course*)a;
    Course *cb = (Course*)b;
    return ca->semester - cb->semester;
}

int compare_by_grade(const void *a, const void *b) {
    Course *ca = (Course*)a;
    Course *cb = (Course*)b;
    if (cb->grade > ca->grade) return 1;
    if (cb->grade < ca->grade) return -1;
    return 0;
}

int compare_by_name(const void *a, const void *b) {
    Course *ca = (Course*)a;
    Course *cb = (Course*)b;
    return strcmp(ca->name, cb->name);
}

void sort_courses_by_semester(Student *s) {
    qsort(s->courses, s->course_count, sizeof(Course), compare_by_semester);
    printf("[OK] Courses sorted by semester!\n");
}

void sort_courses_by_grade(Student *s) {
    qsort(s->courses, s->course_count, sizeof(Course), compare_by_grade);
    printf("[OK] Courses sorted by grade (highest first)!\n");
}

void sort_courses_by_name(Student *s) {
    qsort(s->courses, s->course_count, sizeof(Course), compare_by_name);
    printf("[OK] Courses sorted by name!\n");
}

// ========== SEARCH FUNCTIONS ==========

void search_course_by_name(Student *s) {
    if (s->course_count == 0) {
        printf("No courses to search!\n");
        return;
    }

    clear_input();
    char search_name[COURSE_NAME_LEN];
    printf("Enter course name (or part of it): ");
    fgets(search_name, COURSE_NAME_LEN, stdin);
    search_name[strcspn(search_name, "\n")] = '\0';
    search_name[strcspn(search_name, "\r")] = '\0';

    int found = 0;
    printf("\n=== SEARCH RESULTS ===\n");
    for (int i = 0; i < s->course_count; i++) {
        if (my_strcasestr(s->courses[i].name, search_name)) {
            char letter[4];
            grade_to_letter(s->courses[i].grade, letter);
            printf("%d. %s | Sem %d | %.2f (%s) | %d credits | %s%s\n",
                   i+1, s->courses[i].name, s->courses[i].semester,
                   s->courses[i].grade, letter, s->courses[i].credits,
                   s->courses[i].is_major_course ? "Major" : "Elective",
                   s->courses[i].is_pf ? " [P/F]" : "");
            found = 1;
        }
    }
    if (!found) {
        printf("No courses found with name containing '%s'\n", search_name);
    }
}

void search_course_by_semester(Student *s) {
    if (s->course_count == 0) {
        printf("No courses to search!\n");
        return;
    }

    int sem;
    printf("Enter semester (1-%d): ", MAX_SEMESTER);
    scanf("%d", &sem);

    int found = 0;
    printf("\n=== SEARCH RESULTS ===\n");
    for (int i = 0; i < s->course_count; i++) {
        if (s->courses[i].semester == sem) {
            char letter[4];
            grade_to_letter(s->courses[i].grade, letter);
            printf("%d. %s | %.2f (%s) | %d credits | %s%s\n",
                   i+1, s->courses[i].name, s->courses[i].grade,
                   letter, s->courses[i].credits,
                   s->courses[i].is_major_course ? "Major" : "Elective",
                   s->courses[i].is_pf ? " [P/F]" : "");
            found = 1;
        }
    }
    if (!found) {
        printf("No courses found in semester %d\n", sem);
    }
}

// ========== GPA CALCULATIONS ==========

float calculate_cumulative_gpa(Student *s) {
    float total_points = 0;
    int total_credits = 0;
    for (int i = 0; i < s->course_count; i++) {
        if (s->courses[i].is_pf) continue;  // P/F courses don't affect GPA
        total_points += s->courses[i].grade * s->courses[i].credits;
        total_credits += s->courses[i].credits;
    }
    return total_credits == 0 ? 0 : total_points / total_credits;
}

float calculate_major_gpa(Student *s) {
    float total_points = 0;
    int total_credits = 0;
    for (int i = 0; i < s->course_count; i++) {
        if (s->courses[i].is_pf) continue;  // P/F courses don't affect GPA
        if (s->courses[i].is_major_course) {
            total_points += s->courses[i].grade * s->courses[i].credits;
            total_credits += s->courses[i].credits;
        }
    }
    return total_credits == 0 ? 0 : total_points / total_credits;
}

// Show GPA - NUMBERS ONLY, no letter conversion
void show_gpa(Student *s) {
    printf("\n========================================\n");
    printf("            GPA CALCULATOR\n");
    printf("========================================\n");
    printf("Cumulative GPA: %.3f\n", calculate_cumulative_gpa(s));
    printf("Major GPA: %.3f\n", calculate_major_gpa(s));
    printf("========================================\n");
}

// ========== SEMESTER TREND ==========

void semester_trend_analysis(Student *s) {
    printf("\n========================================\n");
    printf("      SEMESTER TREND ANALYSIS\n");
    printf("========================================\n");

    int max_semester = 0;
    for (int i = 0; i < s->course_count; i++) {
        if (s->courses[i].semester > max_semester)
            max_semester = s->courses[i].semester;
    }

    if (max_semester > MAX_SEMESTER) {
        max_semester = MAX_SEMESTER;
    }

    float semester_gpa[MAX_SEMESTER + 1] = {0};
    int semester_credits[MAX_SEMESTER + 1] = {0};

    for (int i = 0; i < s->course_count; i++) {
        int sem = s->courses[i].semester;
        if (sem >= 1 && sem <= MAX_SEMESTER) {
            if (!s->courses[i].is_pf) {
                semester_gpa[sem] += s->courses[i].grade * s->courses[i].credits;
                semester_credits[sem] += s->courses[i].credits;
            }
        }
    }

    printf("\nSemester     GPA         Trend\n");
    printf("----------------------------------------\n");
    float prev_gpa = -1;
    for (int i = 1; i <= max_semester; i++) {
        if (semester_credits[i] > 0) {
            float gpa = semester_gpa[i] / semester_credits[i];
            printf("Sem %d        %.2f        ", i, gpa);
            if (prev_gpa > 0) {
                if (gpa > prev_gpa + 0.05) printf("UP (Improving)");
                else if (gpa < prev_gpa - 0.05) printf("DOWN (Declining)");
                else printf("STABLE");
            }
            printf("\n");
            prev_gpa = gpa;
        }
    }
    printf("========================================\n");
}

// ========== ADVANCED TRANSCRIPT ==========

void advanced_transcript(Student *s) {
    sort_courses_by_semester(s);

    printf("\n========================================\n");
    printf("         ADVANCED TRANSCRIPT\n");
    printf("========================================\n");
    printf("Student: %s (%s)\n", s->name, s->id);
    printf("Major: %s\n", s->major);
    printf("========================================\n");

    for (int sem = 1; sem <= MAX_SEMESTER; sem++) {
        int has_course = 0;
        for (int i = 0; i < s->course_count; i++) {
            if (s->courses[i].semester == sem) {
                if (!has_course) {
                    printf("\n[Semester %d]\n", sem);
                    has_course = 1;
                }
                char letter[4];
                grade_to_letter(s->courses[i].grade, letter);
                printf("  %-30s %2d credits  Grade: %.2f (%s)%s%s\n",
                       s->courses[i].name, s->courses[i].credits,
                       s->courses[i].grade, letter,
                       s->courses[i].is_major_course ? " [Major]" : "",
                       s->courses[i].is_pf ? " [P/F]" : "");
            }
        }
    }

    float cgpa = calculate_cumulative_gpa(s);
    float mgpa = calculate_major_gpa(s);

    // NUMBERS ONLY - no letter conversion for cumulative GPA
    printf("\n========================================\n");
    printf("Cumulative GPA: %.3f\n", cgpa);
    printf("Major GPA: %.3f\n", mgpa);
    printf("========================================\n");
}

// ========== CREDIT TRACKER ==========

void credit_tracker(Student *s) {
    int total_credits = 0;
    int major_credits = 0;
    for (int i = 0; i < s->course_count; i++) {
        total_credits += s->courses[i].credits;
        if (s->courses[i].is_major_course)
            major_credits += s->courses[i].credits;
    }

    float progress = 0;
    if (s->required_credits > 0) {
        progress = (float)total_credits / s->required_credits * 100;
        if (progress > 100) progress = 100;
    }

    int remaining = s->required_credits - total_credits;
    if (remaining < 0) remaining = 0;

    printf("\n========================================\n");
    printf("           CREDIT TRACKER\n");
    printf("========================================\n");
    printf("Total Credits Completed: %d / %d\n", total_credits, s->required_credits);
    printf("Major Credits Completed: %d / %d\n", major_credits, s->major_credits_required);
    printf("Remaining Credits: %d\n", remaining);
    printf("Graduation Progress: %.1f%%\n", progress);
    printf("========================================\n");
}

// ========== COURSE STATISTICS ==========

void course_statistics(Student *s) {
    if (s->course_count == 0) {
        printf("No courses to analyze!\n");
        return;
    }

    float highest = -1;
    float lowest = 5;
    char highest_course[COURSE_NAME_LEN] = "";
    char lowest_course[COURSE_NAME_LEN] = "";
    float total = 0;
    int graded_count = 0;

    for (int i = 0; i < s->course_count; i++) {
        if (s->courses[i].is_pf) continue;  // exclude P/F courses

        float g = s->courses[i].grade;
        total += g;
        graded_count++;

        if (g > highest) {
            highest = g;
            strcpy(highest_course, s->courses[i].name);
        }
        if (g < lowest) {
            lowest = g;
            strcpy(lowest_course, s->courses[i].name);
        }
    }

    printf("\n========================================\n");
    printf("         COURSE STATISTICS\n");
    printf("========================================\n");

    if (graded_count == 0) {
        printf("No graded courses available (all courses are P/F).\n");
        printf("========================================\n");
        return;
    }

    char highest_letter[4], lowest_letter[4];
    grade_to_letter(highest, highest_letter);
    grade_to_letter(lowest, lowest_letter);

    printf("Highest Grade: %.2f (%s) - %s\n", highest, highest_letter, highest_course);
    printf("Lowest Grade: %.2f (%s) - %s\n", lowest, lowest_letter, lowest_course);
    printf("Average Grade: %.2f\n", total / graded_count);
    printf("========================================\n");
}

// ========== GRADE DISTRIBUTION ==========

void grade_distribution(Student *s) {
    if (s->course_count == 0) {
        printf("No courses to analyze!\n");
        return;
    }

    int count_A = 0, count_B = 0, count_C = 0, count_D = 0, count_F = 0;

    for (int i = 0; i < s->course_count; i++) {
        if (s->courses[i].is_pf) continue;  // exclude P/F courses

        float g = s->courses[i].grade;
        if (g >= 3.6) count_A++;
        else if (g >= 3.0) count_B++;
        else if (g >= 2.0) count_C++;
        else if (g >= 1.3) count_D++;
        else count_F++;
    }

    printf("\n========================================\n");
    printf("         GRADE DISTRIBUTION\n");
    printf("========================================\n");
    printf("A (3.6-4.0): %d courses ", count_A);
    for (int i = 0; i < count_A && i < 30; i++) printf("*");
    printf("\n");

    printf("B (3.0-3.59): %d courses ", count_B);
    for (int i = 0; i < count_B && i < 30; i++) printf("*");
    printf("\n");

    printf("C (2.0-2.99): %d courses ", count_C);
    for (int i = 0; i < count_C && i < 30; i++) printf("*");
    printf("\n");

    printf("D (1.3-1.99): %d courses ", count_D);
    for (int i = 0; i < count_D && i < 30; i++) printf("*");
    printf("\n");

    printf("F (0.0-1.29): %d courses ", count_F);
    for (int i = 0; i < count_F && i < 30; i++) printf("*");
    printf("\n");
    printf("========================================\n");
}

// ========== GRADUATION AUDIT ==========

void graduation_audit(Student *s) {
    printf("\n========================================\n");
    printf("         GRADUATION AUDIT\n");
    printf("========================================\n");

    int total_credits = 0;
    int major_credits = 0;
    for (int i = 0; i < s->course_count; i++) {
        total_credits += s->courses[i].credits;
        if (s->courses[i].is_major_course)
            major_credits += s->courses[i].credits;
    }

    float gpa = calculate_cumulative_gpa(s);

    printf("Total credits completed: %d / %d\n", total_credits, s->required_credits);
    printf("Major credits completed: %d / %d\n", major_credits, s->major_credits_required);
    printf("Current GPA: %.2f\n", gpa);

    printf("\n--- REQUIREMENT CHECK ---\n");

    int eligible = 1;

    if (total_credits >= s->required_credits)
        printf("[OK] Credit requirement: MET\n");
    else {
        printf("[NO] Credit requirement: NOT MET (Need %d more credits)\n",
               s->required_credits - total_credits);
        eligible = 0;
    }

    if (major_credits >= s->major_credits_required)
        printf("[OK] Major credit requirement: MET\n");
    else {
        printf("[NO] Major credit requirement: NOT MET (Need %d more major credits)\n",
               s->major_credits_required - major_credits);
        eligible = 0;
    }

    if (gpa >= 2.0)
        printf("[OK] GPA requirement: MET (%.2f >= 2.0)\n", gpa);
    else {
        printf("[NO] GPA requirement: NOT MET (%.2f < 2.0)\n", gpa);
        eligible = 0;
    }

    printf("\n=== FINAL VERDICT ===\n");
    if (eligible && total_credits >= s->required_credits)
        printf("CONGRATULATIONS! You are eligible for graduation!\n");
    else
        printf("You are NOT yet eligible for graduation. Keep working!\n");
    printf("========================================\n");
}

// ========== WHAT-IF ANALYZER ==========

void what_if_analyzer(Student *s) {
    printf("\n========================================\n");
    printf("       WHAT-IF SCENARIO ANALYZER\n");
    printf("========================================\n");

    float current_gpa = calculate_cumulative_gpa(s);
    int current_credits = 0;      // graded credits only (for GPA math)
    int total_credits_done = 0;   // all credits including P/F
    for (int i = 0; i < s->course_count; i++) {
        total_credits_done += s->courses[i].credits;
        if (!s->courses[i].is_pf)
            current_credits += s->courses[i].credits;
    }

    int remaining_credits = s->required_credits - total_credits_done;
    if (remaining_credits <= 0) {
        printf("You've already completed all required credits!\n");
        return;
    }

    printf("Current GPA: %.3f\n", current_gpa);
    printf("Current credits completed: %d (incl. %d P/F)\n", total_credits_done, total_credits_done - current_credits);
    printf("Remaining credits needed: %d\n\n", remaining_credits);

    float targets[] = {3.0, 3.2, 3.5, 3.7, 4.0};
    printf("Scenario Analysis:\n");
    printf("-------------------------------------------------------------\n");
    printf("%-12s %-30s %s\n", "Target GPA", "Required Avg in Remaining", "Feasible?");
    printf("-------------------------------------------------------------\n");

    for (int i = 0; i < 5; i++) {
        float target = targets[i];
        float required = (target * (current_credits + remaining_credits) -
                         current_gpa * current_credits) / remaining_credits;

        if (required > 4.01) {
            char buf[30];
            snprintf(buf, sizeof(buf), "%.2f (IMPOSSIBLE!)", required);
            printf("%-12.2f %-30s %s\n", target, buf, "NO");
        } else if (required < 0) {
            printf("%-12.2f %-30s %s\n", target, "Already achieved", "YES");
        } else {
            char letter[4];
            grade_to_letter(required, letter);
            char buf[30];
            snprintf(buf, sizeof(buf), "%.2f (%s)", required, letter);
            printf("%-12.2f %-30s %s\n", target, buf, "YES");
        }
    }
    printf("========================================\n");
}

// ========== FINAL GPA PREDICTOR ==========

void predict_final_gpa(Student *s) {
    printf("\n========================================\n");
    printf("       FINAL GPA PREDICTOR\n");
    printf("========================================\n");

    float current_gpa = calculate_cumulative_gpa(s);
    int current_credits = 0;
    for (int i = 0; i < s->course_count; i++)
        if (!s->courses[i].is_pf)
            current_credits += s->courses[i].credits;

    int remaining_credits = s->required_credits - current_credits;
    if (remaining_credits <= 0) {
        printf("You've already completed all credits!\n");
        printf("Your final GPA will be: %.3f\n", current_gpa);
        return;
    }

    printf("Current GPA: %.3f\n", current_gpa);
    printf("Current credits: %d\n", current_credits);
    printf("Remaining credits: %d\n\n", remaining_credits);

    float target_gpa;
    printf("Enter your target final GPA (e.g., 3.5): ");
    scanf("%f", &target_gpa);

    float needed_gpa = (target_gpa * (current_credits + remaining_credits) -
                        current_gpa * current_credits) / remaining_credits;

    printf("\n--- RESULT ---\n");
    if (needed_gpa > 4.01) {
        printf("IMPOSSIBLE! You need %.2f GPA but max is 4.0\n", needed_gpa);
        float max_gpa = (4.0 * remaining_credits + current_gpa * current_credits) /
                        (current_credits + remaining_credits);
        printf("Maximum achievable GPA: %.3f\n", max_gpa);
    } else if (needed_gpa < 0) {
        printf("Already achieved! You can relax.\n");
    } else {
        char letter[4];
        grade_to_letter(needed_gpa, letter);
        printf("You need to maintain an average of %.2f (%s) in remaining courses.\n", needed_gpa, letter);

        if (needed_gpa >= 3.6)
            printf("Suggestion: Aim for A (3.6-4.0) in all remaining courses!\n");
        else if (needed_gpa >= 3.0)
            printf("Suggestion: Aim for B (3.0-3.5) or higher.\n");
        else if (needed_gpa >= 2.0)
            printf("Suggestion: Aim for C (2.0-2.5) or higher.\n");
    }
    printf("========================================\n");
}

// ========== EXPORT TO EXCEL ==========

void export_to_excel(Student *s) {
    char base_name[80];
    char filepath[200];
    printf("\n=== EXPORT TO EXCEL ===\n");
    printf("Enter filename to export (e.g., transcript): ");
    clear_input();
    fgets(base_name, 80, stdin);
    base_name[strcspn(base_name, "\n")] = '\0';
    base_name[strcspn(base_name, "\r")] = '\0';

    // Strip .csv if user already typed it
    if (strlen(base_name) > 4 && strcmp(base_name + strlen(base_name) - 4, ".csv") == 0)
        base_name[strlen(base_name) - 4] = '\0';

    snprintf(filepath, sizeof(filepath), OUTPUT_DIR "%s.csv", base_name);

    FILE *f = fopen(filepath, "w");
    if (!f) {
        // Fallback: try creating the other/ directory first (Windows mkdir)
        system("if not exist \"..\\other\" mkdir \"..\\other\"");
        f = fopen(filepath, "w");
    }
    if (!f) {
        printf("[ERROR] Cannot create file at %s\n", filepath);
        printf("        Make sure the 'other' folder exists next to the 'code' folder.\n");
        return;
    }

    fprintf(f, "TSINGHUA UNIVERSITY ACADEMIC TRANSCRIPT\n");
    fprintf(f, "Generated by GPA Planner\n");
    fprintf(f, "\n");

    fprintf(f, "Student Information\n");
    fprintf(f, "Student Name,%s\n", s->name);
    fprintf(f, "Student ID,%s\n", s->id);
    fprintf(f, "Major,%s\n", s->major);
    fprintf(f, "Required Credits,%d\n", s->required_credits);
    fprintf(f, "Major Credits Required,%d\n", s->major_credits_required);
    fprintf(f, "\n");

    fprintf(f, "COURSE LIST\n");
    fprintf(f, "##COURSE_DATA_START##\n");
    fprintf(f, "Course Name,Credits,Semester,Grade,Major,PF\n");

    for (int i = 0; i < s->course_count; i++) {
        fprintf(f, "\"%s\",%d,%d,%.2f,%d,%d\n",
                s->courses[i].name,
                s->courses[i].credits,
                s->courses[i].semester,
                s->courses[i].grade,
                s->courses[i].is_major_course,
                s->courses[i].is_pf);
    }

    fprintf(f, "\n\nSTATISTICS SUMMARY\n");
    fprintf(f, "Metric,Value\n");
    fprintf(f, "Cumulative GPA,%.3f\n", calculate_cumulative_gpa(s));
    fprintf(f, "Major GPA,%.3f\n", calculate_major_gpa(s));

    int total_credits = 0, major_credits = 0;
    for (int i = 0; i < s->course_count; i++) {
        total_credits += s->courses[i].credits;
        if (s->courses[i].is_major_course) major_credits += s->courses[i].credits;
    }
    fprintf(f, "Total Credits Completed,%d\n", total_credits);
    fprintf(f, "Major Credits Completed,%d\n", major_credits);

    int remaining = s->required_credits - total_credits;
    if (remaining < 0) remaining = 0;
    fprintf(f, "Remaining Credits,%d\n", remaining);

    float progress = 0;
    if (s->required_credits > 0) {
        progress = (float)total_credits / s->required_credits * 100;
        if (progress > 100) progress = 100;
    }
    fprintf(f, "Graduation Progress,%.1f%%\n", progress);

    fclose(f);

    // Show absolute path so user knows exactly where the file was saved
    char abs_path[512] = "";
#ifdef _WIN32
    _fullpath(abs_path, filepath, sizeof(abs_path));
#else
    if (!realpath(filepath, abs_path)) strcpy(abs_path, filepath);
#endif
    printf("\n[OK] Excel report exported successfully!\n");
    printf("     File saved at: %s\n", strlen(abs_path) > 0 ? abs_path : filepath);
    printf("     Open with: File Explorer -> navigate to the 'other' folder -> double-click the .csv\n");
    printf("     Or in Excel: File -> Open -> navigate to the path above\n");
}

// ========== IMPORT FROM CSV ==========

// Parse a CSV field that may be wrapped in double quotes.
// Returns a pointer to the next character after the comma, or NULL if end of line.
static char *csv_parse_field(char *src, char *dest, int dest_size) {
    int i = 0;
    if (*src == '"') {
        src++; // skip opening quote
        while (*src && !(*src == '"' && *(src+1) != '"')) {
            if (i < dest_size - 1) dest[i++] = *src;
            if (*src == '"' && *(src+1) == '"') src++; // handle escaped quote ""
            src++;
        }
        if (*src == '"') src++; // skip closing quote
        if (*src == ',') src++; // skip comma
    } else {
        while (*src && *src != ',' && *src != '\n' && *src != '\r') {
            if (i < dest_size - 1) dest[i++] = *src;
            src++;
        }
        if (*src == ',') src++;
    }
    dest[i] = '\0';
    return src;
}

void import_from_csv(Student *s) {
    char filename[100];
    printf("\n=== IMPORT FROM CSV ===\n");
    printf("  USAGE TIPS:\n");
    printf("  - To import, first export via Option 12\n");
    printf("  - Enter just the filename if CSV is in the SAME FOLDER\n");
    printf("  - e.g.: transcript.csv\n");
    printf("  - Or enter full path e.g.: C:\\\\Users\\\\YourName\\\\transcript.csv\n\n");
    printf("Enter CSV filename or full path: ");
    clear_input();
    fgets(filename, 100, stdin);
    filename[strcspn(filename, "\n")] = '\0';
    filename[strcspn(filename, "\r")] = '\0';

    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Cannot open file %s!\n", filename);
        printf("CSV format: \"Course Name\",Credits,Semester,Grade,Major(1/0)\n");
        return;
    }

    char line[512];
    int imported = 0;
    int data_started = 0;

    while (fgets(line, sizeof(line), f) && s->course_count < MAX_COURSES) {
        // Trim newline
        line[strcspn(line, "\r\n")] = '\0';

        // Skip blank lines
        if (strlen(line) == 0) continue;

        // Look for the marker written by the export function
        if (strstr(line, "##COURSE_DATA_START##")) {
            data_started = 1;
            // Read one more header line (Course Name,Credits,...)
            fgets(line, sizeof(line), f);
            continue;
        }

        // If marker not seen yet, try to detect header manually:
        // If the first line contains "Course Name" or "course", treat it as a header
        if (!data_started) {
            if (strstr(line, "Course Name") || strstr(line, "course name") ||
                strstr(line, "COURSE") || strstr(line, "No.,")) {
                data_started = 1;
                continue; // skip header
            }
            // If the line has no comma or can't be parsed, skip it
            if (!strchr(line, ',')) continue;
            // No marker but the line looks like data — parse it
            data_started = 1;
        }

        // Skip summary/statistics lines that start with non-data text
        if (strstr(line, "STATISTICS") || strstr(line, "Metric") ||
            strstr(line, "Student") || strstr(line, "Major,") ||
            strstr(line, "Required") || strstr(line, "Generated") ||
            strstr(line, "TSINGHUA")) continue;

        Course c;
        char name[COURSE_NAME_LEN] = "";
        char credits_str[10] = "", sem_str[10] = "";
        char grade_str[10] = "", major_str[10] = "";

        char *p = line;
        p = csv_parse_field(p, name,        COURSE_NAME_LEN);
        p = csv_parse_field(p, credits_str, sizeof(credits_str));
        p = csv_parse_field(p, sem_str,     sizeof(sem_str));
        p = csv_parse_field(p, grade_str,   sizeof(grade_str));
        p = csv_parse_field(p, major_str,   sizeof(major_str));

        char pf_str[10] = "0";
        p = csv_parse_field(p, pf_str, sizeof(pf_str));

        if (strlen(name) == 0) continue;

        c.credits      = atoi(credits_str);
        c.semester     = atoi(sem_str);
        c.is_major_course = atoi(major_str);
        c.is_pf        = atoi(pf_str);

        if (isalpha((unsigned char)grade_str[0])) {
            c.grade = letter_to_grade(grade_str);
            if (c.grade < 0) c.grade = 0.0;
        } else {
            c.grade = atof(grade_str);
        }

        // Validate each field
        if (c.credits < 1 || c.credits > 6) continue;
        if (c.semester < 1 || c.semester > MAX_SEMESTER) continue;
        if (!is_valid_grade(c.grade)) continue;
        if (c.is_major_course != 0 && c.is_major_course != 1) continue;
        if (c.is_pf != 0 && c.is_pf != 1) c.is_pf = 0;  // default to non-P/F if invalid
        if (course_exists(s, name)) {
            printf("[SKIP] '%s' already exists.\n", name);
            continue;
        }

        strcpy(c.name, name);
        s->courses[s->course_count++] = c;
        imported++;
    }

    fclose(f);
    printf("[OK] Imported %d courses successfully!\n", imported);
    if (imported > 0) save_data(s);
}

// ========== ACADEMIC DASHBOARD ==========

void dashboard(Student *s) {
    float cgpa = calculate_cumulative_gpa(s);
    float mgpa = calculate_major_gpa(s);
    int total_credits = 0, major_credits = 0;
    for (int i = 0; i < s->course_count; i++) {
        total_credits += s->courses[i].credits;
        if (s->courses[i].is_major_course) major_credits += s->courses[i].credits;
    }

    float progress = 0;
    if (s->required_credits > 0) {
        progress = (float)total_credits / s->required_credits * 100;
        if (progress > 100) progress = 100;
    }

    printf("\n==================================================\n");
    printf("         TSINGHUA UNIVERSITY DASHBOARD\n");
    printf("==================================================\n");
    printf("Student: %s\n", s->name);
    printf("ID: %s\n", s->id);
    printf("Major: %s\n", s->major);
    printf("--------------------------------------------------\n");

    // NUMBERS ONLY - no letter conversion for GPA
    printf("Cumulative GPA: %.3f\n", cgpa);
    printf("Major GPA: %.3f\n", mgpa);
    printf("Credits: %d / %d (%.1f%%)\n", total_credits, s->required_credits, progress);
    printf("Major Credits: %d / %d\n", major_credits, s->major_credits_required);

    printf("--------------------------------------------------\n");

    if (cgpa >= 3.6)
        printf("CLASSIFICATION (for reference only): First Class Honours (Excellent)\n");
    else if (cgpa >= 3.0)
        printf("CLASSIFICATION (for reference only): Second Class Honours (Good)\n");
    else if (cgpa >= 2.0)
        printf("CLASSIFICATION (for reference only): Third Class Honours (Pass)\n");
    else
        printf("CLASSIFICATION (for reference only): Not eligible for graduation\n");

    printf("==================================================\n");
}

// ========== MENU ==========

void print_menu() {
    printf("\n==================================================\n");
    printf("      TSINGHUA UNIVERSITY GPA PLANNER\n");
    printf("==================================================\n");
    printf("  1. Student Profile Management\n");
    printf("  2. Course Management\n");
    printf("  3. GPA Calculator\n");
    printf("  4. Semester Trend Analysis\n");
    printf("  5. Advanced Transcript\n");
    printf("  6. Credit Tracker\n");
    printf("  7. Course Statistics\n");
    printf("  8. Grade Distribution\n");
    printf("  9. Graduation Audit\n");
    printf(" 10. What-If Scenario Analyzer\n");
    printf(" 11. Final GPA Predictor\n");
    printf(" 12. Export to Excel (CSV)\n");
    printf(" 13. Import from CSV\n");
    printf(" 14. Academic Dashboard\n");
    printf("  0. Save & Exit\n");
    printf("==================================================\n");
    printf("Your choice: ");
}

// ========== MAIN ==========

int main() {
    Student student;
    memset(&student, 0, sizeof(Student));

    printf("\n");
    printf("==================================================\n");
    printf("   WELCOME TO TSINGHUA UNIVERSITY GPA PLANNER\n");
    printf("==================================================\n");
    printf("(4-year University - 8 Semesters)\n");
    printf("==================================================\n");

    if (!load_data(&student)) {
        printf("\nNo existing data found. Please set up your profile first.\n");
        student.required_credits = 140;
        student.major_credits_required = 70;
    }

    int choice;
    do {
        print_menu();
        scanf("%d", &choice);
        clear_input();

        switch(choice) {
            case 1:
                input_student_profile(&student);
                show_student_profile(&student);
                break;
            case 2: {
                int subchoice;
                do {
                    printf("\n--- COURSE MANAGEMENT ---\n");
                    printf("1. Add Course\n");
                    printf("2. View All Courses\n");
                    printf("3. Edit Course\n");
                    printf("4. Delete Course\n");
                    printf("5. Sort by Semester\n");
                    printf("6. Sort by Grade\n");
                    printf("7. Sort by Name\n");
                    printf("8. Search by Name\n");
                    printf("9. Search by Semester\n");
                    printf("0. Back\n");
                    printf("Choice: ");
                    scanf("%d", &subchoice);
                    clear_input();

                    if (subchoice == 1) {
                        add_course(&student);
                    } else if (subchoice == 2) {
                        printf("\n=== ALL COURSES ===\n");
                        if (student.course_count == 0) {
                            printf("No courses added yet.\n");
                        } else {
                            for (int i = 0; i < student.course_count; i++) {
                                char letter[4];
                                grade_to_letter(student.courses[i].grade, letter);
                                printf("%d. %s | %d credits | Sem %d | %.2f (%s) | %s%s\n",
                                       i+1, student.courses[i].name, student.courses[i].credits,
                                       student.courses[i].semester, student.courses[i].grade, letter,
                                       student.courses[i].is_major_course ? "Major" : "Elective",
                                       student.courses[i].is_pf ? " [P/F]" : "");
                            }
                        }
                    } else if (subchoice == 3) {
                        edit_course(&student);
                    } else if (subchoice == 4) {
                        delete_course(&student);
                    } else if (subchoice == 5) {
                        sort_courses_by_semester(&student);
                        save_data(&student);
                    } else if (subchoice == 6) {
                        sort_courses_by_grade(&student);
                        save_data(&student);
                    } else if (subchoice == 7) {
                        sort_courses_by_name(&student);
                        save_data(&student);
                    } else if (subchoice == 8) {
                        search_course_by_name(&student);
                    } else if (subchoice == 9) {
                        search_course_by_semester(&student);
                    }
                } while (subchoice != 0);
                break;
            }
            case 3:
                show_gpa(&student);
                break;
            case 4:
                semester_trend_analysis(&student);
                break;
            case 5:
                advanced_transcript(&student);
                break;
            case 6:
                credit_tracker(&student);
                break;
            case 7:
                course_statistics(&student);
                break;
            case 8:
                grade_distribution(&student);
                break;
            case 9:
                graduation_audit(&student);
                break;
            case 10:
                what_if_analyzer(&student);
                break;
            case 11:
                predict_final_gpa(&student);
                break;
            case 12:
                export_to_excel(&student);
                break;
            case 13:
                import_from_csv(&student);
                break;
            case 14:
                dashboard(&student);
                break;
            case 0:
                save_data(&student);
                printf("\n[OK] Data saved. Goodbye!\n");
                break;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    } while (choice != 0);

    return 0;
}