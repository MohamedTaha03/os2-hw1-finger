#include "finger.h"

// Custom implementation of strcasestr if not available
// This function searches for the string "needle" inside "haystack" ignoring case differences
#ifndef _GNU_SOURCE
char *strcasestr(const char *haystack, const char *needle) {
    if (!*needle) return (char *) haystack; // If needle is empty, return haystack
    for ( ; *haystack; ++haystack) {
        if (tolower((unsigned char)*haystack) == tolower((unsigned char)*needle)) {
            const char *h, *n;
            for (h = haystack, n = needle; *h && *n; ++h, ++n) {
                if (tolower((unsigned char)*h) != tolower((unsigned char)*n)) break;
            }
            if (!*n) return (char *) haystack;
        }
    }
    return NULL;
}
#endif

// Function to get user information
void get_user_info(UserInfo *user, int show_plan, int long_format, int match_names) {
    struct passwd *pw = getpwnam(user->login_name); // Get user info from login name
    if (pw == NULL) { // If user is not found
        if (!match_names) {
            fprintf(stderr, "User %s not found\n", user->login_name); // Print error message
            exit(EXIT_FAILURE); // Terminate program
        } else {
            // Iterate over all users and try to match the real name
            setpwent(); // Reset passwd file to beginning
            while ((pw = getpwent()) != NULL) {
                if (strcasestr(pw->pw_gecos, user->login_name) != NULL) { // Search login name in GECOS field
                    // Copy login name from pw structure
                    strncpy(user->login_name, pw->pw_name, sizeof(user->login_name) - 1);
                    user->login_name[sizeof(user->login_name) - 1] = '\0';
                    break;
                }
            }
            endpwent(); // Close passwd file

            if (pw == NULL) {
                fprintf(stderr, "User %s not found\n", user->login_name); // Print error if user not found
                exit(EXIT_FAILURE); // Terminate program
            }
        }
    }

    // Parse GECOS field
    char *gecos = strdup(pw->pw_gecos); // Duplicate GECOS field
    char *tokens[4] = {NULL, NULL, NULL, NULL};
    int i = 0;

    tokens[i++] = strtok(gecos, ","); // Split GECOS field into tokens separated by commas
    while ((tokens[i++] = strtok(NULL, ",")) != NULL && i < 4);

    // Populate user info from GECOS field and passwd structure
    strncpy(user->real_name, tokens[0] ? tokens[0] : "", sizeof(user->real_name) - 1);
    user->real_name[sizeof(user->real_name) - 1] = '\0';

    strncpy(user->office_location, tokens[1] ? tokens[1] : "", sizeof(user->office_location) - 1);
    user->office_location[sizeof(user->office_location) - 1] = '\0';

    strncpy(user->office_phone, tokens[2] ? format_phone_number(tokens[2]) : "", sizeof(user->office_phone) - 1);
    user->office_phone[sizeof(user->office_phone) - 1] = '\0';

    strncpy(user->home_phone, tokens[3] ? format_phone_number(tokens[3]) : "", sizeof(user->home_phone) - 1);
    user->home_phone[sizeof(user->home_phone) - 1] = '\0';

    free(gecos); // Free memory allocated for duplicated GECOS field

    // Populate remaining user info from passwd structure
    strncpy(user->home_directory, pw->pw_dir, sizeof(user->home_directory) - 1);
    user->home_directory[sizeof(user->home_directory) - 1] = '\0';

    strncpy(user->login_shell, pw->pw_shell, sizeof(user->login_shell) - 1);
    user->login_shell[sizeof(user->login_shell) - 1] = '\0';

    // Get terminal, idle time, and login time
    get_login_time(user->login_name, user->login_time, long_format);
    get_idle_time(user->terminal, user->login_time, user->idle_time, long_format);

    // Read additional user files like .plan, .project, and .pgpkey
    read_user_files(user, show_plan);

    // Mail status
    char mail_path[256];
    snprintf(mail_path, sizeof(mail_path), "/var/mail/%s", user->login_name); // Create path to user's mail file
    get_mail_status(mail_path, user->mail_status);

    // Get terminal and write status
    struct utmp *ut;
    setutent(); // Reset utmp file to beginning
    while ((ut = getutent()) != NULL) {
        if (ut->ut_type == USER_PROCESS && strncmp(ut->ut_user, user->login_name, sizeof(ut->ut_user)) == 0) { // Search for user in utmp
            strncpy(user->terminal, ut->ut_line, sizeof(user->terminal) - 1);
            user->terminal[sizeof(user->terminal) - 1] = '\0';
            user->write_status = check_write_permission(user->terminal); // Check write permissions for terminal
            break;
        }
    }
    endutent(); // Close utmp file
}

