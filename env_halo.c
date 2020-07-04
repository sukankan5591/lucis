#include "env_halo.h"

#define    SHELL_SIZE      16
#define    USER_SIZE       40
#define    HOME_SIZE       40
#define    LOGNAME_SIZE    40
#define    PWD_SIZE        4096

static char TERM[]                = "TERM=xterm-256color";
static char SHELL[SHELL_SIZE]     = "SHELL=";
static char USER[USER_SIZE]       = "USER=";
static char HOME[HOME_SIZE]       = "HOME=";
static char LOGNAME[LOGNAME_SIZE] = "LOGNAME=";
static char PWD[PWD_SIZE]         = "PWD=";
static char LANGUAGE[]            = "LANGUAGE=en_US.UTF-8";
static char LANG[]                = "LANGUAGE=en_US.UTF-8";
static char LC_ALL[]              = "LC_ALL=en_US.UTF-8";
static char PS1[]                 = "PS1=\\[\\033[1;31m\\]\\u\\[\\033[33m\\]@\\[\\033[36m\\]\\h\\[\\033[35m\\] \\W\\[\\033[0m\\] $ ";
static char PATH[]                = "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin";
static char LS_COLORS[]           = "LS_COLORS=rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arj=01;31:*.taz=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lz=01;31:*.xz=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.jpg=01;35:*.jpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.axv=01;35:*.anx=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=00;36:*.au=00;36:*.flac=00;36:*.mid=00;36:*.midi=00;36:*.mka=00;36:*.mp3=00;36:*.mpc=00;36:*.ogg=00;36:*.ra=00;36:*.wav=00;36:*.axa=00;36:*.oga=00;36:*.spx=00;36:*.xspf=00;36:";

char *halo_env[]={
    TERM,
    SHELL,
    USER,
    HOME,
    PWD,
    LOGNAME,
    LS_COLORS,
    PATH,
    LANGUAGE,
    LANG,
    LC_ALL,
    PS1,
    NULL
};


void set_env_HOME(const char *str)
{
    strncpy(HOME+5, str, HOME_SIZE);
}


void set_env_USER(const char *str)
{
    strncpy(USER+5, str, USER_SIZE);
}


void set_env_SHELL(const char *str)
{
    strncpy(SHELL+6, str, SHELL_SIZE);
}


void set_env_PWD(const char *str)
{
    strncpy(PWD+4, str, PWD_SIZE);
}


void set_env_LOGNAME(const char *str)
{
    strncpy(LOGNAME+8, str, LOGNAME_SIZE);
}


void handle_env(const struct passwd *pwd)
{
    set_env_HOME(pwd->pw_dir);
    set_env_USER(pwd->pw_name);
    set_env_SHELL(pwd->pw_shell);
    set_env_PWD(pwd->pw_dir);
    set_env_LOGNAME(pwd->pw_name);
}
