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

#include <assert.h>
#include <ctype.h>
#include <memory.h>
#ifndef NDEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "pbs_ifl.h"
#include "list_link.h"
#include "attribute.h"
#include "pbs_error.h"

/*
 * This file contains functions for manipulating attributes of type string
 *
 * Then there are a set of functions for each type of attribute:
 * string
 *
 * Each set has functions for:
 * Decoding the value string to the internal representation.
 * Encoding the internal attribute form to external form
 * Setting the value by =, + or - operators.
 * Comparing a (decoded) value with the attribute value.
 *
 * Some or all of the functions for an attribute type may be shared with
 * other attribute types.
 *
 * The prototypes are declared in "attribute.h"
 *
 * -------------------------------------------------
 * Set of general attribute functions for attributes
 * with value type "string"
 * -------------------------------------------------
 */

/*
 * decode_str - decode string into string attribute
 *
 * Returns: 0 if 0k
 *  >0 error number if error
 *  *patr members set
 */

int decode_str(

  attribute *patr,   /* (I modified, allocated ) */
  char      *name,   /* (I - optional) attribute name */
  char      *rescn,  /* resource name - unused here */
  char      *val,    /* attribute value */
  int        perm)   /* only used for resources */

  {
  size_t len;

  if (patr->at_val.at_str != NULL)
    {
    free(patr->at_val.at_str);

    patr->at_val.at_str = NULL;
    }

  if ((val != NULL) && ((len = strlen(val) + 1) > 1))
    {
    patr->at_val.at_str = calloc(1, (unsigned)len);

    if (patr->at_val.at_str == NULL)
      {
      /* FAILURE - cannot allocate memory */

      return(PBSE_SYSTEM);
      }

    strcpy(patr->at_val.at_str, val);

    patr->at_flags |= ATR_VFLAG_SET | ATR_VFLAG_MODIFY;
    }
  else
    {
    /* string is empty */

    patr->at_flags = (patr->at_flags & ~ATR_VFLAG_SET) | ATR_VFLAG_MODIFY;
    patr->at_val.at_str = NULL;
    }

  /* SUCCESS */

  return(0);
  }  /* END decode_str() */




/*
 * encode_str - encode attribute of type ATR_TYPE_STR into attr_extern
 *
 * Returns: >0 if ok
 *   =0 if no value, so no link added to list
 *   <0 if error
 */

/*ARGSUSED*/

int encode_str(

  attribute  *attr,    /* ptr to attribute */
  tlist_head *phead,   /* head of attrlist */
  char       *atname,  /* name of attribute */
  char       *rsname,  /* resource name or null */
  int         mode,    /* encode mode, unused here */
  int         perm)    /* only used for resources */

  {
  svrattrl *pal;
  int len = 0;

  if (attr == NULL)
    {
    return(-1);
    }

  if (!(attr->at_flags & ATR_VFLAG_SET) ||
      !attr->at_val.at_str ||
      (*attr->at_val.at_str == '\0'))
    {
    return(0);
    }

  len = strlen(attr->at_val.at_str);

  pal = attrlist_create(atname, rsname, len + 1);

  if (pal == NULL)
    {
    return(-1);
    }

  strncpy(pal->al_value, attr->at_val.at_str, len);

  pal->al_flags = attr->at_flags;

  append_link(phead, &pal->al_link, pal);

  return(1);
  }




/*
 * set_str - set attribute value based upon another
 *
 * A+B --> B is concatenated to end of A
 * A=B --> A is replaced with B
 * A-B --> If B is a substring at the end of A, it is stripped off
 *
 * Returns: 0 if ok
 *  >0 if error
 */

int set_str(
    
  struct attribute *attr,
  struct attribute *new,
  enum batch_op     op)

  {
  char *new_value;
  char *p;
  size_t nsize;

  assert(attr && new && new->at_val.at_str && (new->at_flags & ATR_VFLAG_SET));
  nsize = strlen(new->at_val.at_str) + 1; /* length of new string */

  if ((op == INCR) && (attr->at_val.at_str == NULL))
    op = SET; /* no current string, change INCR to SET */

  switch (op)
    {

    case SET: /* set is replace old string with new */

      if ((new_value = calloc(1, nsize)) == NULL)
        return (PBSE_SYSTEM);

      if (attr->at_val.at_str)
        (void)free(attr->at_val.at_str);
      attr->at_val.at_str = new_value;

      (void)strcpy(attr->at_val.at_str, new->at_val.at_str);

      break;

    case INCR: /* INCR is concatenate new to old string */

      nsize += strlen(attr->at_val.at_str);
      new_value = calloc(1, nsize + 1);

      if (new_value == NULL)
        return (PBSE_SYSTEM);

      strcat(new_value, attr->at_val.at_str);
      strcat(new_value, new->at_val.at_str);

      free(attr->at_val.at_str);
      attr->at_val.at_str = new_value;

      break;

    case DECR: /* DECR is remove substring if match, start at end */

      if (attr->at_val.at_str == NULL)
        break;

      if (--nsize == 0)
        break;

      p = attr->at_val.at_str + strlen(attr->at_val.at_str) - nsize;

      while (p >= attr->at_val.at_str)
        {
        if (strncmp(p, new->at_val.at_str, (int)nsize) == 0)
          {
          do
            {
            *p = *(p + nsize);
            }
          while (*p++);
          }

        p--;
        }

      break;

    default:
      return (PBSE_INTERNAL);
    }

  if ((attr->at_val.at_str != (char *)0) && (*attr->at_val.at_str != '\0'))
    attr->at_flags |= ATR_VFLAG_SET | ATR_VFLAG_MODIFY;
  else
    attr->at_flags &= ~ATR_VFLAG_SET;

  return (0);
  }




/*
 * comp_str - compare two attributes of type ATR_TYPE_STR
 *
 * Returns: see strcmp(3)
 */

int
comp_str(struct attribute *attr, struct attribute *with)
  {
  if (!attr || !attr->at_val.at_str)
    return (-1);

  return (strcmp(attr->at_val.at_str, with->at_val.at_str));
  }




/*
 * free_str - free space calloc-ed for string attribute value
 */

void free_str(

  struct attribute *attr)

  {
  if ((attr->at_flags & ATR_VFLAG_SET) && (attr->at_val.at_str != NULL))
    {
    free(attr->at_val.at_str);
    }

  attr->at_val.at_str = NULL;

  attr->at_flags &= ~ATR_VFLAG_SET;

  return;
  }  /* END free_str() */



/*
 * replace_attr_string - replace string attribute value
 */

void replace_attr_string(

  struct attribute *attr,
  char *newval)

  {
  free_str(attr);

  attr->at_val.at_str = newval;
  attr->at_flags |= ATR_VFLAG_SET;

  return;
  }  /* END replace_job_attr_string() */


/* END attr_fn_str.c */

/* END attr_fn_str.c */