// Function to format a phone number
char* format_phone_number(const char *input) {
    static char output[16]; // Static buffer for formatted output
    size_t len = strlen(input); // Length of input string
    if (len == 11) {
        snprintf(output, sizeof(output), "+%c-%3.3s-%3.3s-%4.4s", input[0], &input[1], &input[4], &input[7]);
    } else if (len == 10) {
        snprintf(output, sizeof(output), "%3.3s-%3.3s-%4.4s", input, &input[3], &input[6]);
    } else if (len == 7) {
        snprintf(output, sizeof(output), "%3.3s-%4.4s", input, &input[3]);
    } else if (len == 5) {
        snprintf(output, sizeof(output), "x%c-%4.4s", input[0], &input[1]);
    } else if (len == 4) {
        snprintf(output, sizeof(output), "x%4.4s", input);
    } else {
        snprintf(output, sizeof(output), "Invalid"); // If length matches no known format, return "Invalid"
    }
    return output; // Return formatted phone number
}

// Function to get idle time
void get_idle_time(const char *tty, char *login_time, char *idle_time, int long_format) {
    if (strcmp(tty, "*") == 0 || strcmp(login_time, "*") == 0) { // If terminal or login time are invalid
        snprintf(idle_time, 2, "*"); // Set idle time to "*"
        return;
    }

    char tty_path[256];
    snprintf(tty_path, sizeof(tty_path), "/dev/%s", tty); // Create path to terminal file

    struct stat statbuf;
    if (stat(tty_path, &statbuf) == -1) { // Get info on terminal file
        snprintf(idle_time, 2, "*"); // If failed, set idle time to "*"
        return;
    }

    time_t now = time(NULL); // Get current time
    time_t last_access = statbuf.st_atime; // Get last access time of terminal file

    int idle_seconds = (int)difftime(now, last_access); // Calculate idle time in seconds
    int minutes = idle_seconds / 60; // Convert seconds to minutes
    int hours = minutes / 60; // Convert minutes to hours
    int seconds = idle_seconds % 60; // Get remaining seconds
    minutes %= 60; // Get remaining minutes

    if (long_format) { // If long format is enabled
        if (hours > 0) {
            snprintf(idle_time, 64, "%d hours %d minutes %d seconds idle", hours, minutes, seconds); // Format idle time as hours, minutes, seconds
        } else if (minutes > 0) {
            snprintf(idle_time, 64, "%d minutes %d seconds idle", minutes, seconds); // Format idle time as minutes, seconds
        } else {
            snprintf(idle_time, 64, "%d seconds idle", seconds); // Format idle time as seconds
        }
    } else { // If short format is enabled
        int total_minutes = (idle_seconds + 59) / 60; // Round up to minutes
        if (total_minutes >= 60) {
            int hours = total_minutes / 60;
            int minutes = total_minutes % 60;
            snprintf(idle_time, 16, "%d:%02d", hours, minutes); // Format idle time as HH:MM
        } else {
            snprintf(idle_time, 16, "%d", total_minutes); // Format idle time as minutes
        }
    }
}

