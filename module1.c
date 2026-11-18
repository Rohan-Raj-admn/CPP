#include "cpp_header.h"

// Helper function to clear leftover characters in the input buffer
static void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Portable case-insensitive string comparison using <ctype.h>
static int match_tail_number(const char *a, const char *b) {
    while (*a && *b) {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}


// 1. Register New Aircraft in Database

void add_aircraft(void) {
    FILE *fp = fopen(Fleet_file, "ab");
    if (fp == NULL) {
        printf("\n[ERROR] Unable to open '%s'.\n", Fleet_file);
        return;
    }

    Aircraft ac;
    memset(&ac, 0, sizeof(Aircraft));

    printf("\n--- REGISTER NEW AIRCRAFT ---\n");

    printf("Enter Aircraft Tail Number (e.g., VT-A320): ");
    clear_input_buffer();
    fgets(ac.tail_number, sizeof(ac.tail_number), stdin);
    ac.tail_number[strcspn(ac.tail_number, "\n")] = '\0';

    // Convert tail number to uppercase for consistent storage
    for (int i = 0; ac.tail_number[i] != '\0'; i++) {
        ac.tail_number[i] = (char)toupper((unsigned char)ac.tail_number[i]);
    }

    printf("Enter Aircraft Model (e.g., Airbus A320): ");
    fgets(ac.model, sizeof(ac.model), stdin);
    ac.model[strcspn(ac.model, "\n")] = '\0';

    printf("Enter Total Flight Hours: ");
    while (scanf("%lf", &ac.fhours) != 1 || ac.fhours < 0.0) {
        printf("Invalid input. Enter positive flight hours: ");
        clear_input_buffer();
    }

    printf("Enter Total Flight Cycles: ");
    while (scanf("%d", &ac.fcycles) != 1 || ac.fcycles < 0) {
        printf("Invalid input. Enter positive flight cycles: ");
        clear_input_buffer();
    }

    // Initialize fatigue index, remaining C-check hours, and grounded flag
    ac.fatigue_index = (ac.fhours / 12000.0) * 60.0 + (ac.fcycles / 8000.0) * 40.0;
    if (ac.fatigue_index > 100.0) {
        ac.fatigue_index = 100.0;
    }

    ac.hrs_rem = 6000.0 - fmod(ac.fhours, 6000.0);
    ac.is_grnded = (ac.fatigue_index >= 85.0 || ac.hrs_rem <= 0.0) ? 1 : 0;

    size_t written = fwrite(&ac, sizeof(Aircraft), 1, fp);
    fclose(fp);

    if (written == 1) {
        printf("\n[SUCCESS] Aircraft '%s' registered successfully!\n", ac.tail_number);
    } else {
        printf("\n[ERROR] Failed to write aircraft record to '%s'.\n", Fleet_file);
    }
}


// 2. Display All Fleet Records

void display_fleet(void) {
    FILE *fp = fopen(Fleet_file, "rb");
    if (fp == NULL) {
        printf("\n[INFO] No fleet records found. Register an aircraft first.\n");
        return;
    }

    Aircraft ac;
    int count = 0;

    printf("\n=========================================================================================================\n");
    printf("%-14s | %-22s | %-12s | %-8s | %-12s | %-12s | %-10s\n",
           "Tail Number", "Model", "Flight Hrs", "Cycles", "Fatigue (%)", "Hrs Rem.", "Status");
    printf("=========================================================================================================\n");

    while (fread(&ac, sizeof(Aircraft), 1, fp) == 1) {
        printf("%-14s | %-22s | %-12.2lf | %-8d | %-12.2lf | %-12.2lf | %-10s\n",
               ac.tail_number,
               ac.model,
               ac.fhours,
               ac.fcycles,
               ac.fatigue_index,
               ac.hrs_rem,
               ac.is_grnded ? "GROUNDED" : "ACTIVE");
        count++;
    }

    printf("=========================================================================================================\n");
    printf("Total Aircraft in Fleet: %d\n", count);

    fclose(fp);
}


// 3. Search Aircraft by Tail Number

void look_aircraft(const char *tail_number) {
    FILE *fp = fopen(Fleet_file, "rb");
    if (fp == NULL) {
        printf("\n[ERROR] Fleet database '%s' could not be opened.\n", Fleet_file);
        return;
    }

    Aircraft ac;
    int found = 0;

    while (fread(&ac, sizeof(Aircraft), 1, fp) == 1) {
        if (match_tail_number(ac.tail_number, tail_number)) {
            printf("\n--- AIRCRAFT RECORD FOUND ---\n");
            printf("Tail Number         : %s\n", ac.tail_number);
            printf("Model               : %s\n", ac.model);
            printf("Total Flight Hours  : %.2lf hrs\n", ac.fhours);
            printf("Flight Cycles       : %d\n", ac.fcycles);
            printf("Fatigue Index       : %.2lf%%\n", ac.fatigue_index);
            printf("Hours Remaining     : %.2lf hrs\n", ac.hrs_rem);
            printf("Operational Status  : %s\n", ac.is_grnded ? "GROUNDED" : "ACTIVE");
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("\n[NOT FOUND] Aircraft with tail number '%s' does not exist.\n", tail_number);
    }

    fclose(fp);
}


// 4. Update Aircraft Model or Grounded Status In-Place

void update_aircraft(const char *tail_number) {
    FILE *fp = fopen(Fleet_file, "r+b");
    if (fp == NULL) {
        printf("\n[ERROR] Fleet database '%s' could not be opened.\n", Fleet_file);
        return;
    }

    Aircraft ac;
    int found = 0;

    while (fread(&ac, sizeof(Aircraft), 1, fp) == 1) {
        if (match_tail_number(ac.tail_number, tail_number)) {
            found = 1;
            printf("\nUpdating Aircraft: %s (Current Model: %s)\n", ac.tail_number, ac.model);

            printf("Enter New Model (or press ENTER to keep current): ");
            clear_input_buffer();
            char new_model[22];
            fgets(new_model, sizeof(new_model), stdin);
            new_model[strcspn(new_model, "\n")] = '\0';

            if (strlen(new_model) > 0) {
                strcpy(ac.model, new_model);
            }

            printf("Set Operational Status (0 = Active, 1 = Grounded): ");
            while (scanf("%d", &ac.is_grnded) != 1 || (ac.is_grnded != 0 && ac.is_grnded != 1)) {
                printf("Invalid input. Enter 0 for Active or 1 for Grounded: ");
                clear_input_buffer();
            }

            // Rewind file pointer by one struct size and overwrite
            fseek(fp, -((long)sizeof(Aircraft)), SEEK_CUR);
            fwrite(&ac, sizeof(Aircraft), 1, fp);

            printf("\n[SUCCESS] Aircraft '%s' updated successfully!\n", ac.tail_number);
            break;
        }
    }

    if (!found) {
        printf("\n[NOT FOUND] Cannot update. Tail number '%s' not found.\n", tail_number);
    }

    fclose(fp);
}


// 5. Remove Aircraft Record from Database

void remove_aircraft(const char *tail_number) {
    FILE *fp = fopen(Fleet_file, "rb");
    if (fp == NULL) {
        printf("\n[ERROR] Fleet database '%s' could not be opened.\n", Fleet_file);
        return;
    }

    FILE *temp_fp = fopen("temp_fleet.dat", "wb");
    if (temp_fp == NULL) {
        printf("\n[ERROR] Could not create temporary file.\n");
        fclose(fp);
        return;
    }

    Aircraft ac;
    int found = 0;

    while (fread(&ac, sizeof(Aircraft), 1, fp) == 1) {
        if (match_tail_number(ac.tail_number, tail_number)) {
            found = 1; // Skip writing this record to delete it
        } else {
            fwrite(&ac, sizeof(Aircraft), 1, temp_fp);
        }
    }

    fclose(fp);
    fclose(temp_fp);

    if (found) {
        remove(Fleet_file);
        rename("temp_fleet.dat", Fleet_file);
        printf("\n[SUCCESS] Aircraft '%s' removed from fleet database.\n", tail_number);
    } else {
        remove("temp_fleet.dat");
        printf("\n[NOT FOUND] Tail number '%s' not found. No records deleted.\n", tail_number);
    }
}