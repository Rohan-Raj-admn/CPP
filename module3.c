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

// Verify whether the aircraft exists in fleet.dat before logging maintenance
static int aircraft_exists(const char *tail_num) {
    FILE *fp = fopen(Fleet_file, "rb");
    if (fp == NULL) {
        return 0;
    }

    Aircraft ac;
    int found = 0;
    while (fread(&ac, sizeof(Aircraft), 1, fp) == 1) {
        if (match_tail_number(ac.tail_number, tail_num)) {
            found = 1;
            break;
        }
    }

    fclose(fp);
    return found;
}

// Auto-generate next sequential Work Order ID (1 to 127 to fit char wrk_id)
static char generate_next_wrk_id(void) {
    FILE *fp = fopen(Mentos_file, "rb");
    if (fp == NULL) {
        return 1;
    }

    MaintenanceRecord rec;
    int max_id = 0;
    while (fread(&rec, sizeof(MaintenanceRecord), 1, fp) == 1) {
        if ((unsigned char)rec.wrk_id > max_id) {
            max_id = (unsigned char)rec.wrk_id;
        }
    }

    fclose(fp);
    if (max_id >= 127) {
        return -1; // Maximum capacity for signed char ID reached
    }
    return (char)(max_id + 1);
}


// 1. Create & Append Maintenance Work Order Record

void create_wrk_ord(void) {
    MaintenanceRecord rec;
    memset(&rec, 0, sizeof(MaintenanceRecord));

    printf("\n--- CREATE MAINTENANCE WORK ORDER ---\n");

    printf("Enter Aircraft Tail Number (e.g., VT-A320): ");
    clear_input_buffer();
    fgets(rec.tail_number, sizeof(rec.tail_number), stdin);
    rec.tail_number[strcspn(rec.tail_number, "\n")] = '\0';

    for (int i = 0; rec.tail_number[i] != '\0'; i++) {
        rec.tail_number[i] = (char)toupper((unsigned char)rec.tail_number[i]);
    }

    if (!aircraft_exists(rec.tail_number)) {
        printf("\n[ERROR] Aircraft '%s' not found in '%s'. Register it in Module 1 first.\n",
               rec.tail_number, Fleet_file);
        return;
    }

    rec.wrk_id = generate_next_wrk_id();
    if (rec.wrk_id <= 0) {
        printf("\n[ERROR] Maximum work order ID limit (127) reached for char ID type.\n");
        return;
    }

    // Date input with automatic system date fallback using <time.h>
    printf("Enter Date (YYYY-MM-DD) or press ENTER for today's date: ");
    char date_input[16];
    fgets(date_input, sizeof(date_input), stdin);
    date_input[strcspn(date_input, "\n")] = '\0';

    if (strlen(date_input) == 0) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        strftime(rec.date, sizeof(rec.date), "%Y-%m-%d", tm_info);
    } else {
        strncpy(rec.date, date_input, sizeof(rec.date) - 1);
        rec.date[sizeof(rec.date) - 1] = '\0';
    }

    printf("Enter Lead Engineer ID (e.g., ENG-104): ");
    fgets(rec.engner_id, sizeof(rec.engner_id), stdin);
    rec.engner_id[strcspn(rec.engner_id, "\n")] = '\0';
    for (int i = 0; rec.engner_id[i] != '\0'; i++) {
        rec.engner_id[i] = (char)toupper((unsigned char)rec.engner_id[i]);
    }

    printf("Enter Maintenance / Repair Description: ");
    fgets(rec.descrpt, sizeof(rec.descrpt), stdin);
    rec.descrpt[strcspn(rec.descrpt, "\n")] = '\0';

    printf("Enter Total Repair Cost (USD): ");
    while (scanf("%lf", &rec.cost) != 1 || rec.cost < 0.0) {
        printf("Invalid input. Enter a non-negative cost: ");
        clear_input_buffer();
    }

    FILE *fp = fopen(Mentos_file, "ab");
    if (fp == NULL) {
        printf("\n[ERROR] Unable to open '%s'.\n", Mentos_file);
        return;
    }

    size_t written = fwrite(&rec, sizeof(MaintenanceRecord), 1, fp);
    fclose(fp);

    if (written == 1) {
        printf("\n[SUCCESS] Work Order #%d logged for '%s' on %s!\n",
               (int)rec.wrk_id, rec.tail_number, rec.date);
    } else {
        printf("\n[ERROR] Failed to save work order to '%s'.\n", Mentos_file);
    }
}


// 2. Display Maintenance History for a Specific Aircraft

void display_mentos_hsty(const char *tail_number) {
    FILE *fp = fopen(Mentos_file, "rb");
    if (fp == NULL) {
        printf("\n[INFO] No maintenance records found ('%s' not yet created).\n", Mentos_file);
        return;
    }

    MaintenanceRecord rec;
    int count = 0;
    double aircraft_total_cost = 0.0;

    printf("\n=========================================================================================================\n");
    printf("                         MAINTENANCE HISTORY FOR AIRCRAFT: %-15s                              \n", tail_number);
    printf("=========================================================================================================\n");
    printf("%-6s | %-12s | %-12s | %-12s | %-45s\n",
           "ID", "Date", "Engineer ID", "Cost ($)", "Work Description");
    printf("---------------------------------------------------------------------------------------------------------\n");

    while (fread(&rec, sizeof(MaintenanceRecord), 1, fp) == 1) {
        if (match_tail_number(rec.tail_number, tail_number)) {
            printf("%-6d | %-12s | %-12s | %-12.2lf | %-45s\n",
                   (int)rec.wrk_id,
                   rec.date,
                   rec.engner_id,
                   rec.cost,
                   rec.descrpt);
            aircraft_total_cost += rec.cost;
            count++;
        }
    }

    printf("=========================================================================================================\n");
    if (count == 0) {
        printf("No work orders recorded for tail number '%s'.\n", tail_number);
    } else {
        printf("Total Work Orders: %d | Cumulative Maintenance Spend: $%.2lf\n",
               count, aircraft_total_cost);
    }

    fclose(fp);
}