// Function to get login time
void get_login_time(const char *login_name, char *login_time, int long_format) {
    struct utmp *ut;
    setutent(); // Reset utmp file to beginning
    while ((ut = getutent()) != NULL) {
        if (ut->ut_type == USER_PROCESS && strncmp(ut->ut_user, login_name, sizeof(ut->ut_user)) == 0) { // Search user in utmp
            time_t login_timestamp = ut->ut_tv.tv_sec; // Get login timestamp
            struct tm *login_tm = localtime(&login_timestamp); // Convert timestamp to tm struct

            if (long_format) { // If long format is enabled
                strftime(login_time, 64, "%A, %d %B %Y %H:%M:%S", login_tm); // Format login time as readable string
            } else { // If short format is enabled
                strftime(login_time, 64, "%b %d %H:%M", login_tm); // Format login time as short string
            }
            break;
        }
    }
    endutent(); // Close utmp file

    if (ut == NULL) { // If user not found
        snprintf(login_time, 2, "*"); // Set login time to "*"
    }
}

// Definition of read_file_content function
void read_file_content(const char *file_path, char *buffer, size_t buffer_size) {
    FILE *file = fopen(file_path, "r");
    if (file) {
        fread(buffer, 1, buffer_size - 1, file);
        buffer[buffer_size - 1] = '\0'; // Ensure null termination
        fclose(file);
    }
}

// Function to read user files like .plan, .project, and .pgpkey
void read_user_files(UserInfo *user, int show_plan) {
    if (show_plan) {
        char file_path[256];
        snprintf(file_path, sizeof(file_path), "%s/.plan", user->home_directory); // Create path for .plan
        read_file_content(file_path, user->plan, sizeof(user->plan)); // Read .plan content

        snprintf(file_path, sizeof(file_path), "%s/.project", user->home_directory); // Create path for .project
        read_file_content(file_path, user->project, sizeof(user->project)); // Read .project content

        snprintf(file_path, sizeof(file_path), "%s/.pgpkey", user->home_directory); // Create path for .pgpkey
        read_file_content(file_path, user->pgpkey, sizeof(user->pgpkey)); // Read .pgpkey content
    }
}

// Function to get mail status
void get_mail_status(const char *mail_path, char *mail_status) {
    struct stat mail_stat;
    if (stat(mail_path, &mail_stat) == -1) { // Get info on mail file
        snprintf(mail_status, 16, "No Mail"); // If file doesn't exist, set status to "No Mail"
    } else {
        if (mail_stat.st_size == 0) { // If file exists but is empty
            snprintf(mail_status, 16, "No Mail"); // Set status to "No Mail"
        } else {
            time_t last_change = mail_stat.st_mtime; // Get last modification time
            struct tm *last_change_tm = localtime(&last_change); // Convert to tm struct
            strftime(mail_status, 64, "Mail last read %b %d %H:%M", last_change_tm); // Format last read time
        }
    }
}

int check_write_permission(const char *tty) {
    char tty_path[256];
    snprintf(tty_path, sizeof(tty_path), "/dev/%s", tty);
    
    // Check if writing to terminal is possible
    if (access(tty_path, W_OK) == 0) {
        return 1; // Access allowed
    } else {
        return 0; // Access denied
    }
}

void print_user_info(UserInfo *user, int long_format, int show_plan) {
        if (long_format) {
        // Print login and name info
        printf("Login: %-30s Name: %s\n", user->login_name, user->real_name);

        // Print directory and shell
        printf("Directory: %-25s Shell: %s\n", user->home_directory, user->login_shell);

        // Print office and home phone
        printf("Office: %-28s Office Phone: %-15s Home Phone: %s\n", user->office_location, user->office_phone, user->home_phone);

         if (strcmp(user->login_time,"*")==0){
            printf("Never logged in.\n");

         }else{

         // Print login time, terminal, and idle time
        printf("On since %s on %s from \n", user->login_time, user->terminal);
        printf("   %s idle\n", user->idle_time);}

        // Print mail status
        if (strcmp(user->mail_status, "No mail.") == 0) {
            
        } else {
            printf("Mail: %s\n", user->mail_status);
        }

        // Print plan and project if requested
        if (show_plan) {
            if (strcmp(user->plan, "") == 0) {
                printf("No Plan.\n");
            } else {
                printf("Plan: %s\n", user->plan);
            }

            if (strcmp(user->project, "") == 0) {
                printf("No Project.\n");
            } else {
                printf("Project: %s\n", user->project);
            }

            if (strcmp(user->pgpkey, "") == 0) {
                printf("No PGP Key.\n");
            } else {
                printf("PGP Key: %s\n", user->pgpkey);
            }
        }
        printf("\n");
    }
    else {
        // Print headers
        printf("%-10s %-15s %-15s %-15s %-15s %-15s %-10s\n",
               "Login", "Name", "Idle Time",
               "Login Time", "Office", "Office Phone", "Tty");

        // Print user information
        printf("%-10s %-15s %-15s %-15s %-15s %-15s %-1s%s\n",
               user->login_name, user->real_name, user->idle_time,
               user->login_time, user->office_location, user->office_phone,
               user->terminal, (user->write_status==0 && strcmp(user->login_time,"*")==0)? "*" : "");
               
    }
}

