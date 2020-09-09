#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <openssl/md5.h>
#include <linux/limits.h>

#include <bfd.h>

#include "cpu-utils.h"
#include "cpu-common.h"
#include "log.h"

#define CRICKET_ELF_NV_INFO_PREFIX ".nv.info"
#define CRICKET_ELF_NV_SHARED_PREFIX ".nv.shared."
#define CRICKET_ELF_NV_TEXT_PREFIX ".nv.text."
#define CRICKET_ELF_TEXT_PREFIX ".text."

#define CRICKET_ELF_FATBIN ".nv_fatbin"
#define CRICKET_ELF_REGFUN "_ZL24__sti____cudaRegisterAllv"

int cpu_utils_md5hash(char *filename, unsigned long *high, unsigned long *low)
{
    unsigned char c[MD5_DIGEST_LENGTH];
    FILE *fd;
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[1024];

    if (filename == NULL || high == NULL || low == NULL) {
        return -1;
    }

    if ((fd = fopen(filename, "rb")) == NULL) {
        LOGE(LOG_ERROR, "%s can't be opened.", filename);
        return -1;
    }

    MD5_Init (&mdContext);
    while ((bytes = fread(data, 1, 1024, fd)) != 0)
        MD5_Update(&mdContext, data, bytes);
    MD5_Final(c, &mdContext);
    fclose (fd);
    *high = *((unsigned long*)c);
    *low  = *((unsigned long*)(c+8));
    return 0;
}

void* cricketd_utils_symbol_address(char *symbol)
{
    bfd *hostbfd = NULL;
    asection *section;
    FILE *hostbfd_fd = NULL;
    void *ret = NULL;
    size_t symtab_size, symtab_length;
    asymbol **symtab = NULL;


    bfd_init();

    if ((hostbfd_fd = fopen("/proc/self/exe", "rb")) == NULL) {
        LOGE(LOG_ERROR, "fopen failed");
        return NULL;
    }

    if ((hostbfd = bfd_openstreamr("/proc/self/exe", NULL, hostbfd_fd)) == NULL) {
        LOGE(LOG_ERROR, "bfd_openr failed on %s",
                "/proc/self/exe");
        fclose(hostbfd_fd);
        goto cleanup;
    }

    if (!bfd_check_format(hostbfd, bfd_object)) {
        LOGE(LOG_ERROR, "%s has wrong bfd format",
                "/proc/self/exe");
        goto cleanup;
    }

    if ((symtab_size = bfd_get_symtab_upper_bound(hostbfd)) == -1) {
        LOGE(LOG_ERROR, "bfd_get_symtab_upper_bound failed");
        return NULL;
    }

    if ((symtab = (asymbol **)malloc(symtab_size)) == NULL) {
        LOGE(LOG_ERROR, "malloc symtab failed");
        return NULL;
    }

    if ((symtab_length = bfd_canonicalize_symtab(hostbfd, symtab)) == 0) {
        LOG(LOG_WARNING, "symtab is empty...");
    } else {
        //printf("%lu symtab entries\n", symtab_length);
    }

    for (int i = 0; i < symtab_length; ++i) {
        if (strcmp(bfd_asymbol_name(symtab[i]), CRICKET_ELF_REGFUN) == 0) {
            ret = (void*)bfd_asymbol_value(symtab[i]);
            break;
        }
        //printf("%d: %s: %lx\n", i, bfd_asymbol_name(symtab[i]),
        //       bfd_asymbol_value(symtab[i]));
    }


cleanup:
    free(symtab);
    if (hostbfd != NULL)
        bfd_close(hostbfd);
    return ret;
}

int cpu_utils_launch_child(const char *file, char **args)
{
    int filedes[2];
    FILE *fd = NULL;

    if (pipe(filedes) == -1) {
        LOGE(LOG_ERROR, "error while creating pipe");
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        LOGE(LOG_ERROR, "error while forking");
        return -1;
    } else if (pid == 0) {
        while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
        close(filedes[1]);
        close(filedes[0]);
        char *env[] = {NULL};
        execvpe(file, args, env);
        exit(1);
    }
    close(filedes[1]);
    return filedes[0];
}

