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

// --------------------------------------------------------------------------
// 1. Run Full Fleet Airworthiness Compliance Check
// --------------------------------------------------------------------------
void chk_fleet_awrth(void) {
    FILE *fp = fopen(Fleet_file, "rb");
    if (fp == NULL) {
        printf("\n[INFO] Fleet database '%s' is empty or not yet created.\n", Fleet_file);
        return;
    }

    Aircraft ac;
    int total = 0;
    int compliant = 0;
    int warning = 0;
    int grounded = 0;

    printf("\n=========================================================================================================\n");
    printf("                             CIVIL AVIATION AIRWORTHINESS COMPLIANCE AUDIT                               \n");
    printf("=========================================================================================================\n");
    printf("%-14s | %-20s | %-12s | %-12s | %-34s\n",
           "Tail Number", "Model", "Fatigue (%)", "Hrs Rem.", "Compliance Determination");
    printf("---------------------------------------------------------------------------------------------------------\n");

    while (fread(&ac, sizeof(Aircraft), 1, fp) == 1) {
        total++;
        if (ac.is_grnded || ac.fatigue_index >= 85.0 || ac.hrs_rem <= 0.0) {
            printf("%-14s | %-20s | %-12.2lf | %-12.2lf | [NON-COMPLIANT] GROUNDED\n",
                   ac.tail_number, ac.model, ac.fatigue_index, ac.hrs_rem);
            grounded++;
        } else if (ac.fatigue_index >= 75.0 || ac.hrs_rem <= 500.0) {
            printf("%-14s | %-20s | %-12.2lf | %-12.2lf | [CONDITIONAL] Inspection Due Soon\n",
                   ac.tail_number, ac.model, ac.fatigue_index, ac.hrs_rem);
            warning++;
        } else {
            printf("%-14s | %-20s | %-12.2lf | %-12.2lf | [AIRWORTHY] Cleared for Flight\n",
                   ac.tail_number, ac.model, ac.fatigue_index, ac.hrs_rem);
            compliant++;
        }
    }

    printf("=========================================================================================================\n");
    printf("Total Evaluated: %d | Airworthy: %d | Conditional Warning: %d | Grounded: %d\n",
           total, compliant, warning, grounded);

    fclose(fp);
}

// --------------------------------------------------------------------------
// 2. Generate Detailed Grounded Aircraft Safety Report
// --------------------------------------------------------------------------
void gnrt_grnded_aircraft_report(void) {
    FILE *fp = fopen(Fleet_file, "rb");
    if (fp == NULL) {
        printf("\n[INFO] Fleet database '%s' could not be opened.\n", Fleet_file);
        return;
    }

    Aircraft ac;
    int grounded_count = 0;

    printf("\n=========================================================================================================\n");
    printf("                                 GROUNDED AIRCRAFT SAFETY & RISK REPORT                                  \n");
    printf("=========================================================================================================\n");

    while (fread(&ac, sizeof(Aircraft), 1, fp) == 1) {
        if (ac.is_grnded || ac.fatigue_index >= 85.0 || ac.hrs_rem <= 0.0) {
            grounded_count++;
            printf("Aircraft #%d: %s (%s)\n", grounded_count, ac.tail_number, ac.model);
            printf("  - Accumulated Hours / Cycles : %.2lf hrs / %d cycles\n", ac.fhours, ac.fcycles);
            printf("  - Structural Fatigue Index   : %.2lf%% (Limit: 85.00%%)\n", ac.fatigue_index);
            printf("  - Hours Remaining to C-Check : %.2lf hrs\n", ac.hrs_rem);
            printf("  - Grounding Reason(s)        : ");

            if (ac.fatigue_index >= 85.0 && ac.hrs_rem <= 0.0) {
                printf("Structural Fatigue Limit Exceeded AND Mandatory C-Check Overdue\n");
            } else if (ac.fatigue_index >= 85.0) {
                printf("Structural Fatigue Limit Exceeded (>= 85.0%%)\n");
            } else if (ac.hrs_rem <= 0.0) {
                printf("Mandatory C-Check Interval Expired (0.0 hrs remaining)\n");
            } else {
                printf("Administrative / Manual Maintenance Hold\n");
            }
            printf("---------------------------------------------------------------------------------------------------------\n");
        }
    }

    if (grounded_count == 0) {
        printf("No aircraft are currently grounded. 100%% of fleet meets flight safety criteria.\n");
        printf("=========================================================================================================\n");
    } else {
        printf("Total Grounded Airframes Requiring Clearance: %d\n", grounded_count);
        printf("=========================================================================================================\n");
    }

    fclose(fp);
}

