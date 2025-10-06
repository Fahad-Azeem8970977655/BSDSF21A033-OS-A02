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

// ----------------- Feature 3: Vertical Column Display -----------------
void vertical_display(const char *target_dir) {
    DIR *dir = opendir(target_dir);
    if (!dir) {
        perror("opendir");
        return;
    }

    char **filenames = NULL;
    int count = 0, max_len = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
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

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_width = w.ws_col ? w.ws_col : 80;
    int spacing = 2;
    int num_columns = term_width / (max_len + spacing);
    if (num_columns < 1) num_columns = 1;
    int num_rows = (count + num_columns - 1) / num_columns;

    for (int r = 0; r < num_rows; r++) {
        for (int c = 0; c < num_columns; c++) {
            int index = r + c * num_rows;
            if (index < count)
                printf("%-*s", max_len + spacing, filenames[index]);
        }
        printf("\n");
    }

    for (int i = 0; i < count; i++) free(filenames[i]);
    free(filenames);
}

// ----------------- Feature 4: Horizontal Display (-x Option) -----------------
void horizontal_display(const char *target_dir) {
    DIR *dir = opendir(target_dir);
    if (!dir) {
        perror("opendir");
        return;
    }

    char **filenames = NULL;
    int count = 0, max_len = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
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

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_width = w.ws_col ? w.ws_col : 80;
    int spacing = 2;
    int pos = 0;

    for (int i = 0; i < count; i++) {
        printf("%-*s", max_len + spacing, filenames[i]);
        pos += max_len + spacing;
        if (pos + max_len + spacing > term_width) {
            printf("\n");
            pos = 0;
        }
    }
    if (pos != 0) printf("\n");

    for (int i = 0; i < count; i++) free(filenames[i]);
    free(filenames);
}

// ----------------- Main Function -----------------
int main(int argc, char *argv[]) {
    int opt;
    int display_mode = 0; // 0=vertical, 1=long listing (-l), 2=horizontal (-x)

    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch(opt) {
            case 'l': display_mode = 1; break;
            case 'x': display_mode = 2; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [directory]\n", argv[0]);
                return 1;
        }
    }

    char *target_dir = ".";
    if (optind < argc) target_dir = argv[optind];

    if (display_mode == 1) {
        DIR *dir = opendir(target_dir);
        if (!dir) { perror("opendir"); return 1; }
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') {
                char filepath[1024];
                snprintf(filepath, sizeof(filepath), "%s/%s", target_dir, entry->d_name);
                long_listing(filepath);
            }
        }
        closedir(dir);
    } else if (display_mode == 2) {
        horizontal_display(target_dir);
    } else {
        vertical_display(target_dir);
    }

    return 0;
}


