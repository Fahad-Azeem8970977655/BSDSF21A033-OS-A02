What is the crucial difference between the stat() and lstat() system calls? In the context of
the ls command, when is it more appropriate to use lstat()?

stat(): Returns information about the file that the path points to.

If the path is a symbolic link, stat() returns information about the target of the link, not the link itself.

lstat(): Returns information about the file itself.

If the path is a symbolic link, lstat() returns information about the link, not the file it points to.

In the context of the ls command:

Use lstat() when you want to display information about symbolic links themselves (like ls -l shows symlink -> target).

Use stat() when you are only interested in the actual files/directories (not the links).

struct stat fileStat;
lstat("linkname", &fileStat); // info about the symbolic link
stat("linkname", &fileStat);  // info about the actual file the link points to

Extracting File Type and Permissions from st_mode

The st_mode field contains both the file type and permission bits packed in a single integer.

File type: Use bitwise AND with predefined macros like S_IFDIR, S_IFREG, S_IFLNK, etc.
if ((fileStat.st_mode & S_IFMT) == S_IFDIR) {
    printf("It's a directory\n");
}
if ((fileStat.st_mode & S_IFMT) == S_IFREG) {
    printf("It's a regular file\n");
}
ermission bits: Use bitwise AND with macros like S_IRUSR (read by owner), S_IWUSR (write by owner), S_IXUSR (execute by owner), etc.
if (fileStat.st_mode & S_IRUSR) printf("Owner can read\n");
if (fileStat.st_mode & S_IWUSR) printf("Owner can write\n");
if (fileStat.st_mode & S_IXUSR) printf("Owner can execute\n");
S_IFMT is a mask to extract only the file type bits, ignoring the permission bits.
