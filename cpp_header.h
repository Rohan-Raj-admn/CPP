#ifndef cpp_header
#define cpp_header

#include <stdio.h>
#include <stdib.h>  //memory allocation utility.
#include <string.h>
#include <time.h>   //for time
#include <ctype.h>  //to manipulate uprcase and lwrcase.
#include <math.h>

#include Fleet_file "fleet.dat"
#include Mentos_file "mentos.dat"
#include Audit_file "awrth_audit.txt"

// To flush data into the database. for fleet.dat
typedef struct {
    char tail_number[26];
    char model[22];
    double fhours;
    int fcycles;
    double fatigue_index;
    double hrs_rem;
    int is_grnded;
} Aircraft;

//To flush data into the database. for mentos.dat
typedef struct {
    char wrk_id;
    char tail_number[26];
    char date[11];
    char engner_id[10]; //It could be mix of number and char, ig foermat of id.
    char descrpt[120];
    double cost;
} MaintenanceRecord;


//Module starts here.

//module 1
void add_aircraft(void);
void display_fleet(void);
void look_aircraft(const char *tail_number);
void update_aircraft(const char *tail_number);
void remove_aircraft(const char *tail_number);

//module 2
void log_flight(cont char *tail_num, double *fhours);
void clcult_struct_ft(Aircraft *ac);
void chk_inspt_intervals(void);
void reset_mentos_counters(const char *tail_num);
void display_ft_analylitcs(void);

//module 3
void create_wrk_ord(void);     //appends mentos file.
void display_mentos_hsty(const char *tail_number);
void clcult_fleet_mentos_cst(void);
void update_wrk_ord(char wrk_id);
void dlt_wrk_ord(char wrk_id);

//module 4
void chk_fleet_awrth(void);
void gnrt_grnded_aircraft_report(void);
void export_compliance_audit_file(void);
void print_fleet_summary(void);
void look_audit_history(cont char *tail_number);

#endif