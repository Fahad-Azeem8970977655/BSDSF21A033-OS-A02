Feature 1: Simple Listing

Concepts Covered: Directory Traversal, opendir(), readdir(), File Filtering

Description:
This feature implements the basic ls functionality, listing files in a directory. Hidden files (starting with .) are skipped.

Implementation Notes:
Used opendir() and readdir() to read directory contents, and printed each filename using printf(). Default listing is horizontal (simple file names) without extra information. Example logic: open directory, loop through entries, and print filenames skipping hidden files.
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Feature 2: Long Listing (-l)

Concepts Covered: File Metadata, stat()/lstat(), File Permissions, Owner/Group Info

Description:
The -l option implements a long listing similar to standard ls -l, displaying file type (directory, regular file, symbolic link), permissions (read/write/execute for user/group/others), number of links, owner and group names, file size, and last modification time.

Key Points:

stat() vs. lstat(): stat() returns info about the target file of a symlink, while lstat() returns info about the symlink itself. We use lstat() to correctly display symbolic links.

Extracting file type and permissions: st_mode contains both file type and permission bits. File type is extracted using bitwise AND with masks like S_IFDIR, S_IFREG, S_IFLNK. Permissions are extracted with S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, etc. For example, (fileStat.st_mode & S_IFMT) == S_IFDIR checks if a file is a directory, and (fileStat.st_mode & S_IRUSR) checks if the owner has read permission.

Owner and group names are obtained via getpwuid(st.st_uid) and getgrgid(st.st_gid).

Modification time is formatted using strftime().
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Feature 3: Column Display (Down Then Across)

Concepts Covered: Output Formatting, Terminal I/O (ioctl), Dynamic Memory

Description:
Upgrades the default output to display files in multiple columns “down then across,” automatically adjusting to terminal width and filename lengths.

Implementation Notes:
All filenames are read into a dynamically allocated array and the length of the longest filename is tracked. Terminal width is determined using ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) with a fallback of 80 columns. Number of columns is calculated as terminal_width divided by (max_filename_length + spacing), and number of rows as ceil(total_files / num_columns). Files are printed row by row: for row r and column c, the index is r + c * num_rows. Memory is freed after printing.
Report Questions:

A single loop is insufficient because we want vertical alignment first; we cannot print filenames immediately without calculating rows and columns.

ioctl() detects terminal width for dynamic column adjustment. Using a fixed width would make output inconsistent on different terminal sizes.                                                           --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Feature 4: Horizontal Column Display (-x)

Concepts Covered: Output Formatting Logic, Command-Line Argument Parsing, State Management

Description:
Adds a horizontal display mode triggered by -x. Files are printed from left to right and wrap to the next line when the current line is full.

Implementation Notes:
The getopt() loop is extended to recognize the -x flag. Display mode flags are used: long_flag for -l, horizontal_flag for -x, and default for vertical “down then across”. Terminal width is determined using ioctl(). Column width is calculated based on the longest filename plus spacing. While iterating filenames, each is printed padded to column width. A horizontal position counter tracks the current position; if adding the next filename exceeds terminal width, a newline is printed and the counter resets.

Report Questions:

Vertical “down then across” requires pre-calculation of rows and indexing, making it more complex. Horizontal display only tracks horizontal position, making it simpler.
Display modes are managed using flags set by command-line arguments. After reading filenames, the program checks flags and calls the appropriate function: long listing for -l, horizontal display for -x, or vertical column display for default.
