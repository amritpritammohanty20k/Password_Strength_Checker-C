/*
  password_tool_enhanced.c
  Enhanced Password Strength Checker & Generator
  
  FEATURES:
  - Check password strength
  - Generate strong passwords
  - Save passwords with labels (Gmail, Facebook, etc.)
  - Search passwords by label name
  - Update existing passwords
  - View all saved passwords
  - Delete specific password by label
  - Delete all passwords

  Compile:
    gcc password_tool_enhanced.c -o password_tool

  Run:
    ./password_tool
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_PW_LEN 256
#define MIN_STRONG_LEN 12
#define SAVE_FILENAME "saved_passwords.txt"
#define TEMP_FILENAME "temp_passwords.txt"

/* Forward declarations */
void show_menu();
void check_password_interactive();
void generate_password_interactive();
void save_user_password();
void view_saved_passwords();
void search_by_label();              // NEW - Find password by label
void update_password();              // NEW - Update existing password
void delete_by_label();              // NEW - Delete specific password
void delete_all_passwords();
int count_saved_passwords();
int evaluate_password(const char *pw, int *has_lower, int *has_upper, int *has_digit, int *has_special);
void print_suggestions(const char *pw, int score, int has_lower, int has_upper, int has_digit, int has_special);
char *generate_password(int length);
int ask_yes_no(const char *prompt);
void save_password_to_file(const char *pw);

int main() {
    srand((unsigned) time(NULL));
    int choice;

    printf("=== Password Manager & Generator ===\n");
    
    int saved_count = count_saved_passwords();
    if (saved_count > 0) {
        printf("You have %d saved password(s).\n", saved_count);
    }
    printf("\n");

    while (1) {
        show_menu();
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("Invalid input. Please enter a number.\n\n");
            continue;
        }
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}

        switch (choice) {
            case 1:
                check_password_interactive();
                break;
            case 2:
                generate_password_interactive();
                break;
            case 3:
                save_user_password();
                break;
            case 4:
                view_saved_passwords();
                break;
            case 5:
                search_by_label();           // NEW
                break;
            case 6:
                update_password();           // NEW
                break;
            case 7:
                delete_by_label();           // NEW
                break;
            case 8:
                delete_all_passwords();
                break;
            case 9:
                printf("Exiting. Stay secure!\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
        }
        printf("\n");
    }

    return 0;
}

/* Print main menu */
void show_menu() {
    printf("===== MENU =====\n");
    printf("  1. Check password strength\n");
    printf("  2. Generate a strong password\n");
    printf("  3. Save a new password\n");
    printf("  4. View all saved passwords\n");
    printf("  5. Search password by label\n");        // NEW
    printf("  6. Update a password\n");                // NEW
    printf("  7. Delete a password\n");                // NEW
    printf("  8. Delete all passwords\n");
    printf("  9. Exit\n");
    printf("================\n");
}

/* Interactive password strength checker */
void check_password_interactive() {
    char pw[MAX_PW_LEN];

    printf("\nEnter the password to evaluate: ");
    if (scanf("%255s", pw) != 1) {
        printf("Input error.\n");
        return;
    }

    int has_lower = 0, has_upper = 0, has_digit = 0, has_special = 0;
    int score = evaluate_password(pw, &has_lower, &has_upper, &has_digit, &has_special);

    printf("\nPassword: %s\n", pw);
    printf("Score: %d/10  ", score);

    if (score >= 8)           printf("[STRONG]\n");
    else if (score >= 5)      printf("[MODERATE]\n");
    else                      printf("[WEAK]\n");

    print_suggestions(pw, score, has_lower, has_upper, has_digit, has_special);
}