// 3. Calculate Total Fleet Maintenance Costs & Analytics

void clcult_fleet_mentos_cst(void) {
    FILE *fp = fopen(Mentos_file, "rb");
    if (fp == NULL) {
        printf("\n[INFO] Maintenance database '%s' is empty or unavailable.\n", Mentos_file);
        return;
    }

    MaintenanceRecord rec;
    int total_orders = 0;
    double total_cost = 0.0;
    double max_cost = -1.0;
    int max_id = 0;
    char max_tail[26] = "N/A";

    while (fread(&rec, sizeof(MaintenanceRecord), 1, fp) == 1) {
        total_orders++;
        total_cost += rec.cost;

        if (rec.cost > max_cost) {
            max_cost = rec.cost;
            max_id = (int)rec.wrk_id;
            strcpy(max_tail, rec.tail_number);
        }
    }

    fclose(fp);

    printf("\n================================================================================\n");
    printf("                    FLEET MAINTENANCE FINANCIAL SUMMARY                         \n");
    printf("================================================================================\n");
    if (total_orders == 0) {
        printf("No maintenance records available to calculate costs.\n");
    } else {
        printf("Total Work Orders Logged     : %d\n", total_orders);
        printf("Total Fleet Maintenance Cost : $%.2lf\n", total_cost);
        printf("Average Cost per Work Order  : $%.2lf\n", total_cost / total_orders);
        printf("Highest Single Repair Cost   : $%.2lf (Work Order #%d - %s)\n",
               max_cost, max_id, max_tail);
    }
    printf("================================================================================\n");
}


// 4. Update Existing Maintenance Work Order In-Place

void update_wrk_ord(char wrk_id) {
    FILE *fp = fopen(Mentos_file, "r+b");
    if (fp == NULL) {
        printf("\n[ERROR] Maintenance database '%s' could not be opened.\n", Mentos_file);
        return;
    }

    MaintenanceRecord rec;
    int found = 0;

    while (fread(&rec, sizeof(MaintenanceRecord), 1, fp) == 1) {
        if (rec.wrk_id == wrk_id) {
            found = 1;
            printf("\nUpdating Work Order #%d (Aircraft: %s | Date: %s)\n",
                   (int)rec.wrk_id, rec.tail_number, rec.date);
            printf("Current Engineer ID : %s\n", rec.engner_id);
            printf("Current Description : %s\n", rec.descrpt);
            printf("Current Cost        : $%.2lf\n\n", rec.cost);

            clear_input_buffer();

            printf("Enter New Engineer ID (or press ENTER to keep '%s'): ", rec.engner_id);
            char new_eng[10];
            fgets(new_eng, sizeof(new_eng), stdin);
            new_eng[strcspn(new_eng, "\n")] = '\0';
            if (strlen(new_eng) > 0) {
                for (int i = 0; new_eng[i] != '\0'; i++) {
                    new_eng[i] = (char)toupper((unsigned char)new_eng[i]);
                }
                strcpy(rec.engner_id, new_eng);
            }

            printf("Enter New Description (or press ENTER to keep current): ");
            char new_desc[120];
            fgets(new_desc, sizeof(new_desc), stdin);
            new_desc[strcspn(new_desc, "\n")] = '\0';
            if (strlen(new_desc) > 0) {
                strcpy(rec.descrpt, new_desc);
            }

            printf("Enter New Cost in USD (or enter -1 to keep $%.2lf): ", rec.cost);
            double new_cost = -1.0;
            if (scanf("%lf", &new_cost) == 1 && new_cost >= 0.0) {
                rec.cost = new_cost;
            }

            // Rewind file pointer by one struct size and overwrite updated record
            fseek(fp, -((long)sizeof(MaintenanceRecord)), SEEK_CUR);
            fwrite(&rec, sizeof(MaintenanceRecord), 1, fp);

            printf("\n[SUCCESS] Work Order #%d updated successfully!\n", (int)rec.wrk_id);
            break;
        }
    }

    if (!found) {
        printf("\n[NOT FOUND] Work Order #%d does not exist.\n", (int)wrk_id);
    }

    fclose(fp);
}


// 5. Delete Maintenance Work Order Record

void dlt_wrk_ord(char wrk_id) {
    FILE *fp = fopen(Mentos_file, "rb");
    if (fp == NULL) {
        printf("\n[ERROR] Maintenance database '%s' could not be opened.\n", Mentos_file);
        return;
    }

    FILE *temp_fp = fopen("temp_mentos.dat", "wb");
    if (temp_fp == NULL) {
        printf("\n[ERROR] Unable to create temporary file.\n");
        fclose(fp);
        return;
    }

    MaintenanceRecord rec;
    int found = 0;

    while (fread(&rec, sizeof(MaintenanceRecord), 1, fp) == 1) {
        if (rec.wrk_id == wrk_id) {
            found = 1; // Skip writing this record to delete it
        } else {
            fwrite(&rec, sizeof(MaintenanceRecord), 1, temp_fp);
        }
    }

    fclose(fp);
    fclose(temp_fp);

    if (found) {
        remove(Mentos_file);
        rename("temp_mentos.dat", Mentos_file);
        printf("\n[SUCCESS] Work Order #%d deleted from '%s'.\n", (int)wrk_id, Mentos_file);
    } else {
        remove("temp_mentos.dat");
        printf("\n[NOT FOUND] Work Order #%d not found. No records deleted.\n", (int)wrk_id);
    }
}