void parse_command_line(int argc, char *argv[], int *long_format, int *show_plan, int *match_names, char user_list[][32], int *user_count) {
    int opt;
    while ((opt = getopt(argc, argv, "lpsm")) != -1) {
        switch (opt) {
            case 'l':
                *long_format = 1;
                break;
            case 'p':
                *long_format = 1;
                *show_plan = 0;
                break;
            case 's':
                *long_format = 0;
                break;
            case 'm':
                *match_names = 0;
                break;
            default:
                fprintf(stderr, "Usage: %s [user ...] [-lpsm]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    for (int i = optind; i < argc; i++) {
        strncpy(user_list[*user_count], argv[i], 31);
        user_list[*user_count][31] = '\0'; // Ensure null-termination
        (*user_count)++;
    }
}

void handle_user_info(UserInfo *user, int show_plan, int long_format, int match_names, char *processed_users[MAX_USERS]) {
    // Gets user info and stores it in the user structure
    get_user_info(user, show_plan, long_format, match_names);
    
    // Iterates through the list of processed users to avoid duplicates
    for (int i = 0; i < MAX_USERS; i++) {
        if (processed_users[i] == NULL) {
            // If user hasn't been processed yet, store login_name and print info
            processed_users[i] = strdup(user->login_name);
            print_user_info(user, long_format, show_plan);
            break;
        } else if (strcmp(processed_users[i], user->login_name) == 0) {
            // If user was already processed, exit function
            return;
        }
    }
}

void remove_spaces(char *str) {
    // Removes spaces from a string
    int index = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] != ' ') {
            str[index++] = str[i];
        }
    }
    str[index] = '\0'; // Terminate new string without spaces
}

char **copy_gecos_list(char *gecos_list[]) {
    // Copies a list of pointers dynamically
    int num_users = 0;
    while (gecos_list[num_users] != NULL) {
        num_users++;
    }

    // Allocate new list of pointers
    char **new_list = malloc(num_users * sizeof(char *));
    if (new_list == NULL) {
        perror("Error allocating memory for new_list");
        exit(EXIT_FAILURE);
    }

    // Copy values from gecos_list to new_list
    for (int i = 0; i < num_users; i++) {
        new_list[i] = malloc(strlen(gecos_list[i]) + 1);
        if (new_list[i] == NULL) {
            perror("Error allocating memory for new_list[i]");
            exit(EXIT_FAILURE);
        }
        strcpy(new_list[i], gecos_list[i]);
    }

    return new_list;
}