/* Interactive generator */
void generate_password_interactive() {
    int length;
    printf("\nEnter desired password length (suggested >= %d): ", MIN_STRONG_LEN);
    if (scanf("%d", &length) != 1) {
        int c; while ((c = getchar()) != '\n' && c != EOF) {}
        printf("Invalid input.\n");
        return;
    }
    if (length < 4) {
        printf("Length too short. Using minimum length 4.\n");
        length = 4;
    }
    if (length > 200) {
        printf("Length too large; limiting to 200.\n");
        length = 200;
    }

    char *pw = generate_password(length);
    if (!pw) {
        printf("Memory error.\n");
        return;
    }

    printf("\n=== GENERATED PASSWORD ===\n");
    printf("%s\n", pw);
    printf("==========================\n");

    int has_lower = 0, has_upper = 0, has_digit = 0, has_special = 0;
    int score = evaluate_password(pw, &has_lower, &has_upper, &has_digit, &has_special);
    printf("\nStrength score: %d/10  ", score);
    if (score >= 8)           printf("[STRONG]\n");
    else if (score >= 5)      printf("[MODERATE]\n");
    else                      printf("[WEAK]\n");

    if (ask_yes_no("\nSave this password? (y/n): ")) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        
        char label[100];
        printf("Enter a label (e.g., Gmail, Facebook): ");
        if (fgets(label, sizeof(label), stdin) != NULL) {
            label[strcspn(label, "\n")] = '\0';
        }
        
        if (strlen(label) > 0) {
            FILE *f = fopen(SAVE_FILENAME, "a");
            if (f) {
                time_t t = time(NULL);
                struct tm *lt = localtime(&t);
                char buf[64];
                strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
                fprintf(f, "[%s] [%s] %s\n", buf, label, pw);
                fclose(f);
                printf("Password saved with label '%s'!\n", label);
            }
        }
    }

    free(pw);
}

/* Save user's own password with label */
void save_user_password() {
    char pw[MAX_PW_LEN];
    char label[100];
    
    printf("\n=== Save Your Password ===\n");
    printf("Enter the password: ");
    if (scanf("%255s", pw) != 1) {
        printf("Input error.\n");
        return;
    }
    
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
    
    printf("Enter a label (e.g., Gmail, Facebook, Netflix): ");
    if (fgets(label, sizeof(label), stdin) != NULL) {
        label[strcspn(label, "\n")] = '\0';
    }
    
    if (strlen(label) == 0) {
        printf("Label is required! Password not saved.\n");
        return;
    }
    
    int has_lower = 0, has_upper = 0, has_digit = 0, has_special = 0;
    int score = evaluate_password(pw, &has_lower, &has_upper, &has_digit, &has_special);
    
    printf("\nPassword strength: %d/10  ", score);
    if (score >= 8)           printf("[STRONG]\n");
    else if (score >= 5)      printf("[MODERATE]\n");
    else                      printf("[WEAK]\n");
    
    if (score < 5) {
        printf("\nWARNING: This password is weak!\n");
        if (!ask_yes_no("Save anyway? (y/n): ")) {
            printf("Password not saved.\n");
            return;
        }
    }
    
    FILE *f = fopen(SAVE_FILENAME, "a");
    if (!f) {
        printf("Error opening file.\n");
        return;
    }
    
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
    fprintf(f, "[%s] [%s] %s\n", buf, label, pw);
    fclose(f);
    
    printf("\n✓ Password saved for '%s'!\n", label);
}

/* NEW: Search password by label */
void search_by_label() {
    char search_label[100];
    
    printf("\nEnter label to search (e.g., Gmail, Facebook): ");
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
    
    if (fgets(search_label, sizeof(search_label), stdin) == NULL) {
        printf("Input error.\n");
        return;
    }
    search_label[strcspn(search_label, "\n")] = '\0';
    
    if (strlen(search_label) == 0) {
        printf("Please enter a label.\n");
        return;
    }
    
    FILE *f = fopen(SAVE_FILENAME, "r");
    if (!f) {
        printf("No saved passwords found.\n");
        return;
    }
    
    printf("\n=== Search Results for '%s' ===\n", search_label);
    char line[512];
    int found = 0;
    
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, search_label) != NULL) {
            printf("%s", line);
            found++;
        }
    }
    
    fclose(f);
    
    if (found == 0) {
        printf("No passwords found with label '%s'.\n", search_label);
    } else {
        printf("\n✓ Found %d match(es).\n", found);
    }
}