// --------------------------------------------------------------------------
// 3. Export Formal Regulatory Compliance Audit to Text File
// --------------------------------------------------------------------------
void export_compliance_audit_file(void) {
    FILE *fleet_fp = fopen(Fleet_file, "rb");
    if (fleet_fp == NULL) {
        printf("\n[ERROR] Cannot export audit: '%s' not found.\n", Fleet_file);
        return;
    }

    FILE *out_fp = fopen(Audit_file, "w");
    if (out_fp == NULL) {
        printf("\n[ERROR] Unable to create audit report file '%s'.\n", Audit_file);
        fclose(fleet_fp);
        return;
    }

    // Generate current timestamp using <time.h>
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(out_fp, "=========================================================================================\n");
    fprintf(out_fp, "               OFFICIAL AIRCRAFT FLEET AIRWORTHINESS COMPLIANCE AUDIT                    \n");
    fprintf(out_fp, "               Generated On: %s                                         \n", timestamp);
    fprintf(out_fp, "=========================================================================================\n\n");
    fprintf(out_fp, "%-14s | %-20s | %-10s | %-8s | %-11s | %-10s | %-12s\n",
            "Tail Number", "Model", "Hours", "Cycles", "Fatigue(%)", "Hrs Rem.", "Status");
    fprintf(out_fp, "-----------------------------------------------------------------------------------------\n");

    Aircraft ac;
    int total_ac = 0;
    int grounded_ac = 0;
    double sum_fatigue = 0.0;

    while (fread(&ac, sizeof(Aircraft), 1, fleet_fp) == 1) {
        total_ac++;
        sum_fatigue += ac.fatigue_index;
        int is_unsafe = (ac.is_grnded || ac.fatigue_index >= 85.0 || ac.hrs_rem <= 0.0);
        if (is_unsafe) {
            grounded_ac++;
        }

        fprintf(out_fp, "%-14s | %-20s | %-10.2lf | %-8d | %-11.2lf | %-10.2lf | %-12s\n",
                ac.tail_number,
                ac.model,
                ac.fhours,
                ac.fcycles,
                ac.fatigue_index,
                ac.hrs_rem,
                is_unsafe ? "GROUNDED" : "AIRWORTHY");
    }
    fclose(fleet_fp);

    // Aggregate maintenance expenditure from Mentos_file
    FILE *mentos_fp = fopen(Mentos_file, "rb");
    int total_orders = 0;
    double total_maint_cost = 0.0;
    if (mentos_fp != NULL) {
        MaintenanceRecord rec;
        while (fread(&rec, sizeof(MaintenanceRecord), 1, mentos_fp) == 1) {
            total_orders++;
            total_maint_cost += rec.cost;
        }
        fclose(mentos_fp);
    }

    fprintf(out_fp, "-----------------------------------------------------------------------------------------\n\n");
    fprintf(out_fp, "EXECUTIVE AUDIT SUMMARY:\n");
    fprintf(out_fp, "  Total Registered Airframes   : %d\n", total_ac);
    fprintf(out_fp, "  Airworthy / Operational      : %d\n", total_ac - grounded_ac);
    fprintf(out_fp, "  Grounded / Non-Compliant     : %d\n", grounded_ac);
    fprintf(out_fp, "  Fleet Average Fatigue Index  : %.2lf%%\n", (total_ac > 0) ? (sum_fatigue / total_ac) : 0.0);
    fprintf(out_fp, "  Logged Maintenance Orders    : %d\n", total_orders);
    fprintf(out_fp, "  Cumulative Maintenance Spend : $%.2lf\n", total_maint_cost);
    fprintf(out_fp, "=========================================================================================\n");

    fclose(out_fp);
    printf("\n[SUCCESS] Formal compliance audit report exported to '%s'!\n", Audit_file);
}

