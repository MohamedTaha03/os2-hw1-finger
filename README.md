<div align="center">

# 👆 Finger - Unix User Information Tool

[![C](https://img.shields.io/badge/Language-C-blue.svg?logo=c)](https://en.wikipedia.org/wiki/C_(programming_language))
[![POSIX](https://img.shields.io/badge/Platform-POSIX%2FLinux-green.svg?logo=linux)](https://en.wikipedia.org/wiki/POSIX)
[![License](https://img.shields.io/badge/License-Educational-orange.svg)](.)
[![OS2](https://img.shields.io/badge/Course-Operating%20Systems%202-purple.svg)](.)

*A modern, feature-rich implementation of the classic Unix `finger` command*

</div>

---

## 📖 Overview

This project is a custom implementation of the classic Unix `finger` utility, written entirely in **C**. It retrieves and displays detailed information about system users by querying standard Unix data sources like `/etc/passwd`, `utmp`, and user home directories.

> 💡 **Fun Fact**: The original `finger` command dates back to 1971 and was one of the first social networking tools, allowing users to find information about each other on shared systems!

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| 🔍 **User Lookup** | Find users by login name or real name (case-insensitive GECOS search) |
| 📋 **Long Format** | Comprehensive multi-line report with all user details |
| 📊 **Short Format** | Compact single-line tabular view |
| ⏰ **Idle Time** | Real-time calculation from terminal device access times |
| 📞 **Phone Formatting** | Smart formatting for various phone number lengths |
| 📬 **Mail Status** | Check user's mail status from `/var/mail` |
| 📝 **Plan/Project** | Display `.plan`, `.project`, and `.pgpkey` files |

### Data Sources

```
📁 System Files Used
├── /etc/passwd        → User details (name, shell, home dir)
├── /var/run/utmp      → Active sessions (login time, terminal)
├── /var/mail/<user>   → Mail status
└── ~/<user>/
    ├── .plan          → User's plan file
    ├── .project       → Current project info
    └── .pgpkey        → PGP public key
```

---

## 🛠️ Build Instructions

### Prerequisites

- **OS**: Linux / POSIX-compliant system
- **Compiler**: GCC (or any C99-compatible compiler)

### 🚀 Quick Build (Recommended)

```bash
gcc -D_GNU_SOURCE -Wall -Wextra -O2 -o finger finger.c
```

### 🔧 Build Options Explained

| Flag | Purpose |
|------|---------|
| `-D_GNU_SOURCE` | Enables GNU extensions (required for `strcasestr`) |
| `-Wall -Wextra` | Enable comprehensive warnings |
| `-O2` | Level 2 optimization for better performance |
| `-o finger` | Output executable name |

### 🐛 Debug Build

```bash
gcc -D_GNU_SOURCE -Wall -Wextra -g -o finger_debug finger.c
```

Use this build with `gdb` for step-by-step debugging.

---

## 🎯 Usage

### Syntax

```
./finger [options] [user1] [user2] ...
```

### Command-Line Options

| Option | Name | Description |
|--------|------|-------------|
| `-l` | Long Format | Detailed multi-line output **(default)** |
| `-s` | Short Format | Compact single-line table |
| `-p` | No Plan | Long format without `.plan`/`.project`/`.pgpkey` |
| `-m` | Match Exact | Match login names only (disable GECOS search) |

### 📝 Examples

```bash
# Get detailed info for a user
./finger root

# Short format for multiple users
./finger -s root daemon

# Search by real name
./finger "John Doe"

# Long format without plan files
./finger -p username

# Exact login name match only
./finger -m username

# List all logged-in users
./finger
```

---

## 📊 Output Formats

### Long Format (`-l`) - Default

```
Login: john                            Name: John Doe
Directory: /home/john                  Shell: /bin/bash
Office: Room 123                       Office Phone: 555-1234       Home Phone: 555-5678
On since Monday, 21 December 2024 10:30:00 on pts/0 from
   2 hours 15 minutes 30 seconds idle
Mail: Mail last read Dec 21 09:00
Plan: Working on OS2 homework
Project: Finger Implementation
```

### Short Format (`-s`)

```
Login      Name            Idle Time       Login Time      Office          Office Phone    Tty
john       John Doe        2:15            Dec 21 10:30    Room 123        555-1234        pts/0
```

---

## 🏗️ Project Structure

```
os2-hw1-finger/
├── 📄 finger.c      # Main source code (540+ lines)
│   ├── get_user_info()      # Fetch user data from system
│   ├── get_idle_time()      # Calculate terminal idle time
│   ├── get_login_time()     # Parse login timestamps
│   ├── get_mail_status()    # Check mail file status
│   ├── read_user_files()    # Read .plan, .project, .pgpkey
│   ├── format_phone_number() # Smart phone formatting
│   ├── print_user_info()    # Output formatting (long/short)
│   ├── parse_command_line() # CLI argument parsing
│   └── main()               # Entry point & orchestration
│
├── 📄 finger.h      # Header file
│   ├── UserInfo struct      # Data container for user info
│   ├── Function prototypes  # All function declarations
│   └── Required includes    # System headers
│
└── 📄 README.md     # This file
```

---

## 🔬 Implementation Highlights

### 📌 Key Algorithms

1. **GECOS Parsing**: Splits the comma-separated GECOS field to extract real name, office, and phone numbers

2. **Idle Time Calculation**: 
   ```c
   idle_time = current_time - stat("/dev/tty").st_atime
   ```

3. **Phone Number Formatting**:
   | Input Length | Output Format |
   |--------------|---------------|
   | 11 digits | `+X-XXX-XXX-XXXX` |
   | 10 digits | `XXX-XXX-XXXX` |
   | 7 digits | `XXX-XXXX` |
   | 4-5 digits | `xX-XXXX` or `xXXXX` |

4. **Duplicate Prevention**: Tracks processed users to avoid duplicate entries for users with multiple sessions

### 🛡️ Error Handling

- ✅ User not found → Informative error message
- ✅ Missing files → Graceful fallback with `*` placeholder
- ✅ Permission denied → Handled silently
- ✅ Memory allocation → Proper error reporting

---

## 📚 Technical Details

| Aspect | Implementation |
|--------|----------------|
| **Language** | C (C99 standard) |
| **Max Users** | 100 (configurable via `MAX_USERS`) |
| **Buffer Sizes** | Login: 32, Name: 64, Path: 256, Plan: 1024 |
| **System Calls** | `getpwnam`, `getpwent`, `setutent`, `stat`, `access` |

---

## 📜 License

> ⚠️ **Educational Use Only**
> 
> This code is provided for educational purposes as part of the **Operating Systems 2** course (Homework 1). Ensure compliance with your institution's academic integrity policies before reusing or modifying this code.

---

<div align="center">

**Made with ❤️ for OS2 Course**

*Matricola: 2086047*

</div>