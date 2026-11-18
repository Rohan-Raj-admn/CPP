#include "cpp_header.h"

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


// 1. Calculate Structural Fatigue & Evaluate Grounding Thresholds

void clcult_struct_ft(Aircraft *ac) {
    if (ac == NULL) {
        return;
    }

    // Weighted fatigue formula: 60% flight hours (12,000 hr ref) + 40% cycles (8,000 cycle ref)
    double hour_factor = (ac->fhours / 12000.0) * 60.0;
    double cycle_factor = (ac->fcycles / 8000.0) * 40.0;
    ac->fatigue_index = hour_factor + cycle_factor;

    if (ac->fatigue_index > 100.0) {
        ac->fatigue_index = 100.0;
    }

    // Automatically ground aircraft if fatigue reaches 85% or C-check hours expire
    if (ac->fatigue_index >= 85.0 || ac->hrs_rem <= 0.0) {
        ac->is_grnded = 1;
    }
}


// 2. Log a Flight & Recalculate Aircraft Fatigue In-Place

void log_flight(const char *tail_num, double *fhours) {
    if (tail_num == NULL || fhours == NULL || *fhours <= 0.0) {
        printf("\n[ERROR] Invalid tail number or flight duration.\n");
        return;
    }

    FILE *fp = fopen(Fleet_file, "r+b");
    if (fp == NULL) {
        printf("\n[ERROR] Fleet database '%s' could not be opened.\n", Fleet_file);
        return;
    }

    Aircraft ac;
    int found = 0;

    while (fread(&ac, sizeof(Aircraft), 1, fp) == 1) {
        if (match_tail_number(ac.tail_number, tail_num)) {
            found = 1;

            if (ac.is_grnded) {
                printf("\n[WARNING] Aircraft '%s' is currently GROUNDED and restricted from flight!\n", ac.tail_number);
                fclose(fp);
                return;
            }

            // Update flight hours, pressurization cycles, and C-check countdown
            ac.fhours += *fhours;
            ac.fcycles += 1;
            ac.hrs_rem -= *fhours;
            if (ac.hrs_rem < 0.0) {
                ac.hrs_rem = 0.0;
            }

            // Recalculate structural fatigue and check safety limits
            clcult_struct_ft(&ac);

            // Rewind file pointer by one struct size and overwrite updated record
            fseek(fp, -((long)sizeof(Aircraft)), SEEK_CUR);
            fwrite(&ac, sizeof(Aircraft), 1, fp);

            printf("\n--- FLIGHT LOGGED & FATIGUE UPDATED ---\n");
            printf("Aircraft Tail Number  : %s (%s)\n", ac.tail_number, ac.model);
            printf("Flight Duration Added : +%.2lf hrs\n", *fhours);
            printf("Total Flight Hours    : %.2lf hrs\n", ac.fhours);
            printf("Total Flight Cycles   : %d cycles (+1 cycle)\n", ac.fcycles);
            printf("Updated Fatigue Index : %.2lf%%\n", ac.fatigue_index);
            printf("Hours to Next C-Check : %.2lf hrs\n", ac.hrs_rem);
            printf("Airworthiness Status  : %s\n", ac.is_grnded ? "GROUNDED (Limit Exceeded!)" : "ACTIVE");
            break;
        }
    }

    if (!found) {
        printf("\n[NOT FOUND] Aircraft '%s' does not exist in '%s'.\n", tail_num, Fleet_file);
    }

    fclose(fp);
}


// 3. Scan Entire Fleet & Enforce Inspection / Fatigue Intervals

void chk_inspt_intervals(void) {
    FILE *fp = fopen(Fleet_file, "r+b");
    if (fp == NULL) {
        printf("\n[INFO] Fleet database '%s' is empty or unavailable.\n", Fleet_file);
        return;
    }

    Aircraft ac;
    int total_scanned = 0;
    int grounded_count = 0;
    int warning_count = 0;

    printf("\n=========================================================================================\n");
    printf("                    FLEET INSPECTION & FATIGUE INTERVAL SCAN                             \n");
    printf("=========================================================================================\n");
    printf("%-14s | %-12s | %-12s | %-38s\n",
           "Tail Number", "Fatigue (%)", "Hrs Rem.", "Inspection Evaluation");
    printf("-----------------------------------------------------------------------------------------\n");

    while (fread(&ac, sizeof(Aircraft), 1, fp) == 1) {
        total_scanned++;
        int prev_status = ac.is_grnded;
        clcult_struct_ft(&ac);

        if (ac.is_grnded != prev_status) {
            fseek(fp, -((long)sizeof(Aircraft)), SEEK_CUR);
            fwrite(&ac, sizeof(Aircraft), 1, fp);
            fseek(fp, 0, SEEK_CUR); // Ensure stream state is valid before next fread
        }

        if (ac.fatigue_index >= 85.0) {
            printf("%-14s | %-12.2lf | %-12.2lf | [GROUNDED] Fatigue limit (>= 85%%) exceeded\n",
                   ac.tail_number, ac.fatigue_index, ac.hrs_rem);
            grounded_count++;
        } else if (ac.hrs_rem <= 0.0) {
            printf("%-14s | %-12.2lf | %-12.2lf | [GROUNDED] Mandatory C-Check overdue\n",
                   ac.tail_number, ac.fatigue_index, ac.hrs_rem);
            grounded_count++;
        } else if (ac.hrs_rem <= 500.0 || ac.fatigue_index >= 75.0) {
            printf("%-14s | %-12.2lf | %-12.2lf | [WARNING] Approaching maintenance threshold\n",
                   ac.tail_number, ac.fatigue_index, ac.hrs_rem);
            warning_count++;
        } else {
            printf("%-14s | %-12.2lf | %-12.2lf | [OK] Within safe operational envelope\n",
                   ac.tail_number, ac.fatigue_index, ac.hrs_rem);
        }
    }

    printf("=========================================================================================\n");
    printf("Scanned: %d | Grounded: %d | Warnings: %d\n", total_scanned, grounded_count, warning_count);

    fclose(fp);
}


