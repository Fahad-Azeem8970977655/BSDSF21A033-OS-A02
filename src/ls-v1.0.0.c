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

// ----------------- Feature 2: Long Listing -----------------
void long_listing(const char *filename) {
    struct stat st;
    if (lstat(filename, &st) == -1) {
        perror("lstat");
        return;
    }

    // File type
    char type = '-';
    if (S_ISDIR(st.st_mode)) type = 'd';
    else if (S_ISLNK(st.st_mode)) type = 'l';

    // Permissions
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

    // Owner and group
    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);

    // Modification time
    char timebuf[80];
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));

    printf("%c%s %ld %s %s %ld %s %s\n",
           type,
           perms,
           st.st_nlink,
           pw ? pw->pw_name : "unknown",
           gr ? gr->gr_name : "unknown",
           st.st_size,
           timebuf,
           filename);
}

// ----------------- Feature 3: Column Display -----------------
void column_display(const char *target_dir) {
    DIR *dir = opendir(target_dir);
    if (!dir) {
        perror("opendir");
        return;
    }

    // Read all filenames into dynamic array
    char **filenames = NULL;
    int count = 0, max_len = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // skip hidden

        filenames = realloc(filenames, sizeof(char*) * (count + 1));
        filenames[count] = strdup(entry->d_name);
        int len = strlen(entry->d_name);
        if (len > max_len) max_len = len;
        count++;
    }
    closedir(dir);

    if (count == 0) {
        free(filenames);
        return;
    }

    // Get terminal width
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_width = w.ws_col ? w.ws_col : 80; // fallback
    int spacing = 2;
    int num_columns = term_width / (max_len + spacing);
    if (num_columns < 1) num_columns = 1;
    int num_rows = (count + num_columns - 1) / num_columns; // ceil division

    // Print "down then across"
    for (int r = 0; r < num_rows; r++) {
        for (int c = 0; c < num_columns; c++) {
            int index = r + c * num_rows;
            if (index < count)
                printf("%-*s", max_len + spacing, filenames[index]);
        }
        printf("\n");
    }

    // Free memory
    for (int i = 0; i < count; i++) free(filenames[i]);
    free(filenames);
}

// ----------------- Main Function -----------------
int main(int argc, char *argv[]) {
    int opt;
    int long_flag = 0;

    // Parse options
    while ((opt = getopt(argc, argv, "l")) != -1) {
        switch (opt) {
            case 'l': long_flag = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [directory]\n", argv[0]);
                return 1;
        }
    }

    char *target_dir = ".";
    if (optind < argc) target_dir = argv[optind];

    if (long_flag) {
        // Long listing: Feature 2
        DIR *dir = opendir(target_dir);
        if (!dir) {
            perror("opendir");
            return 1;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') {
                char filepath[1024];
                snprintf(filepath, sizeof(filepath), "%s/%s", target_dir, entry->d_name);
                long_listing(filepath);
            }
        }
        closedir(dir);
    } else {
        // Column display: Feature 3
        column_display(target_dir);
    }

    return 0;
}

