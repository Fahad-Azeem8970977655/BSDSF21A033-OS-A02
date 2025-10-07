#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>

// ----------------- Feature 6: Color Macros -----------------
#define BLUE "\033[0;34m"
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define PINK "\033[0;35m"
#define REVERSE "\033[7m"
#define RESET "\033[0m"

// ----------------- Feature 2: Long Listing -----------------
void long_listing(const char *filename) {
    struct stat st;
    if (lstat(filename, &st) == -1) {
        perror("lstat");
        return;
    }

    char type = '-';
    if (S_ISDIR(st.st_mode)) type = 'd';
    else if (S_ISLNK(st.st_mode)) type = 'l';

    char perms[10];
    perms[0] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    perms[1] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    perms[2] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    perms[3] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    perms[4] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    perms[5] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    perms[6] = (st.st_mode & S_IROTH) ? 'r' : '-';
    perms[7] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    perms[8] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    perms[9] = '\0';

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);

    char timebuf[80];
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));

    char *color = RESET;
    if (S_ISDIR(st.st_mode)) color = BLUE;
    else if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)) color = GREEN;
    else if (strstr(filename, ".tar") || strstr(filename, ".gz") || strstr(filename, ".zip")) color = RED;
    else if (S_ISLNK(st.st_mode)) color = PINK;
    else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode)) color = REVERSE;

    printf("%c%s %ld %s %s %ld %s %s%s%s\n",
           type,
           perms,
           st.st_nlink,
           pw ? pw->pw_name : "unknown",
           gr ? gr->gr_name : "unknown",
           st.st_size,
           timebuf,
           color,
           filename,
           RESET);
}

// ----------------- Feature 3 & 4: Column & Horizontal Display -----------------
void column_display(char **filenames, int count, int horizontal) {
    if (count == 0) return;

    int max_len = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(filenames[i]);
        if (len > max_len) max_len = len;
    }

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_width = w.ws_col ? w.ws_col : 80;
    int spacing = 2;

    if (horizontal) {
        int pos = 0;
        for (int i = 0; i < count; i++) {
            struct stat st;
            lstat(filenames[i], &st);
            char *color = RESET;
            if (S_ISDIR(st.st_mode)) color = BLUE;
            else if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)) color = GREEN;
            else if (strstr(filenames[i], ".tar") || strstr(filenames[i], ".gz") || strstr(filenames[i], ".zip")) color = RED;
            else if (S_ISLNK(st.st_mode)) color = PINK;
            else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode)) color = REVERSE;

            printf("%s%-*s%s", color, max_len + spacing, filenames[i], RESET);
            pos += max_len + spacing;
            if (pos + max_len + spacing > term_width) { printf("\n"); pos = 0; }
        }
        printf("\n");
    } else {
        int num_columns = term_width / (max_len + spacing);
        if (num_columns < 1) num_columns = 1;
        int num_rows = (count + num_columns - 1) / num_columns;

        for (int r = 0; r < num_rows; r++) {
            for (int c = 0; c < num_columns; c++) {
                int index = r + c * num_rows;
                if (index < count) {
                    struct stat st;
                    lstat(filenames[index], &st);
                    char *color = RESET;
                    if (S_ISDIR(st.st_mode)) color = BLUE;
                    else if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)) color = GREEN;
                    else if (strstr(filenames[index], ".tar") || strstr(filenames[index], ".gz") || strstr(filenames[index], ".zip")) color = RED;
                    else if (S_ISLNK(st.st_mode)) color = PINK;
                    else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode)) color = REVERSE;

                    printf("%s%-*s%s", color, max_len + spacing, filenames[index], RESET);
                }
            }
            printf("\n");
        }
    }
}

// ----------------- Feature 5: Comparison Function for qsort -----------------
int cmpfunc(const void *a, const void *b) {
    const char *s1 = *(const char **)a;
    const char *s2 = *(const char **)b;
    return strcmp(s1, s2);
}

// ----------------- Main Function (Feature 1–5) -----------------
int main(int argc, char *argv[]) {
    int opt;
    int long_flag = 0, horizontal_flag = 0;

    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l': long_flag = 1; break;
            case 'x': horizontal_flag = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [directory]\n", argv[0]);
                return 1;
        }
    }

    char *target_dir = ".";
    if (optind < argc) target_dir = argv[optind];

    DIR *dir = opendir(target_dir);
    if (!dir) { perror("opendir"); return 1; }

    char **filenames = NULL;
    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        filenames = realloc(filenames, sizeof(char*) * (count + 1));
        filenames[count++] = strdup(entry->d_name);
    }
    closedir(dir);

    if (count == 0) { free(filenames); return 0; }

    qsort(filenames, count, sizeof(char*), cmpfunc);

    if (long_flag) {
        for (int i = 0; i < count; i++) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", target_dir, filenames[i]);
            long_listing(path);
        }
    } else {
        column_display(filenames, count, horizontal_flag);
    }

    for (int i = 0; i < count; i++) free(filenames[i]);
    free(filenames);
    return 0;
}