/* NEW: Update existing password */
void update_password() {
    char label[100];
    
    printf("\n=== Update Password ===\n");
    printf("Enter label of password to update: ");
    
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
    
    if (fgets(label, sizeof(label), stdin) == NULL) {
        printf("Input error.\n");
        return;
    }
    label[strcspn(label, "\n")] = '\0';
    
    if (strlen(label) == 0) {
        printf("Label required.\n");
        return;
    }
    
    FILE *f = fopen(SAVE_FILENAME, "r");
    if (!f) {
        printf("No saved passwords.\n");
        return;
    }
    
    FILE *temp = fopen(TEMP_FILENAME, "w");
    if (!temp) {
        fclose(f);
        printf("Error creating temp file.\n");
        return;
    }
    
    char line[512];
    int found = 0;
    
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, label) != NULL && !found) {
            printf("Current: %s", line);
            found = 1;
            
            char new_pw[MAX_PW_LEN];
            printf("Enter new password: ");
            if (scanf("%255s", new_pw) == 1) {
                time_t t = time(NULL);
                struct tm *lt = localtime(&t);
                char buf[64];
                strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
                fprintf(temp, "[%s] [%s] %s\n", buf, label, new_pw);
            } else {
                fprintf(temp, "%s", line);
            }
        } else {
            fprintf(temp, "%s", line);
        }
    }
    
    fclose(f);
    fclose(temp);
    
    if (found) {
        remove(SAVE_FILENAME);
        rename(TEMP_FILENAME, SAVE_FILENAME);
        printf("\n✓ Password updated for '%s'!\n", label);
    } else {
        remove(TEMP_FILENAME);
        printf("Label '%s' not found.\n", label);
    }
}

/* NEW: Delete specific password by label */
void delete_by_label() {
    char label[100];
    
    printf("\n=== Delete Password ===\n");
    printf("Enter label to delete: ");
    
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
    
    if (fgets(label, sizeof(label), stdin) == NULL) {
        printf("Input error.\n");
        return;
    }
    label[strcspn(label, "\n")] = '\0';
    
    if (strlen(label) == 0) {
        printf("Label required.\n");
        return;
    }
    
    FILE *f = fopen(SAVE_FILENAME, "r");
    if (!f) {
        printf("No saved passwords.\n");
        return;
    }
    
    FILE *temp = fopen(TEMP_FILENAME, "w");
    if (!temp) {
        fclose(f);
        printf("Error creating temp file.\n");
        return;
    }
    
    char line[512];
    int found = 0;
    
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, label) != NULL && !found) {
            printf("Found: %s", line);
            found = 1;
            // Don't write this line (delete it)
        } else {
            fprintf(temp, "%s", line);
        }
    }
    
    fclose(f);
    fclose(temp);
    
    if (found) {
        remove(SAVE_FILENAME);
        rename(TEMP_FILENAME, SAVE_FILENAME);
        printf("\n✓ Password deleted for '%s'!\n", label);
    } else {
        remove(TEMP_FILENAME);
        printf("Label '%s' not found.\n", label);
    }
}

/* View all saved passwords */
void view_saved_passwords() {
    FILE *f = fopen(SAVE_FILENAME, "r");
    if (!f) {
        printf("\nNo saved passwords found.\n");
        return;
    }

    printf("\n===== ALL SAVED PASSWORDS =====\n");
    char line[512];
    int count = 0;
    
    while (fgets(line, sizeof(line), f)) {
        count++;
        printf("%d. %s", count, line);
    }
    
    fclose(f);
    
    if (count == 0) {
        printf("No passwords saved yet.\n");
    } else {
        printf("\nTotal: %d password(s)\n", count);
    }
}

/* Delete all saved passwords */
void delete_all_passwords() {
    int count = count_saved_passwords();
    
    if (count == 0) {
        printf("\nNo saved passwords to delete.\n");
        return;
    }
    
    printf("\nYou have %d saved password(s).\n", count);
    printf("WARNING: This will delete ALL passwords!\n");
    
    if (ask_yes_no("Are you sure? (y/n): ")) {
        if (remove(SAVE_FILENAME) == 0) {
            printf("✓ All passwords deleted.\n");
        } else {
            printf("Error deleting file.\n");
        }
    } else {
        printf("Cancelled.\n");
    }
}

