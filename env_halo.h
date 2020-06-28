#ifndef _ENV_HALO_H_
#define _ENV_HALO_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>

extern char *halo_env[2048];

void set_env_HOME(const char *);
void set_env_USER(const char *);
void set_env_SHELL(const char *);
void set_env_PWD(const char *);
void set_env_LOGNAME(const char *);
void handle_env(const struct passwd *);

#endif
