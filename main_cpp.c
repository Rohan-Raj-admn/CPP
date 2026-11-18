#include "project_data.h"

// Local function prototypes for menu navigation
static void clear_input_buffer(void);
static void menu_module1(void);
static void menu_module2(void);
static void menu_module3(void);
static void menu_module4(void);

int main(void) {
    int main_choice = 0;

    do {
        printf("\n================================================================================\n");
        printf("             AIRCRAFT FLEET AIRWORTHINESS & MAINTENANCE SYSTEM                  \n");
        printf("================================================================================\n");
        printf("  1. Fleet Inventory Management\n");
        printf("  2. Flight Logging & Fatigue Tracker\n");
        printf("  3. Maintenance Operations & Work Logs\n");
        printf("  4. Airworthiness Compliance Audit\n");
        printf("  5. Exit Application\n");
        printf("================================================================================\n");
        printf("Enter choice (1-5): ");

        if (scanf("%d", &main_choice) != 1) {
            printf("\n[ERROR] Invalid entry! Please input a number between 1 and 5.\n");
            clear_input_buffer();
            continue;
        }

        switch (main_choice) {
            case 1:
                menu_module1();
                break;
            case 2:
                menu_module2();
                break;
            case 3:
                menu_module3();
                break;
            case 4:
                menu_module4();
                break;
            case 5:
                printf("\nExiting Aircraft Maintenance System. Flight logs saved successfully!\n\n");
                break;
            default:
                printf("\n[ERROR] Invalid option selected! Please choose between 1 and 5.\n");
        }

    } while (main_choice != 5);

    return 0;
}


// Helper Function: Safely Flushes Standard Input Buffer

static void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}


// Sub-Menu: Module 1 (Fleet Inventory Management)

