# Custom `finger` Implementation in C

This project is a custom implementation of the classic Unix `finger` utility, written in C. It retrieves and displays detailed information about system users by querying standard Unix data sources.

## Features

- **User Lookup**: Find users by their login name or, by default, perform a case-insensitive search on their real name (GECOS field).
- **Multiple Output Formats**:
  - **Long Format (-l)**: A comprehensive, multi-line report for each user.
  - **Short Format (-s)**: A compact, single-line tabular view.
- **Data Sources**: The program aggregates information from multiple system files:
  - `/etc/passwd`: For basic user details (login, real name, office, phones, home directory, shell).
  - `/var/run/utmp`: For active session details (login time, terminal, idle time).
  - `/var/mail/<username>`: To check mail status.
  - `~/.plan`, `~/.project`, `~/.pgpkey`: To display user-provided information (can be disabled).
- **Idle Time Calculation**: Estimates user idle time by checking the last access time of their terminal device (`/dev/<tty>`).
- **Phone Number Formatting**: Automatically formats phone numbers from the GECOS field into a readable, standardized layout.
- **Graceful Fallbacks**: Displays placeholder information (e.g., "*") when data (like login time) is unavailable.

## Build Instructions

This program is designed for a POSIX-compliant environment (e.g., Linux) and requires a C compiler like `gcc`.

### Recommended Build (Optimized)

This command compiles the program with optimizations and all recommended warnings enabled. This is the suggested command for general use.

```sh
gcc -D_GNU_SOURCE -Wall -Wextra -O2 -o finger finger.c
```

**Command Breakdown:**
- `gcc`: The compiler to use.
- `-D_GNU_SOURCE`: This flag is crucial. It enables non-standard, GNU-specific extensions, including `strcasestr`, which is used for case-insensitive name matching. Without this, the build may fail on some systems.
- `-Wall -Wextra`: Enable all standard and extra warnings. This helps catch potential bugs and stylistic issues in the code.
- `-O2`: Apply level-2 optimizations, which improves the performance of the final executable.
- `-o finger`: Specifies that the output executable file should be named `finger`.
- `finger.c`: The input source file to compile.

### Debug Build

If you need to debug the program (e.g., using `gdb`), compile it with debugging symbols and without optimization. This makes it much easier to inspect variables and trace the program's execution.

```sh
gcc -D_GNU_SOURCE -Wall -Wextra -g -o finger_debug finger.c
```

**Command Breakdown:**
- `-g`: Includes debugging information (like function names, line numbers, and variable names) in the executable.
- `-o finger_debug`: It's good practice to give the debug executable a different name to avoid confusion with the optimized version.

## Usage

The program can be run in two main modes:

1.  **Query Specific Users**: Provide one or more login names or real names as arguments.
2.  **List Logged-in Users**: Run without arguments to get a report on all currently active users.

### Command-Line Syntax

```
finger [options] [user1] [user2] ...
```

### Options

-   `-l` (Long Format): Displays a detailed, multi-line report. This is the **default** format.
-   `-s` (Short Format): Displays a compact, single-line table. Overrides `-l`.
-   `-p` (No Plan): Use long format but **suppress** the display of `~/.plan`, `~/.project`, and `~/.pgpkey` files. This is useful for a slightly less verbose report.
-   `-m` (Match Exact): Disables the default behavior of matching against real names (GECOS field). When this flag is active, arguments are **only** treated as login names.

### Examples

- **Get default (long format) info for a user**:
  ```sh
  ./finger root
  ```

- **Get short format info for multiple users**:
  ```sh
  ./finger -s root daemon
  ```

- **Search for a user by their real name (e.g., "John Doe")**:
  ```sh
  ./finger "John Doe"
  ```

- **Get long format info without the .plan, .project, or .pgpkey files**:
  ```sh
  ./finger -p username
  ```

- **Force a search by login name only (disables real name matching)**:
  ```sh
  ./finger -m username
  ```

- **List all currently logged-in users**:
  ```sh
  ./finger
  ```

## Detailed Information Displayed

### Long Format (`-l`)

-   **Login**: The user's login name.
-   **Name**: The user's full real name.
-   **Directory**: The user's home directory path.
-   **Shell**: The user's default login shell.
-   **Office**: Office location (from GECOS).
-   **Office Phone**: Office phone number (from GECOS, formatted).
-   **Home Phone**: Home phone number (from GECOS, formatted).
-   **On since**: Login timestamp and terminal device.
-   **Idle**: Estimated idle time.
-   **Mail**: Mail status (e.g., "No Mail" or "Mail last read ...").
-   **Plan, Project, PGP Key**: Contents of `~/.plan`, `~/.project`, and `~/.pgpkey` respectively (unless `-p` is used).

### Short Format (`-s`)

-   **Login**: Login name.
-   **Name**: Real name.
-   **Idle Time**: Idle time in a compact format (minutes or HH:MM).
-   **Login Time**: Compact login timestamp.
-   **Office**: Office location.
-   **Office Phone**: Formatted office phone number.
-   **Tty**: Terminal name. An asterisk (`*`) appears next to the terminal if it is not writable.

## Implementation Details

-   **User Info Storage**: A `UserInfo` struct (`finger.h`) holds all retrieved data for a user before printing.
-   **GECOS Parsing**: The GECOS field from `/etc/passwd` is parsed by splitting on commas to extract the real name, office location, and phone numbers.
-   **Login/Idle Time**: The `utmp` file is scanned for `USER_PROCESS` entries to find active sessions. Idle time is calculated by `stat`-ing the terminal device file in `/dev` and comparing its last access time (`st_atime`) to the current time.
-   **File Reading**: User-specific files (`.plan`, etc.) are read from their home directory. The program handles cases where these files do not exist or are not readable.
-   **Duplicate Prevention**: When listing all logged-in users, the program keeps track of processed users to avoid printing duplicate entries for users with multiple sessions.
-   **Error Handling**: The program provides informative error messages if a user is not found or if system files cannot be accessed.

## Code Structure

-   `finger.c`: Contains the main logic, including command-line parsing, user lookup, data retrieval functions, and printing routines.
-   `finger.h`: Defines the `UserInfo` struct, function prototypes, and necessary header includes.

## License / Usage
This code is provided for educational purposes in the Operating Systems 2 course. Ensure compliance with your institution's policies if reusing or modifying the code.