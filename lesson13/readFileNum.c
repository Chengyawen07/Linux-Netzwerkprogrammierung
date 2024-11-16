#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

int getFileNum(const char * path);

int main(int argc, char * argv[]) {
    if(argc < 2) {
        printf("Usage: %s path\n", argv[0]);
        return -1;
    }

    int num = getFileNum(argv[1]);
    printf("普通文件的个数为：%d\n", num);

    return 0;
}

int getFileNum(const char * path) {
    DIR *dir = opendir(path);
    if(dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    int total = 0;
    char fullPath[1024];

    while((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);
        
        struct stat statbuf;
        if (stat(fullPath, &statbuf) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            total += getFileNum(fullPath);
        } else if (S_ISREG(statbuf.st_mode)) {
            total++;
        }
    }

    closedir(dir);
    return total;
}
