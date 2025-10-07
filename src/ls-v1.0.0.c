#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <limits.h>

#define COLOR_RESET   "\033[0m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_RED     "\033[0;31m"
#define COLOR_PINK    "\033[0;35m"
#define COLOR_REVERSE "\033[7m"

int long_flag = 0;
int horizontal_flag = 0;
int recursive_flag = 0;
int spacing = 2;

// Comparison function for qsort
int cmpstring(const void *a, const void *b) {
    char *const *str1 = a;
    char *const *str2 = b;
    return strcmp(*str1, *str2);
}

// Function to display a file in long format (-l)
void long_listing(char *filename) {
    struct stat st;
    lstat(filename, &st);
    char perms[11] = "----------";

    if (S_ISDIR(st.st_mode)) perms[0] = 'd';
    else if (S_ISLNK(st.st_mode)) perms[0] = 'l';
    else if (S_ISCHR(st.st_mode)) perms[0] = 'c';
    else if (S_ISBLK(st.st_mode)) perms[0] = 'b';
    else if (S_ISSOCK(st.st_mode)) perms[0] = 's';
    else if (S_ISFIFO(st.st_mode)) perms[0] = 'p';

    if (st.st_mode & S_IRUSR) perms[1] = 'r';
    if (st.st_mode & S_IWUSR) perms[2] = 'w';
    if (st.st_mode & S_IXUSR) perms[3] = 'x';
    if (st.st_mode & S_IRGRP) perms[4] = 'r';
    if (st.st_mode & S_IWGRP) perms[5] = 'w';
    if (st.st_mode & S_IXGRP) perms[6] = 'x';
    if (st.st_mode & S_IROTH) perms[7] = 'r';
    if (st.st_mode & S_IWOTH) perms[8] = 'w';
    if (st.st_mode & S_IXOTH) perms[9] = 'x';

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));

    printf("%s %ld %s %s %5ld %s %s\n", perms, st.st_nlink, pw->pw_name, gr->gr_name, st.st_size, timebuf, filename);
}

// Function to print filename with color based on type
void print_colored(char *filename) {
    struct stat st;
    lstat(filename, &st);

    char *color = COLOR_RESET;
    if (S_ISDIR(st.st_mode)) color = COLOR_BLUE;
    else if (S_ISLNK(st.st_mode)) color = COLOR_PINK;
    else if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)) color = COLOR_GREEN;
    else if (strstr(filename, ".tar") || strstr(filename, ".gz") || strstr(filename, ".zip")) color = COLOR_RED;
    else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISSOCK(st.st_mode) || S_ISFIFO(st.st_mode)) color = COLOR_REVERSE;

    printf("%s%s%s", color, filename, COLOR_RESET);
}

// Horizontal display (-x)
void horizontal_display(char **filenames, int count) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_width = w.ws_col;
    int max_len = 0;
    for (int i = 0; i < count; i++)
        if ((int)strlen(filenames[i]) > max_len) max_len = strlen(filenames[i]);

    int pos = 0;
    for (int i = 0; i < count; i++) {
        print_colored(filenames[i]);
        printf("%-*s", max_len + spacing - (int)strlen(filenames[i]), "");
        pos += max_len + spacing;
        if (pos + max_len + spacing > term_width) { printf("\n"); pos = 0; }
    }
    printf("\n");
}

// Column display (default vertical)
void column_display(char **filenames, int count) {
    int max_len = 0;
    for (int i = 0; i < count; i++)
        if ((int)strlen(filenames[i]) > max_len) max_len = strlen(filenames[i]);

    int rows = 0;
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_width = w.ws_col;
    int cols = term_width / (max_len + spacing);
    if (cols == 0) cols = 1;
    rows = (count + cols - 1) / cols;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int idx = c * rows + r;
            if (idx < count) {
                print_colored(filenames[idx]);
                printf("%-*s", max_len + spacing - (int)strlen(filenames[idx]), "");
            }
        }
        printf("\n");
    }
}

// Core directory listing function
void do_ls(char *dirname) {
    DIR *dir = opendir(dirname);
    if (!dir) { perror("opendir"); return; }

    struct dirent *entry;
    char **filenames = NULL;
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        filenames = realloc(filenames, sizeof(char*) * (count + 1));
        filenames[count] = strdup(entry->d_name);
        count++;
    }
    closedir(dir);

    qsort(filenames, count, sizeof(char*), cmpstring);

    for (int i = 0; i < count; i++) {
        if (long_flag) long_listing(filenames[i]);
        else if (horizontal_flag) horizontal_display(filenames, count);
        else column_display(filenames, count);
        break; // display only once if not -l
    }

    // Recursive logic for -R
    if (recursive_flag) {
        for (int i = 0; i < count; i++) {
            if (strcmp(filenames[i], ".") == 0 || strcmp(filenames[i], "..") == 0) continue;
            char fullpath[PATH_MAX];
            snprintf(fullpath, PATH_MAX, "%s/%s", dirname, filenames[i]);
            struct stat st;
            lstat(fullpath, &st);
            if (S_ISDIR(st.st_mode)) {
                printf("\n");
                do_ls(fullpath);
            }
        }
    }

    for (int i = 0; i < count; i++) free(filenames[i]);
    free(filenames);
}

// Main function with argument parsing
int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "l x R")) != -1) {
        switch (opt) {
            case 'l': long_flag = 1; break;
            case 'x': horizontal_flag = 1; break;
            case 'R': recursive_flag = 1; break;
        }
    }

    if (optind == argc) {
        do_ls(".");
    } else {
        for (int i = optind; i < argc; i++) {
            do_ls(argv[i]);
        }
    }
    return 0;
}



