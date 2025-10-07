Feature 1: Simple Listing

Concepts Covered: Directory Traversal, opendir(), readdir(), File Filtering

Description:
The basic ls functionality lists files in a directory, skipping hidden files. The program opens a directory using opendir(), loops through each entry with readdir(), and prints filenames using printf(). Hidden files (starting with .) are ignored. Example logic: for each directory entry, if the first character is not ., print the filename. --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Feature 2: Long Listing (-l)

Concepts Covered: File Metadata, stat()/lstat(), File Permissions, Owner/Group Info

Description:
The -l option prints a long listing of files, similar to ls -l. The displayed information includes file type (directory, regular file, or symbolic link), permissions (read/write/execute for user, group, others), number of links, owner name, group name, file size, and last modification time.

Implementation Details:
lstat() is used instead of stat() because it provides information about symbolic links themselves rather than their targets. File type is determined by (st.st_mode & S_IFMT) and checked against S_IFDIR, S_IFREG, S_ISLNK. Permissions are extracted using bitwise AND with S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH. Example: if (st.st_mode & S_IRUSR) is true, the owner has read permission. Owner and group names are retrieved via getpwuid(st.st_uid) and getgrgid(st.st_gid). Modification time is formatted using strftime().

Sample code inline:
For file type detection: char type = '-'; if (S_ISDIR(st.st_mode)) type = 'd'; else if (S_ISLNK(st.st_mode)) type = 'l';. For permissions: perms[0] = (st.st_mode & S_IRUSR) ? 'r' : '-'; similarly for other permission bits. Printing uses: printf("%c%s %ld %s %s %ld %s %s\n", type, perms, st.st_nlink, pw->pw_name, gr->gr_name, st.st_size, timebuf, filename);.

-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Feature 3: Column Display (Down Then Across)

Concepts Covered: Output Formatting, Terminal I/O (ioctl), Dynamic Memory

Description:
Files are displayed in multiple columns with a “down then across” layout, adjusting automatically to terminal width and filename length. All filenames are first read into a dynamically allocated array while tracking the length of the longest filename. Terminal width is determined using ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) with a fallback of 80 columns. Number of columns is calculated as terminal_width / (max_len + spacing) and number of rows as ceil(total_files / num_columns). Files are printed row by row: for row r and column c, the index is r + c * num_rows. Memory is freed after printing.

Report Questions Answered:

A single loop is insufficient because we need vertical alignment; we cannot print filenames sequentially without pre-calculating rows and columns.

ioctl() detects terminal width dynamically, so the output adapts to different terminal sizes. Using a fixed width (e.g., 80) could make output misaligned on larger or smaller terminals.

Sample code inline:
Reading filenames: filenames[count] = strdup(entry->d_name); if (strlen(entry->d_name) > max_len) max_len = strlen(entry->d_name); count++;. Printing: for each row r and column c, index = r + c*num_rows; print with printf("%-*s", max_len + spacing, filenames[index]);.                                                          --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Feature 4: Horizontal Column Display (-x)

Concepts Covered: Output Formatting Logic, Command-Line Argument Parsing, State Management Description:
The -x option prints files in a horizontal (row-major) layout, wrapping to the next line when the current line is full. Argument parsing is extended using getopt() to recognize -x. Display mode flags are used: long_flag for -l, horizontal_flag for -x, and default vertical for “down then across”. Terminal width is detected using ioctl(). Column width is based on the longest filename plus spacing. A horizontal position counter tracks the current line; when adding the next filename would exceed terminal width, a newline is printed and the counter resets.

Report Questions Answered:

Vertical “down then across” is more complex because it requires pre-calculation of rows and careful indexing. Horizontal display only tracks horizontal position, so it is simpler.

Display modes are managed using flags set from command-line arguments. After reading filenames, the program checks flags and calls the corresponding function: long_listing() for -l, horizontal_display() for -x, or column_display() for default.

Sample code inline:
For horizontal display: int pos = 0; for (int i=0; i<count; i++) { printf("%-*s", max_len+spacing, filenames[i]); pos += max_len+spacing; if(pos + max_len + spacing > term_width){ printf("\n");        pos=0; } } printf("\n");.

-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Feature 5: Alphabetical Sort, the REPORT.md answers are as follows:

Reading all directory entries into memory before sorting is necessary because sorting requires knowledge of all the items you want to order. You cannot determine the correct alphabetical order if you only read and process files one by one. By storing all filenames in a dynamic array, the program can compare each filename with others and rearrange them accordingly. The potential drawback of this approach is that it can consume a large amount of memory for directories containing millions of files, possibly causing memory exhaustion or slowing down the system. Processing extremely large directories may also lead to longer execution times since sorting algorithms like qsort operate on the entire dataset at once.

he comparison function required by qsort() serves as the rule for ordering elements. Its signature is int cmp(const void *a, const void *b), where both parameters are pointers to elements in the array being sorted. Inside the function, these pointers are cast to the appropriate type (in this case, pointers to strings) and compared using a standard comparison method like strcmp(). The function returns a negative, zero, or positive integer depending on whether the first element is less than, equal to, or greater than the second. It must take const void * arguments because qsort() is a generic sorting function in C that can sort arrays of any type. Using const void * ensures type-agnostic flexibility while protecting the original data from being modified inside the comparison function.
