/* ----------------------------- MNI Header -----------------------------------
@NAME       : open_connection.c
@DESCRIPTION: File containing routines to open a decnet connection.
@GLOBALS    : 
@CREATED    : November 22, 1993 (Peter Neelin)
@MODIFIED   : $Log: open_connection.c,v $
@MODIFIED   : Revision 1.1  1997-03-04 20:56:47  neelin
@MODIFIED   : Initial revision
@MODIFIED   :
 * Revision 3.0  1995/05/15  19:31:44  neelin
 * Release of minc version 0.3
 *
 * Revision 2.5  1995/02/14  18:12:26  neelin
 * Added project names and defaults files (using volume name).
 * Added process id to log file name.
 * Moved temporary files to subdirectory.
 *
 * Revision 2.4  1995/02/09  13:51:26  neelin
 * Mods for irix 5 lint.
 *
 * Revision 2.3  1995/02/08  19:31:47  neelin
 * Moved ARGSUSED statements for irix 5 lint.
 *
 * Revision 2.2  1994/12/07  09:45:59  neelin
 * Fixed called to ioctl to get rid of type mismatch warning messages.
 *
 * Revision 2.1  94/12/07  08:20:10  neelin
 * Added support for irix 5 decnet.
 * 
 * Revision 2.0  94/09/28  10:35:32  neelin
 * Release of minc version 0.2
 * 
 * Revision 1.5  94/09/28  10:34:50  neelin
 * Pre-release
 * 
 * Revision 1.4  94/01/18  14:23:41  neelin
 * Changed bzero to memset.
 * 
 * Revision 1.3  93/11/30  14:42:13  neelin
 * Copies to minc format.
 * 
 * Revision 1.2  93/11/25  13:26:55  neelin
 * Working version.
 * 
 * Revision 1.1  93/11/23  14:11:54  neelin
 * Initial revision
 * 
@COPYRIGHT  :
              Copyright 1993 Peter Neelin, McConnell Brain Imaging Centre, 
              Montreal Neurological Institute, McGill University.
              Permission to use, copy, modify, and distribute this
              software and its documentation for any purpose and without
              fee is hereby granted, provided that the above copyright
              notice appear in all copies.  The author and McGill University
              make no representations about the suitability of this
              software for any purpose.  It is provided "as is" without
              express or implied warranty.
---------------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <dicomserver.h>

/* ----------------------------- MNI Header -----------------------------------
@NAME       : connection_okay
@INPUT      : sockfd - input file descriptor which might be a socket
@OUTPUT     : (none)
@RETURNS    : TRUE if connection is okay, FALSE otherwise
@DESCRIPTION: Checks whether the connection is allowed. Looks at sockfd
              to find out if remote host is allowed to connect. If sockfd
              is a file and not a socket, then the connection is allowed.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : February 20, 1997 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
private int connection_okay(int sockfd)
{
   struct sockaddr_in us, them;
   int status;
   int namelen;
   extern int Do_logging;

   /* Get our own id. If sockfd is a file, then its okay. Check that we
      have an internet connection. */
   namelen = sizeof(us);
   if (getsockname(sockfd, &us, &namelen) != 0) {
      if (errno == ENOTSOCK) 
         return TRUE;
      else {
         (void) fprintf(stderr, "Unable to get our own host address.\n");
         return FALSE;
      }
   }
   else if (us.sin_family != AF_INET) {
      (void) fprintf(stderr, "Connection is not from network.\n");
      return FALSE;
   }

   /* Try to get id of host at other end of connection */
   namelen = sizeof(us);
   status = getpeername(sockfd, &them, &namelen);
   if (status != 0) {
      (void) fprintf(stderr, "Unable to check connection source.\n");
      return FALSE;
   }

   /* */
   if (Do_logging >= LOW_LOGGING) {
      (void) fprintf(stderr, "Connection from %s ", inet_ntoa(them.sin_addr));
   }

   /* Compare the addresses. Make sure that we have the same IP domain
      assuming class C structure. */
   if ((us.sin_addr.s_addr & IN_CLASSC_NET) != 
       (them.sin_addr.s_addr & IN_CLASSC_NET)) {
      if (Do_logging >= LOW_LOGGING) {
         (void) fprintf(stderr, "refused.\n");
      }
      return FALSE;
   }

   if (Do_logging >= LOW_LOGGING) {
      (void) fprintf(stderr, "accepted.\n");
   }

   return TRUE;
}

/* ----------------------------- MNI Header -----------------------------------
@NAME       : open_connection
@INPUT      : argc - number of command-line arguments
              argv - array of command-line arguments
@OUTPUT     : afpin - Acr file pointer for input
              afpout - Acr file pointer for output
@RETURNS    : (nothing)
@DESCRIPTION: Opens the connection for reading writing dicom messages.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : November 22, 1993 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
/* ARGSUSED */
public void open_connection(int argc, char *argv[], 
                            Acr_File **afpin, Acr_File **afpout)
{
   /* Set default file pointers */
   *afpin = *afpout = NULL;

   /* Check for a valid connection */
   if (!connection_okay(fileno(stdin))) return;

   /* Open the connection */
   *afpin=acr_initialize_dicom_input(stdin, 0, acr_stdio_read);
   *afpout=acr_initialize_dicom_output(stdout, 0, acr_stdio_write);

   /* Ignore SIGPIPE errors in case connection gets closed when we are
      doing output */
   (void) signal(SIGPIPE, SIG_IGN);
}