// --------------------------------------------------------------------------
// 4. Print High-Level Combined Fleet & Maintenance Summary Statistics
// --------------------------------------------------------------------------
void print_fleet_summary(void) {
    FILE *fleet_fp = fopen(Fleet_file, "rb");
    if (fleet_fp == NULL) {
        printf("\n[INFO] Fleet database '%s' is empty or not yet created.\n", Fleet_file);
        return;
    }

    Aircraft ac;
    int total_ac = 0;
    int active_ac = 0;
    int grounded_ac = 0;
    double total_hours = 0.0;
    int total_cycles = 0;
    double total_fatigue = 0.0;

    while (fread(&ac, sizeof(Aircraft), 1, fleet_fp) == 1) {
        total_ac++;
        total_hours += ac.fhours;
        total_cycles += ac.fcycles;
        total_fatigue += ac.fatigue_index;

        if (ac.is_grnded || ac.fatigue_index >= 85.0 || ac.hrs_rem <= 0.0) {
            grounded_ac++;
        } else {
            active_ac++;
        }
    }
    fclose(fleet_fp);

    FILE *mentos_fp = fopen(Mentos_file, "rb");
    int work_orders = 0;
    double total_cost = 0.0;
    if (mentos_fp != NULL) {
        MaintenanceRecord rec;
        while (fread(&rec, sizeof(MaintenanceRecord), 1, mentos_fp) == 1) {
            work_orders++;
            total_cost += rec.cost;
        }
        fclose(mentos_fp);
    }

    double readiness_rate = (total_ac > 0) ? ((double)active_ac / total_ac) * 100.0 : 0.0;

    printf("\n================================================================================\n");
    printf("                     EXECUTIVE FLEET & MAINTENANCE SUMMARY                      \n");
    printf("================================================================================\n");
    printf("  Total Fleet Size             : %d aircraft\n", total_ac);
    printf("  Operational / Airworthy      : %d aircraft\n", active_ac);
    printf("  Grounded / Restricted        : %d aircraft\n", grounded_ac);
    printf("  Fleet Operational Readiness  : %.1lf%%\n", readiness_rate);
    printf("--------------------------------------------------------------------------------\n");
    printf("  Cumulative Fleet Flight Hrs  : %.2lf hrs\n", total_hours);
    printf("  Cumulative Pressurizations   : %d cycles\n", total_cycles);
    printf("  Mean Airframe Fatigue Index  : %.2lf%%\n", (total_ac > 0) ? (total_fatigue / total_ac) : 0.0);
    printf("--------------------------------------------------------------------------------\n");
    printf("  Total Maintenance Work Logs  : %d orders\n", work_orders);
    printf("  Total Maintenance Cost (USD) : $%.2lf\n", total_cost);
    printf("  Avg Maintenance Cost/Aircraft: $%.2lf\n", (total_ac > 0) ? (total_cost / total_ac) : 0.0);
    printf("================================================================================\n");
}

// --------------------------------------------------------------------------
// 5. Search Complete Airworthiness & Maintenance Audit History by Tail Number
// --------------------------------------------------------------------------
void look_audit_history(const char *tail_number) {
    FILE *fleet_fp = fopen(Fleet_file, "rb");
    if (fleet_fp == NULL) {
        printf("\n[ERROR] Fleet database '%s' could not be opened.\n", Fleet_file);
        return;
    }

    Aircraft ac;
    int found_ac = 0;

    while (fread(&ac, sizeof(Aircraft), 1, fleet_fp) == 1) {
        if (match_tail_number(ac.tail_number, tail_number)) {
            found_ac = 1;
            break;
        }
    }
    fclose(fleet_fp);

    if (!found_ac) {
        printf("\n[NOT FOUND] Aircraft '%s' is not registered in '%s'.\n", tail_number, Fleet_file);
        return;
    }

    printf("\n=========================================================================================================\n");
    printf("                          COMPLETE AIRWORTHINESS DOSSIER: %-15s                                \n", ac.tail_number);
    printf("=========================================================================================================\n");
    printf("Model               : %s\n", ac.model);
    printf("Total Flight Hours  : %.2lf hrs | Flight Cycles: %d\n", ac.fhours, ac.fcycles);
    printf("Fatigue Index       : %.2lf%%   | Hours to C-Check: %.2lf hrs\n", ac.fatigue_index, ac.hrs_rem);
    printf("Current Status      : %s\n",
           (ac.is_grnded || ac.fatigue_index >= 85.0 || ac.hrs_rem <= 0.0) ? "GROUNDED (NON-COMPLIANT)" : "AIRWORTHY");
    printf("---------------------------------------------------------------------------------------------------------\n");
    printf("MAINTENANCE & INSPECTION LOGS:\n");
    printf("%-6s | %-12s | %-12s | %-12s | %-45s\n",
           "ID", "Date", "Engineer", "Cost ($)", "Service / Inspection Description");
    printf("---------------------------------------------------------------------------------------------------------\n");

    FILE *mentos_fp = fopen(Mentos_file, "rb");
    int log_count = 0;
    double airframe_cost = 0.0;

    if (mentos_fp != NULL) {
        MaintenanceRecord rec;
        while (fread(&rec, sizeof(MaintenanceRecord), 1, mentos_fp) == 1) {
            if (match_tail_number(rec.tail_number, tail_number)) {
                printf("%-6d | %-12s | %-12s | %-12.2lf | %-45s\n",
                       (int)rec.wrk_id, rec.date, rec.engner_id, rec.cost, rec.descrpt);
                log_count++;
                airframe_cost += rec.cost;
            }
        }
        fclose(mentos_fp);
    }

    if (log_count == 0) {
        printf("No maintenance work orders logged for this airframe yet.\n");
    }

    printf("=========================================================================================================\n");
    printf("Total Maintenance Events: %d | Lifetime Airframe Maintenance Spend: $%.2lf\n",
           log_count, airframe_cost);
}