kernel_info_t* cricketd_utils_search_info(kernel_info_t *infos, size_t kernelnum, char *kernelname)
{
    if (infos == NULL || kernelname == NULL) {
        LOGE(LOG_ERROR, "parameters are supposed to be pre-allocated.");
        return NULL;
    }

    for (int i=0; i < kernelnum; ++i) {
        if (strcmp(kernelname, infos[i].name) == 0) {
            return &infos[i];
        }
    }
    return NULL;
}

static int cpu_utils_read_pars(kernel_info_t *info, FILE* fdesc)
{
    static const char* attr_str[] = {"EIATTR_KPARAM_INFO",
                                     "EIATTR_CBANK_PARAM_SIZE",
                                     "EIATTR_PARAM_CBANK"};
    enum attr_t {KPARAM_INFO = 0,
                 CBANK_PARAM_SIZE = 1,
                 PARAM_CBANK = 2,
                 ATTR_T_LAST}; // states for state machine
    char *line = NULL;
    size_t linelen = 0;
    int ret = 1;
    int read = 0;
    char key[32];
    char val[256] = {0};
    enum attr_t cur_attr = ATTR_T_LAST; // current state of state machine
    info->param_num = 0;
    info->param_offsets = NULL;
    info->param_sizes = NULL;
    while (getline(&line, &linelen, fdesc) != -1) {
        memset(val, 0, 256);
        read = sscanf(line, "%32s %255c\n", key, val);
        val[strlen(val)-1] = '\0';
        if (read == -1 || read == 0) {
            break; //empty line means there is no more info for this kernel
        } else if (read == 1) {
            continue; // some lines have no key-value pair.
                      // We are not interested in those lines.
        }
        if (strcmp(key, "Attribute:") == 0) { // state change
            LOG(LOG_DBG(3), "\"%s\", \"%s\"", key, val);
            cur_attr = ATTR_T_LAST;
            for (int i=0; i < ATTR_T_LAST; i++) {
                if (strcmp(val, attr_str[i]) == 0) {
                    LOG(LOG_DBG(3), "found %s", attr_str[i]);
                    cur_attr = i;
                }
            }
        } else if(strcmp(key, "Value:") == 0) {
            LOG(LOG_DBG(3), "\"%s\", \"%s\"", key, val);
            size_t buf;
            uint16_t ordinal, offset, size;
            switch(cur_attr) {
            case KPARAM_INFO:
                if (sscanf(val, "Index : 0x%*hx Ordinal : 0x%hx Offset : 0x%hx Size : 0x%hx\n", &ordinal, &offset, &size) != 3 ) {
                    LOGE(LOG_ERROR, "unexpected format of cuobjdump output");
                    goto cleanup;
                }
                if (ordinal >= info->param_num) {
                    info->param_offsets = realloc(
                                info->param_offsets,
                                (ordinal+1)*sizeof(uint16_t));
                    info->param_sizes = realloc(
                                info->param_sizes,
                                (ordinal+1)*sizeof(uint16_t));
                    info->param_num = ordinal+1;
                }
                info->param_offsets[ordinal] = offset;
                info->param_sizes[ordinal] = size;
                break;
            case CBANK_PARAM_SIZE:
                if (sscanf(val, "0x%lx", &info->param_size) != 1) {
                    LOGE(LOG_ERROR, "value has wrong format: key: %s, val: %s", key, val);
                    goto cleanup;
                }
                break;
            case PARAM_CBANK:
                if (sscanf(val, "0x%*x 0x%lx", &buf) != 1) {
                    LOGE(LOG_ERROR, "value has wrong format: key: %s, val: %s", key, val);
                    goto cleanup;
                }
                LOG(LOG_DBG(3), "found param address: %d", (uint16_t)(buf & 0xFFFF));
                break;
            default:
                break;
            }
        }


    }

    ret = 0;
 cleanup:
    free(line);
    return ret;
}