static void menu_module1(void) {
    int choice = 0;
    char tail_number[12];

    do {
        printf("\n--------------------------------------------------------------------------------\n");
        printf("                  MODULE 1: FLEET INVENTORY MANAGEMENT                          \n");
        printf("--------------------------------------------------------------------------------\n");
        printf("  1. Register New Aircraft\n");
        printf("  2. Display Active Fleet Table\n");
        printf("  3. Search Aircraft by Tail Number\n");
        printf("  4. Update Aircraft Details\n");
        printf("  5. Remove Aircraft from Database\n");
        printf("  6. Back to Main Menu\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("Enter choice (1-6): ");

        if (scanf("%d", &choice) != 1) {
            printf("\n[ERROR] Invalid input.\n");
            clear_input_buffer();
            continue;
        }

        switch (choice) {
            case 1:
                add_aircraft();
                break;
            case 2:
                display_fleet();
                break;
            case 3:
                printf("\nEnter Tail Number to search (e.g., VT-A320): ");
                scanf("%s", tail_number);
                look_aircraft(tail_number);
                break;
            case 4:
                printf("\nEnter Tail Number to update: ");
                scanf("%s", tail_buffer);
                update_aircraft(tail_number);
                break;
            case 5:
                printf("\nEnter Tail Number to remove: ");
                scanf("%s", tail_buffer);
                remove_aircraft(tail_number);
                break;
            case 6:
                printf("\nReturning to Main Menu...\n");
                break;
            default:
                printf("\n[ERROR] Invalid choice. Select between 1 and 6.\n");
        }
    } while (choice != 6);
}


// Sub-Menu: Module 2 (Flight Logging & Fatigue Tracker)

static void menu_module2(void) {
    int choice = 0;
    char tail_number[12];
    float hours_buffer = 0.0f;

    do {
        printf("\n--------------------------------------------------------------------------------\n");
        printf("              MODULE 2: FLIGHT LOGGING & FATIGUE TRACKER                       \n");
        printf("--------------------------------------------------------------------------------\n");
        printf("  1. Log a Flight & Update Fatigue\n");
        printf("  2. Run Fleet Inspection Interval Check\n");
        printf("  3. Reset Maintenance Counter (Post C-Check Overhaul)\n");
        printf("  4. Display Fatigue Analytics Dashboard\n");
        printf("  5. Back to Main Menu\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("Enter choice (1-5): ");

        if (scanf("%d", &choice) != 1) {
            printf("\n[ERROR] Invalid input.\n");
            clear_input_buffer();
            continue;
        }

        switch (choice) {
            case 1:
                printf("\nEnter Aircraft Tail Number: ");
                scanf("%s", tail_number);
                printf("Enter Flight Duration (hours): ");
                while (scanf("%f", &fhours) != 1 || fhours <= 0) {
                    printf("Invalid input. Enter positive flight duration: ");
                    clear_input_buffer();
                }
                log_flight(tail_number, hours_buffer);
                break;
            case 2:
                chk_inspt_intervals();
                break;
            case 3:
                printf("\nEnter Tail Number completing C-Check: ");
                scanf("%s", tail_number);
                reset_mentos_counters(tail_number);
                break;
            case 4:
                display_ft_analylitcs();
                break;
            case 5:
                printf("\nReturning to Main Menu...\n");
                break;
            default:
                printf("\n[ERROR] Invalid choice. Select between 1 and 5.\n");
        }
    } while (choice != 5);
}


// Sub-Menu: Module 3 (Maintenance Operations & Work Logs)

static void menu_module3(void) {
    int choice = 0;
    int wrk_id = 0;
    char tail_number[12];

    do {
        printf("\n--------------------------------------------------------------------------------\n");
        printf("             MODULE 3: MAINTENANCE OPERATIONS & WORK LOGS                       \n");
        printf("--------------------------------------------------------------------------------\n");
        printf("  1. Create Maintenance Work Order\n");
        printf("  2. View Maintenance History by Aircraft\n");
        printf("  3. Calculate Fleet Maintenance Costs\n");
        printf("  4. Update Work Order Record\n");
        printf("  5. Delete Work Order Record\n");
        printf("  6. Back to Main Menu\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("Enter choice (1-6): ");

        if (scanf("%d", &choice) != 1) {
            printf("\n[ERROR] Invalid input.\n");
            clear_input_buffer();
            continue;
        }

        switch (choice) {
            case 1:
                create_wrk_ord();
                break;
            case 2:
                printf("\nEnter Tail Number: ");
                scanf("%s", tail_buffer);
                display_mentos_hsty(tail_number);
                break;
            case 3:
                clcult_fleet_mentos_cst();
                break;
            case 4:
                printf("\nEnter Work Order ID to update: ");
                scanf("%d", &wrk_id);
                update_wrk_ord(wrk_id);
                break;
            case 5:
                printf("\nEnter Work Order ID to delete: ");
                scanf("%d", &wrk_id);
                dlt_wrk_ord(wrk_id);
                break;
            case 6:
                printf("\nReturning to Main Menu...\n");
                break;
            default:
                printf("\n[ERROR] Invalid choice. Select between 1 and 6.\n");
        }
    } while (choice != 6);
}


// Sub-Menu: Module 4 (Airworthiness Compliance Audit)

static void menu_module4(void) {
    int choice = 0;
    char tail_number[12];

    do {
        printf("\n--------------------------------------------------------------------------------\n");
        printf("              MODULE 4: AIRWORTHINESS COMPLIANCE AUDIT                         \n");
        printf("--------------------------------------------------------------------------------\n");
        printf("  1. Run Full Airworthiness Check\n");
        printf("  2. Display Grounded Aircraft Safety Report\n");
        printf("  3. Export Formal Audit Report to File\n");
        printf("  4. View High-Level Fleet Statistics\n");
        printf("  5. Search Aircraft Audit History\n");
        printf("  6. Back to Main Menu\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("Enter choice (1-6): ");

        if (scanf("%d", &choice) != 1) {
            printf("\n[ERROR] Invalid input.\n");
            clear_input_buffer();
            continue;
        }

        switch (choice) {
            case 1:
                chk_fleet_awrth();
                break;
            case 2:
                gnrt_grnded_aircraft_report();
                break;
            case 3:
                export_compliance_audit_file();
                break;
            case 4:
                print_fleet_summary();
                break;
            case 5:
                printf("\nEnter Tail Number to search audit history: ");
                scanf("%s", tail_number);
                look_audit_history(tail_number);
                break;
            case 6:
                printf("\nReturning to Main Menu...\n");
                break;
            default:
                printf("\n[ERROR] Invalid choice. Select between 1 and 6.\n");
        }
    } while (choice != 6);
}