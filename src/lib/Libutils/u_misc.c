/*
*         OpenPBS (Portable Batch System) v2.3 Software License
*
* Copyright (c) 1999-2000 Veridian Information Solutions, Inc.
* All rights reserved.
*
* ---------------------------------------------------------------------------
* For a license to use or redistribute the OpenPBS software under conditions
* other than those described below, or to purchase support for this software,
* please contact Veridian Systems, PBS Products Department ("Licensor") at:
*
*    www.OpenPBS.org  +1 650 967-4675                  sales@OpenPBS.org
*                        877 902-4PBS (US toll-free)
* ---------------------------------------------------------------------------
*
* This license covers use of the OpenPBS v2.3 software (the "Software") at
* your site or location, and, for certain users, redistribution of the
* Software to other sites and locations.  Use and redistribution of
* OpenPBS v2.3 in source and binary forms, with or without modification,
* are permitted provided that all of the following conditions are met.
* After December 31, 2001, only conditions 3-6 must be met:
*
* 1. Commercial and/or non-commercial use of the Software is permitted
*    provided a current software registration is on file at www.OpenPBS.org.
*    If use of this software contributes to a publication, product, or
*    service, proper attribution must be given; see www.OpenPBS.org/credit.html
*
* 2. Redistribution in any form is only permitted for non-commercial,
*    non-profit purposes.  There can be no charge for the Software or any
*    software incorporating the Software.  Further, there can be no
*    expectation of revenue generated as a consequence of redistributing
*    the Software.
*
* 3. Any Redistribution of source code must retain the above copyright notice
*    and the acknowledgment contained in paragraph 6, this list of conditions
*    and the disclaimer contained in paragraph 7.
*
* 4. Any Redistribution in binary form must reproduce the above copyright
*    notice and the acknowledgment contained in paragraph 6, this list of
*    conditions and the disclaimer contained in paragraph 7 in the
*    documentation and/or other materials provided with the distribution.
*
* 5. Redistributions in any form must be accompanied by information on how to
*    obtain complete source code for the OpenPBS software and any
*    modifications and/or additions to the OpenPBS software.  The source code
*    must either be included in the distribution or be available for no more
*    than the cost of distribution plus a nominal fee, and all modifications
*    and additions to the Software must be freely redistributable by any party
*    (including Licensor) without restriction.
*
* 6. All advertising materials mentioning features or use of the Software must
*    display the following acknowledgment:
*
*     "This product includes software developed by NASA Ames Research Center,
*     Lawrence Livermore National Laboratory, and Veridian Information
*     Solutions, Inc.
*     Visit www.OpenPBS.org for OpenPBS software support,
*     products, and information."
*
* 7. DISCLAIMER OF WARRANTY
*
* THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT
* ARE EXPRESSLY DISCLAIMED.
*
* IN NO EVENT SHALL VERIDIAN CORPORATION, ITS AFFILIATED COMPANIES, OR THE
* U.S. GOVERNMENT OR ANY OF ITS AGENCIES BE LIABLE FOR ANY DIRECT OR INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* This license will be governed by the laws of the Commonwealth of Virginia,
* without reference to its choice of law rules.
*/

#include <pbs_config.h>   /* the master config generated by configure */
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef MAX_CMD_ARGS
#define MAX_CMD_ARGS 10
#endif

int    ArgC = 0;
char **ArgV = NULL;
char  *OriginalPath = NULL;
char  *OriginalCommand = NULL;

/**
 * Abort the program due to memory allocation failure.
 */
static void
fail_nomem(void)
  {
  printf("ERROR:  Insufficient memory to save command line.  Shutting down.\n");
  exit(-1);
  }

/**
 * Locate the executable @a cmd in the supplied @a path.
 *
 * @param cmd   Command to locate (name or absolute path) (I)
 * @param path  Colon-delimited set of directories to search (e.g., $PATH) (I)
 * @return Absolute path to @a cmd (malloc'd), or NULL on failure.
 */
char *
find_command(char *cmd, char *path)
  {
  char *token, *saveptr = NULL;

  /* Duplicate path variable so we don't alter it. */
  if (!path) return NULL;
  else if (!(path = strdup(path)))
    fail_nomem();

  if (!cmd) return NULL;
  else if (*cmd == '/')
    {
    /* Absolute path to command provided. */
    free(path);
    if (access(cmd, X_OK)) return NULL;
    else return strdup(cmd);
    }
  else if (strchr(cmd, '/'))
    {
    char buff[PATH_MAX];
    size_t cwd_len;

    /* Relative path to command provided. */
    free(path);
    if (!getcwd(buff, sizeof(buff))) return NULL;
    cwd_len = strlen(buff);
    if (cwd_len > sizeof(buff) - 2) return NULL;
    strcat(buff, "/");
    cwd_len++;
    strncat(buff, cmd, sizeof(buff) - cwd_len - 1);
    return strdup(buff);
    }

  token = strtok_r(path, ":;", &saveptr);
  while (token)
    {
    size_t len = strlen(token);
    char buff[PATH_MAX];

    if (len)
      {
      if (token[len - 1] == '/')
        {
        snprintf(buff, sizeof(buff), "%s%s", token, cmd);
        }
      else
        {
        snprintf(buff, sizeof(buff), "%s/%s", token, cmd);
        }
      if (!access(buff, X_OK))
        {
        free(path);
        return strdup(buff);
        }
      }
    token = strtok_r(NULL, ":;", &saveptr);
    }
  free(path);
  return NULL;
  }

/**
 * Store the program's @a argc and @a argv[] for later use.
 *
 * @param argc The @a argc value passed to main(). (I)
 * @param argv The @a argv[] array passed to main(). (I)
 */
void save_args(int argc, char **argv)
  {
  int i;

  ArgC = argc;
  if (!(ArgV = (char **) malloc(sizeof(char *) * (ArgC + 1))))
    fail_nomem();
  ArgV[ArgC] = 0;

  /* save argv and the path for later use */
  for (i = 0; i < ArgC; i++)
    {
    ArgV[i] = strdup(argv[i]);
    if (!ArgV[i]) fail_nomem();
    }

  /* save the path before we go into the background.  If we don't do this
   * we can't restart the server because the path will change */
  if (!(OriginalPath = strdup(getenv("PATH"))))
    fail_nomem();

  OriginalCommand = ArgV[0];
  ArgV[0] = find_command(ArgV[0], OriginalPath);
  if (!ArgV[0])
    ArgV[0] = OriginalCommand;
  }
