#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <math.h>

char* const devpath = "/sys/class/backlight/";

typedef enum{
    CMD_INVALID,
    CMD_GET,
    CMD_SET,
} CMD_TYPE;

int return_usage(const char* prgname)
{
    printf("usage:\n");
    printf("\t%s <get/set> [0-100]\n", prgname);
    return -1;
}

int main(int argc, const char** argv) {
    const size_t bufsize = sysconf(_SC_PAGESIZE);
    long light_percent, light_dn, max_light_dn;
    double light_percent_d;
    FILE* backlight_fd;
    errno = 0;
    if (argc < 2)
        return return_usage(argv[0]);

    CMD_TYPE cmd = CMD_INVALID;
    if (strcmp("get", argv[1]) == 0)
        cmd = CMD_GET;
    else if (strcmp("set", argv[1]) == 0)
        cmd = CMD_SET;
    else
        return return_usage(argv[0]);
    
    if (cmd == CMD_SET)
    {
        if (argc < 3)
            return return_usage(argv[0]);
        light_percent_d = strtod(argv[2], NULL);
    }

    DIR* dp = opendir(devpath);

    if (dp == NULL)
    {
        printf("Unable to find backlight device path\n");
        return -1;
    }

    char* buf = malloc(bufsize);
    struct dirent* ep;
    while ((ep = readdir(dp)) != (struct dirent*)NULL)
    {
        if (ep->d_type != DT_LNK)
            continue;
        sprintf(buf, "%s%s/max_brightness", devpath, ep->d_name);
        backlight_fd = fopen(buf, "r");
        char* ret = fgets(buf, sizeof buf, backlight_fd);
        if (ret != buf)
        {
            printf("Error reading device max brightness.\n");
            return -1;
        }
        max_light_dn = strtol(buf, NULL, 10);

        sprintf(buf, "%s%s/brightness", devpath, ep->d_name);
        backlight_fd = fopen(buf, "w");
        if (cmd == CMD_SET)
        {
            if (backlight_fd == (FILE*)NULL)
            {
                switch(errno)
                {
                    case EPERM:
                        printf("%s: permission denied\n", devpath);
                        break;
                    default:
                        printf("unknown errno %d\n", errno);
                        break;
                }

                break;
            }

            light_dn = round(light_percent_d * ((double)max_light_dn) / 100.);
            sprintf(buf, "%ld", light_dn);
            fwrite(buf, 1, strlen(buf), backlight_fd);
        }
        else if (cmd == CMD_GET)
        {
            printf("%s: ", buf);
            FILE* backlight_fd = fopen(buf, "r");
            char* ret = fgets(buf, sizeof buf, backlight_fd);
            if (ret != buf)
            {
                printf("Error reading device.\n");
                return -1;
            }
            light_dn = strtol(buf, NULL, 10);
            light_percent_d =(((double)light_dn) * 100.0/((double)max_light_dn));
            printf("\t%f", light_percent_d);
        }
    }
    
    free(buf);
    return errno;
}