// 4. Reset Maintenance Counters After C-Check Overhaul

void reset_mentos_counters(const char *tail_num) {
    FILE *fp = fopen(Fleet_file, "r+b");
    if (fp == NULL) {
        printf("\n[ERROR] Fleet database '%s' could not be opened.\n", Fleet_file);
        return;
    }

    Aircraft ac;
    int found = 0;

    while (fread(&ac, sizeof(Aircraft), 1, fp) == 1) {
        if (match_tail_number(ac.tail_number, tail_num)) {
            found = 1;
            ac.hrs_rem = 6000.0; // Standard 6,000-hour C-Check interval reset
            clcult_struct_ft(&ac);

            if (ac.fatigue_index < 85.0) {
                ac.is_grnded = 0;
                printf("\n[SUCCESS] C-Check completed for '%s'. Hours remaining reset to 6000.00 hrs.\n", ac.tail_number);
                printf("Status restored to: ACTIVE\n");
            } else {
                ac.is_grnded = 1;
                printf("\n[ALERT] C-Check hours reset to 6000.00 hrs for '%s', but structural fatigue (%.2lf%%) exceeds 85%%.\n",
                       ac.tail_number, ac.fatigue_index);
                printf("Aircraft remains GROUNDED pending structural airframe overhaul.\n");
            }

            fseek(fp, -((long)sizeof(Aircraft)), SEEK_CUR);
            fwrite(&ac, sizeof(Aircraft), 1, fp);
            break;
        }
    }

    if (!found) {
        printf("\n[NOT FOUND] Aircraft '%s' not found in fleet database.\n", tail_num);
    }

    fclose(fp);
}


// 5. Display Fleet-Wide Fatigue Analytics Dashboard

void display_ft_analylitcs(void) {
    FILE *fp = fopen(Fleet_file, "rb");
    if (fp == NULL) {
        printf("\n[INFO] Fleet database '%s' is empty or not yet created.\n", Fleet_file);
        return;
    }

    Aircraft ac;
    int count = 0;
    double total_hours = 0.0;
    int total_cycles = 0;
    double sum_fatigue = 0.0;
    double max_fatigue = -1.0;
    char highest_tail[26] = "N/A";

    printf("\n=========================================================================================\n");
    printf("                         STRUCTURAL FATIGUE ANALYTICS DASHBOARD                          \n");
    printf("=========================================================================================\n");
    printf("%-14s | %-12s | %-8s | %-10s | %-24s\n",
           "Tail Number", "Flight Hrs", "Cycles", "Fatigue", "Fatigue Load Bar");
    printf("-----------------------------------------------------------------------------------------\n");

    while (fread(&ac, sizeof(Aircraft), 1, fp) == 1) {
        count++;
        total_hours += ac.fhours;
        total_cycles += ac.fcycles;
        sum_fatigue += ac.fatigue_index;

        if (ac.fatigue_index > max_fatigue) {
            max_fatigue = ac.fatigue_index;
            strcpy(highest_tail, ac.tail_number);
        }

        // Build 20-segment visual progress bar
        int filled = (int)(ac.fatigue_index / 5.0);
        if (filled > 20) filled = 20;
        if (filled < 0) filled = 0;

        char bar[23];
        bar[0] = '[';
        for (int i = 0; i < 20; i++) {
            bar[i + 1] = (i < filled) ? '#' : '-';
        }
        bar[21] = ']';
        bar[22] = '\0';

        printf("%-14s | %-12.2lf | %-8d | %6.2lf%%    | %s\n",
               ac.tail_number, ac.fhours, ac.fcycles, ac.fatigue_index, bar);
    }

    printf("=========================================================================================\n");
    if (count > 0) {
        printf("Total Fleet Flight Hours   : %.2lf hrs\n", total_hours);
        printf("Total Fleet Cycles         : %d cycles\n", total_cycles);
        printf("Average Fleet Fatigue      : %.2lf%%\n", sum_fatigue / count);
        printf("Highest Fatigue Airframe   : %s (%.2lf%%)\n", highest_tail, max_fatigue);
    } else {
        printf("No aircraft records available for analytics.\n");
    }
    printf("=========================================================================================\n");

    fclose(fp);
}