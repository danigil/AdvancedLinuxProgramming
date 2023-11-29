#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pwd.h>
#include <grp.h>

#define TEXTBG_GREEN "\x1b[42m"
#define TEXTBG_TRANSPARENT "\x1b[m"

#define TEXTFG_BLACK "\x1b[30m"
#define TEXTFG_CYAN "\x1b[36m"
#define TEXTFG_GREEN "\x1b[32m"

#define TEXT_BOLD  "\e[1m"
#define TEXT_OFF   "\e[m"

#define TEXT_DIR   "\033[34;42m"
#define TEXT_FILE "\033[1;32m"

static char permissions[11];

char f_type(mode_t mode)
{
    char c;

    switch (mode & S_IFMT)
    {
    case S_IFBLK:
        c = 'b';
        break;
    case S_IFCHR:
        c = 'c';
        break;
    case S_IFDIR:
        c = 'd';
        break;
    case S_IFIFO:
        c = 'p';
        break;
    case S_IFLNK:
        c = 'l';
        break;
    case S_IFREG:
        c = '-';
        break;
    case S_IFSOCK:
        c = 's';
        break;
    default:
        c = '?';
        break;
    }
    return (c);
}

void print_permissions(const struct stat *sb){
    permissions[0] = f_type(sb->st_mode);
    //permissions[0] = filetypeletter(sb->st_mode);
    //permissions[0] = '$';
    permissions[1] = (sb->st_mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (sb->st_mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (sb->st_mode & S_IXUSR) ? 'x' : '-';
    permissions[4] = (sb->st_mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (sb->st_mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (sb->st_mode & S_IXGRP) ? 'x' : '-';
    permissions[7] = (sb->st_mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (sb->st_mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (sb->st_mode & S_IXOTH) ? 'x' : '-';
}

static struct stat buf;
static struct group *grp;
static struct passwd *pwd;

static int fileCounter = 0;
static int dirCounter = 0;
static char* root;
static char* bgcolor;
static char* fgcolor;
static char* currcolor;

static int
display_info(const char *fpath, const struct stat *sb,
             int tflag, struct FTW *ftwbuf)
{
    if(!strcmp(fpath, root)){
        //printf("%s%s%s\n", TEXT_DIR, fpath, TEXT_OFF);
        printf("%s\n", fpath);
        return 0;
    }

    lstat (fpath, &buf);
    print_permissions(&buf);

    grp = getgrgid(buf.st_gid);
    pwd = getpwuid(buf.st_uid);

    if(tflag == FTW_D){
        dirCounter++;
        currcolor = TEXT_DIR;
        // bgcolor = TEXTBG_GREEN;
        // fgcolor = TEXTFG_CYAN;
    }
    else{
        fileCounter++;
        currcolor = TEXT_FILE;
        // bgcolor = TEXTBG_TRANSPARENT;
        // fgcolor = TEXTFG_GREEN;
    }

    for (int i = 0; i < ftwbuf->level -1; i++) {
            printf("│   ");
            
        }
    if (ftwbuf->level > 0) { 
        if (ftwbuf->base == ftwbuf->level){
            printf(" └── ");
        }else{
            printf("├── "); 
        }        
    }

    // printf("%-3s %2d [%s %7jd]   %-40s %d %s\n",
    //     (tflag == FTW_D) ?   "d"   : (tflag == FTW_DNR) ? "dnr" :
    //     (tflag == FTW_DP) ?  "dp"  : (tflag == FTW_F) ?   "f" :
    //     (tflag == FTW_NS) ?  "ns"  : (tflag == FTW_SL) ?  "sl" :
    //     (tflag == FTW_SLN) ? "sln" : "???",
    //     ftwbuf->level, permissions,(intmax_t) sb->st_size,
    //     fpath, ftwbuf->base, fpath + ftwbuf->base);
    printf("[%s %s   %s %14jd]  %s\n",
            permissions,
            pwd->pw_name,
            grp->gr_name,
            (intmax_t) sb->st_size,
            //TEXT_BOLD,
            //bgcolor,
            //fgcolor,
            // currcolor,
            fpath + ftwbuf->base);
            // TEXT_OFF
            
            //TEXTBG_TRANSPARENT,
            //TEXTFG_BLACK,
            //TEXT_OFF);
    return 0;           /* To tell nftw() to continue */
}



int
main(int argc, char *argv[])
{
    int flags = 0;
    
    memset(permissions, '\0', 11);

   if (argc > 2 && strchr(argv[2], 'd') != NULL)
        flags |= FTW_DEPTH;
    if (argc > 2 && strchr(argv[2], 'p') != NULL)
        flags |= FTW_PHYS;

    if(argc < 2){
        root = malloc(2*sizeof(char));
        if(root == NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(root, ".");
    }
    else{
        root = malloc((strlen(argv[1])+1)*sizeof(char));
        if(root == NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(root, argv[1]);
    }
    

   if (nftw(root, display_info, 20, flags)
            == -1) {
        perror("nftw");
        exit(EXIT_FAILURE);
    }

    char *dir;
    char *file;
    if(dirCounter == 0 || dirCounter > 1)
        dir = "directories";
    else
        dir = "directory";

    if(fileCounter == 0 || fileCounter > 1)
        file = "files";
    else
        file = "file";

    printf("\n%d %s, %d %s\n", dirCounter, dir, fileCounter, file);
    free(root);
    exit(EXIT_SUCCESS);
}
