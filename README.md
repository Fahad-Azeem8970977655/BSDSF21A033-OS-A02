Feature 1 & 2 – Long Listing and Simple Listing

Difference between stat() and lstat()

stat() returns information about the file that the path points to. If the path is a symbolic link, stat() returns information about the target file, not the link itself. lstat() returns information about the file itself. If the path is a symbolic link, lstat() returns information about the link, not the file it points to.

In the context of the ls command, use lstat() when you want to display information about symbolic links themselves, for example when using "ls -l" which shows "symlink -> target". Use stat() when you are only interested in the actual files or directories and not the symbolic links.

Example: To get information about the symbolic link itself, use lstat("linkname", &fileStat); and to get information about the actual file the link points to, use stat("linkname", &fileStat);
Extracting File Type and Permissions from st_mode

The st_mode field in struct stat contains both the file type and permission bits packed in a single integer. To determine the file type, use a bitwise AND with predefined macros like S_IFDIR, S_IFREG, or S_IFLNK. For example, if ((fileStat.st_mode & S_IFMT) == S_IFDIR) then it is a directory. If ((fileStat.st_mode & S_IFMT) == S_IFREG) then it is a regular file.

To determine permission bits, use bitwise AND with macros like S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, and S_IXOTH. For example, if (fileStat.st_mode & S_IRUSR) then the owner can read, if (fileStat.st_mode & S_IWUSR) then the owner can write, and if (fileStat.st_mode & S_IXUSR) then the owner can execute. S_IFMT is a mask used to extract only the file type bits and ignore permission bits.

Feature 3 – Column Display (Down Then Across)
Down-Then-Across Column Logic

In a columnar display like the default ls output, files are displayed in columns that fill top-to-bottom first, and then move to the next column. The logic involves first determining the terminal width and the length of the longest filename. Then, the number of columns is calculated using the formula: num_columns = terminal_width / (max_filename_length + spacing), and the number of rows is calculated as num_rows = ceil(total_files / num_columns).

To print the files in down-then-across format, iterate row by row. For each row i from 0 to num_rows-1, print the filenames at positions i, i + num_rows, i + 2*num_rows, and so on. This ensures that each column is filled top-to-bottom first. A simple single loop through the filenames is insufficient because it would fill the output left-to-right, row by row, which does not match the standard behavior of ls. Proper mapping of each file to its row-column position is required for correct alignment.
Purpose of ioctl

The ioctl system call with the TIOCGWINSZ request is used to detect the current terminal width. This allows the program to dynamically calculate how many columns can fit on the screen. Using ioctl ensures that the output adapts to terminal resizing and columns remain properly aligned.

Limitations of a Fixed-Width Fallback

If a fixed width (for example, 80 columns) were used instead of detecting the terminal width, the output might overflow on smaller terminals, causing misaligned formatting. On larger terminals, space would be wasted and the display would not utilize the available width efficiently. The program would be less dynamic and not behave like the standard ls utility, which automatically adjusts columns based on the terminal size.