int main(int argc, char *argv[]) {
    // Variables to handle flags and user list
    int long_format = 1;
    int show_plan = 1;
    int match_names = 1;
    char user_list[MAX_USERS][32];
    int user_count = 0;
    char *processed_users[MAX_USERS] = {NULL}; // Initialize all to NULL

    // Parse command line arguments
    parse_command_line(argc, argv, &long_format, &show_plan, &match_names, user_list, &user_count);

    if (user_count == 0) {
        // If no users specified, list all active users
        struct utmp *ut;
        setutent();
        while ((ut = getutent()) != NULL) {
            if (ut->ut_type == USER_PROCESS) {
                UserInfo user = {0};
                strncpy(user.login_name, ut->ut_user, sizeof(user.login_name) - 1);
                user.login_name[sizeof(user.login_name) - 1] = '\0'; // Ensure null-termination
                strncpy(user.terminal, ut->ut_line, sizeof(user.terminal) - 1);
                user.terminal[sizeof(user.terminal) - 1] = '\0'; // Ensure null-termination
                handle_user_info(&user, show_plan, long_format, match_names, processed_users);
                break;
            }
        }
        endutent();
    } else {
        struct passwd *pw;
        char *gecos_list[MAX_USERS]; // Array of pointers to store pw->pw_gecos values
        char *gecos_users[MAX_USERS]; // Array of pointers to store usernames
        int count = 0; // Counter for number of stored entries
        int num_users = 0; // Counter for number of stored users

        // Initialize arrays to NULL
        for (int i = 0; i < MAX_USERS; i++) {
            gecos_list[i] = NULL;
            gecos_users[i] = NULL;
        }

        // Populate gecos_list array with GECOS values
        setpwent();
        while ((pw = getpwent()) != NULL && num_users < MAX_USERS) {
            char pw_gecos_copy[256];
            strncpy(pw_gecos_copy, pw->pw_gecos, sizeof(pw_gecos_copy) - 1);
            pw_gecos_copy[sizeof(pw_gecos_copy) - 1] = '\0';

            char *first_element = strtok(pw_gecos_copy, ",");
            if (first_element) {
                char *gecos_copy = strdup(first_element);
                if (gecos_copy == NULL) {
                    exit(EXIT_FAILURE);
                }
                gecos_list[count] = gecos_copy;
                count++;
            }
        }
        endpwent();

        // Populate gecos_users array with usernames
        setpwent();
        while ((pw = getpwent()) != NULL && num_users < MAX_USERS) {
            char *username_copy = strdup(pw->pw_name);
            if (username_copy == NULL) {
                perror("strdup");
                exit(EXIT_FAILURE);
            }
            gecos_users[num_users] = username_copy;
            num_users++;
        }
        endpwent();

        // Process user_list with name matching, including login names
        for (int i = 0; i < user_count; i++) {
            int user_found2 = 0; 

            char *first_name = user_list[i];
            for (int i = 0; i < num_users; i++) {
                if (strcasecmp(gecos_users[i], first_name) == 0) {
                    UserInfo user = {0};
                    strncpy(user.login_name, gecos_users[i], sizeof(user.login_name) - 1);
                    user.login_name[sizeof(user.login_name) - 1] = '\0';
                    handle_user_info(&user, show_plan, long_format, match_names, processed_users);
                    user_found2 = 1;
                }
            }

            int user_found = 0;
            if (match_names) {
                char name[32];
                char **new_list = copy_gecos_list(gecos_list);

                for (int j = 0; j < count; j++) {
                    const char delim[] = "-";
                    char *token;
                    char element[32];
                    strcpy(element, gecos_list[j]);

                    token = strtok(element, delim);

                    while (token != NULL) {
                        strcpy(name, token);
                        token = strtok(NULL, "-");

                        remove_spaces(first_name);

                        char *token2 = strtok(name, " ");

                        while (token2 != NULL) {
                            if (strcasecmp(first_name, token2) == 0) {
                                token2 = strtok(NULL, " ");
                                UserInfo user = {0};
                                strcpy(user.login_name, new_list[j]);
                                user.login_name[sizeof(user.login_name) - 1] = '\0';
                                handle_user_info(&user, show_plan, long_format, match_names, processed_users);
                                user_found = 1;
                            } else {
                                token2 = strtok(NULL, "");
                            }
                        }
                    }
                }
            }

            if (!user_found && !user_found2) {
                user_found2 = 0;
                printf("User not found: %s\n", user_list[i]);
            }

            for (int j = 0; j < count; j++) {
                free(gecos_list[j]);
            }
        }
    }

    
    // Free memory allocated for processed user names
    for (int i = 0; i < MAX_USERS; i++) {
        if (processed_users[i] != NULL) {
            free(processed_users[i]);
        }
    }

    return 0;
}