/* Count saved passwords */
int count_saved_passwords() {
    FILE *f = fopen(SAVE_FILENAME, "r");
    if (!f) return 0;
    
    int count = 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        count++;
    }
    
    fclose(f);
    return count;
}

/* Evaluate password */
int evaluate_password(const char *pw, int *has_lower, int *has_upper, int *has_digit, int *has_special) {
    int len = (int) strlen(pw);
    int score = 0;

    if (len >= MIN_STRONG_LEN) score += 4;
    else if (len >= 10) score += 3;
    else if (len >= 8) score += 2;
    else if (len >= 6) score += 1;

    for (int i = 0; pw[i] != '\0'; ++i) {
        if (islower((unsigned char)pw[i])) *has_lower = 1;
        else if (isupper((unsigned char)pw[i])) *has_upper = 1;
        else if (isdigit((unsigned char)pw[i])) *has_digit = 1;
        else *has_special = 1;
    }

    score += (*has_lower) ? 1 : 0;
    score += (*has_upper) ? 1 : 0;
    score += (*has_digit) ? 1 : 0;
    score += (*has_special) ? 1 : 0;

    int types = (*has_lower) + (*has_upper) + (*has_digit) + (*has_special);
    if (len >= MIN_STRONG_LEN && types >= 3) score += 1;

    if (score > 10) score = 10;
    return score;
}

/* Print suggestions */
void print_suggestions(const char *pw, int score, int has_lower, int has_upper, int has_digit, int has_special) {
    int len = (int) strlen(pw);
    printf("\nSuggestions:\n");

    if (score >= 8) {
        printf("  - Excellent! Password is strong.\n");
        return;
    }

    if (len < MIN_STRONG_LEN) {
        printf("  - Use at least %d characters.\n", MIN_STRONG_LEN);
    }

    if (!has_lower)  printf("  - Add lowercase letters (a-z).\n");
    if (!has_upper)  printf("  - Add uppercase letters (A-Z).\n");
    if (!has_digit)  printf("  - Add digits (0-9).\n");
    if (!has_special)printf("  - Add special characters (!@#$).\n");

    printf("  - Avoid common patterns (1234, qwerty).\n");
}

/* Generate password */
char *generate_password(int length) {
    const char *lower = "abcdefghijklmnopqrstuvwxyz";
    const char *upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char *digits = "0123456789";
    const char *special = "!@#$%^&*()-_=+[]{};:,.<>?/";

    int n_lower = (int) strlen(lower);
    int n_upper = (int) strlen(upper);
    int n_digits = (int) strlen(digits);
    int n_special = (int) strlen(special);

    char *pw = (char *) malloc((size_t) length + 1);
    if (!pw) return NULL;

    int pos = 0;

    if (length >= 4) {
        pw[pos++] = lower[rand() % n_lower];
        pw[pos++] = upper[rand() % n_upper];
        pw[pos++] = digits[rand() % n_digits];
        pw[pos++] = special[rand() % n_special];
    }

    const char *all = "abcdefghijklmnopqrstuvwxyz"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "0123456789"
                      "!@#$%^&*()-_=+[]{};:,.<>?/";
    int n_all = (int) strlen(all);

    while (pos < length) {
        pw[pos++] = all[rand() % n_all];
    }

    for (int i = 0; i < length; ++i) {
        int j = rand() % length;
        char tmp = pw[i];
        pw[i] = pw[j];
        pw[j] = tmp;
    }

    pw[length] = '\0';
    return pw;
}

/* Ask yes/no */
int ask_yes_no(const char *prompt) {
    char resp[8];
    printf("%s", prompt);
    if (scanf("%7s", resp) != 1) {
        int c; while ((c = getchar()) != '\n' && c != EOF) {}
        return 0;
    }
    int c; while ((c = getchar()) != '\n' && c != EOF) {}

    if (resp[0] == 'y' || resp[0] == 'Y') return 1;
    return 0;
}

/* Save password to file */
void save_password_to_file(const char *pw) {
    FILE *f = fopen(SAVE_FILENAME, "a");
    if (!f) {
        printf("Error opening file.\n");
        return;
    }
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
    fprintf(f, "[%s] %s\n", buf, pw);
    fclose(f);
    printf("Saved!\n");
}