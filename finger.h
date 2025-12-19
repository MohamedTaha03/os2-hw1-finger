#ifndef FINGER_H
#define FINGER_H

#include <stddef.h> // For size_t
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <utmp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>   // For tolower
#include <strings.h> // For strcasecmp and strcasestr
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_USERS 100

struct passwd; // Forward declaration of struct passwd

typedef struct {
    char element[32]; // Renamed from elemento
    char login_name[32];
    char real_name[64];
    char office_location[32];
    char office_phone[16];
    char home_phone[16];
    char home_directory[128];
    char login_shell[64];
    char terminal[32];
    char idle_time[64];
    char login_time[64];
    char mail_status[256];
    char plan[1024];
    char project[1024];
    char pgpkey[1024];
    int write_status;
 
} UserInfo;

// Function prototypes
void get_user_info(UserInfo *user, int show_plan, int long_format, int match_names);
void print_full_gecos(const struct passwd *pw);
char* format_phone_number(const char *input);
void get_idle_time(const char *tty, char *login_time, char *idle_time, int long_format);
void get_login_time(const char *login_name, char *login_time, int long_format);
void read_user_files(UserInfo *user, int show_plan);
void read_user_file(const char *directory, const char *filename, char *output, size_t output_size);
void get_mail_status(const char *mail_path, char *mail_status);
void read_file_content(const char *file_path, char *buffer, size_t buffer_size);

int check_write_permission(const char *tty);
void print_user_info(UserInfo *user, int long_format, int show_plan);
void parse_command_line(int argc, char *argv[], int *long_format, int *show_plan, int *match_names, char user_list[][32], int *user_count);

// Helper functions (formerly Italian names)
void remove_spaces(char *str); 
char **copy_gecos_list(char *gecos_list[]);

#endif // FINGER_H