int cpu_utils_parameter_info(kernel_info_t **infos, size_t *kernelnum)
{
    int ret = 1;
    char linktarget[PATH_MAX] = {0};
    char *args[] = {"cuobjdump", "--dump-elf", NULL, NULL};
    int output;
    FILE *fdesc; //fd to read subcommands output from
    int child_exit = 0;
    char *line = NULL;
    size_t linelen;
    static const char nv_info_prefix[] = ".nv.info.";
    kernel_info_t *buf = NULL;
    char *kernelname;

    if (infos == NULL || kernelnum == NULL) {
        LOGE(LOG_ERROR, "parameters are supposed to be pre-allocated.");
        goto out;
    }
    *kernelnum = 0;
    *infos = NULL;

    if (readlink("/proc/self/exe", linktarget, PATH_MAX) == PATH_MAX) {
        LOGE(LOG_ERROR, "executable path length is too long");
        goto out;
    }
    LOG(LOG_DBG(1), "we are running the following binary: \"%s\".", linktarget);
    args[2] = linktarget;

    if ( (output = cpu_utils_launch_child(args[0], args)) == -1) {
        LOGE(LOG_ERROR, "error while launching child.");
        goto out;
    }

    if ( (fdesc = fdopen(output, "r")) == NULL) {
        LOGE(LOG_ERROR, "erro while opening stream");
        goto cleanup1;
    }

    while (getline(&line, &linelen, fdesc) != -1) {
        if (strncmp(line, nv_info_prefix, strlen(nv_info_prefix)) != 0) {
            // Line does not start with .nv.info. so continue searching.
            continue;
        }
        // Line starts with .nv.info.
        // Kernelname is line + strlen(nv_info_prefix)
        kernelname = line + strlen(nv_info_prefix);
        if (strlen(kernelname) == 0) {
            LOGE(LOG_ERROR, "found .nv.info section, but kernelname is empty");
            goto cleanup2;
        }

        if ((*infos = realloc(*infos, (++(*kernelnum))*sizeof(kernel_info_t))) == NULL) {
            LOGE(LOG_ERROR, "realloc failed");
            goto cleanup2;
        }
        buf = &((*infos)[(*kernelnum)-1]);
        memset(buf, 0, sizeof(kernel_info_t));
        if ((buf->name = malloc(strlen(kernelname))) == NULL) {
            LOGE(LOG_ERROR, "malloc failed");
            goto cleanup2;
        }
        //copy string and remove trailing \n
        strncpy(buf->name, kernelname, strlen(kernelname)-1);
        buf->name[strlen(kernelname)-1] = '\0';
    
        if (cpu_utils_read_pars(buf, fdesc) != 0) {
            LOGE(LOG_ERROR, "reading paramter infos failed.\n");
            goto cleanup2;
        }

        LOG(LOG_DEBUG, "found kernel \"%s\" [param_num: %d, param_size: %d]",
            buf->name, buf->param_num, buf->param_size);

    }

    if (ferror(fdesc) != 0) {
        LOGE(LOG_ERROR, "file descriptor shows an error");
        goto cleanup2;
    }

    ret = 0;
 cleanup2:
    fclose(fdesc);
 cleanup1:
    close(output);
    wait(&child_exit);
    LOG(LOG_DEBUG, "child exit code: %d", child_exit);
 out:
    free(line);
    return (ret != 0 ? ret : child_exit);
}

void kernel_infos_free(kernel_info_t *infos, size_t kernelnum)
{
    for (int i=0; i < kernelnum; ++i) {
        free(infos[i].name);
        free(infos[i].param_offsets);
    }
}
