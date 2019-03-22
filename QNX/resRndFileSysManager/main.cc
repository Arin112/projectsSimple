#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <string>

template<typename T>
inline T min(T a, T b){
    return (a<b)?a:b;
}

#define ALIGN(x) (((x) + 3) & ~3)
#define NUM_ENTS            2

static  iofunc_attr_t   atoz_attrs [NUM_ENTS];

struct dirent *
dirent_fill (struct dirent *dp, int inode, int offset,
             char *fname)
{
    dp -> d_ino = inode;
    dp -> d_offset = offset;
    strcpy (dp -> d_name, fname);
    dp -> d_namelen = strlen (dp -> d_name);
    dp -> d_reclen = ALIGN (sizeof (struct dirent) - 4
                   + dp -> d_namelen + 1);
    return ((struct dirent *) ((char *) dp +
            dp -> d_reclen));
}

int
dirent_size (char *fname)
{
  return (ALIGN (sizeof (struct dirent) - 4 + strlen (fname) + 1));
}

static int
my_read_file (resmgr_context_t *ctp, io_read_t *msg,
              iofunc_ocb_t *ocb)
{
    int     nbytes;
    int     nleft;
    char    string;

    // we don't do any xtypes here...
    if ((msg -> i.xtype & _IO_XTYPE_MASK) !=
         _IO_XTYPE_NONE) {
        return (ENOSYS);
    }

    // figure out how many bytes are left
    nleft = ocb -> attr -> nbytes - ocb -> offset;

    // and how many we can return to the client
    //nbytes = min (nleft, msg -> i.nbytes);

    if (1) {
        // create the output string
        //string = ocb -> attr -> inode - 1 + 'A';
        static char buf[200];
        int n;
        if(ocb -> attr -> inode - 1 == 0){
            n=sprintf(buf,"%d ", rand()%100);
        }else{
            n=sprintf(buf, "%lf ", (rand()%10000)/100.);
        }
        // return it to the client
        MsgReply (ctp -> rcvid, n,
                  buf,
                  n);

        // update flags and offset
        ocb -> attr -> flags |= IOFUNC_ATTR_ATIME
                             | IOFUNC_ATTR_DIRTY_TIME;
        //ocb -> offset += nbytes;
    } else {
        // nothing to return, indicate End Of File
        MsgReply (ctp -> rcvid, EOK, NULL, 0);
    }

    // already done the reply ourselves
    return (_RESMGR_NOREPLY);
}

static int
my_read_dir (resmgr_context_t *ctp, io_read_t *msg,
             iofunc_ocb_t *ocb)
{
    int     nbytes;
    int     nleft;
    struct  dirent *dp;
    char    *reply_msg;
    char    fname [_POSIX_PATH_MAX];

    // allocate a buffer for the reply
    reply_msg = (char*)calloc (1, msg -> i.nbytes);
    if (reply_msg == NULL) {
        return (ENOMEM);
    }

    // assign output buffer
    dp = (struct dirent *) reply_msg;

    // we have "nleft" bytes left
    nleft = msg -> i.nbytes;
    while (ocb -> offset < NUM_ENTS) {

        // create the filename
        std::string names[] = {"int", "double"};
        sprintf (fname, "%s", names[ocb -> offset].c_str());

        // see how big the result is
        nbytes = dirent_size (fname);

        // do we have room for it?
        if (nleft - nbytes >= 0) {

            // fill the dirent, and advance the dirent pointer
            dp = dirent_fill (dp, ocb -> offset + 1,
                              ocb -> offset, fname);

            // move the OCB offset
            ocb -> offset++;

            // account for the bytes we just used up
            nleft -= nbytes;
        } else {

            // don't have any more room, stop
            break;
        }
    }

    // return info back to the client
    MsgReply (ctp -> rcvid, (char *) dp - reply_msg,
              reply_msg, (char *) dp - reply_msg);

    // release our buffer
    free (reply_msg);

    // tell resource manager library we already did the reply
    return (_RESMGR_NOREPLY);
}

static int
my_open (resmgr_context_t *ctp, io_open_t *msg,
         iofunc_attr_t *attr, void *extra)
{
    // an empty path means the directory, is that what we have?
    if (msg -> connect.path [0] == 0) {
        return (iofunc_open_default (ctp, msg, attr, extra));

    // else check if it's a single char 'a' -> 'z'
    } else if ( std::string(msg -> connect.path) == "int" ||
                std::string(msg -> connect.path) == "double") {

        // yes, that means it's the file (/dev/atoz/[a-z])
        return (iofunc_open_default (ctp, msg,
                atoz_attrs + ((std::string(msg -> connect.path) == "int")?0:1),
                extra));
    } else {
        return (ENOENT);
    }
}

static int
my_read (resmgr_context_t *ctp, io_read_t *msg,
         iofunc_ocb_t *ocb)
{
    int     sts;

    // use the helper function to decide if valid
    if ((sts = iofunc_read_verify (ctp, msg, ocb,
                                   NULL)) != EOK) {
        return (sts);
    }

    // decide if we should perform the "file" or "dir" read
    if (S_ISDIR (ocb -> attr -> mode)) {
        return (my_read_dir (ctp, msg, ocb));
    } else if (S_ISREG (ocb -> attr -> mode)) {
        return (my_read_file (ctp, msg, ocb));
    } else {
        return (EBADF);
    }
}

int
main (int argc, char **argv)
{
    dispatch_t              *dpp;
    resmgr_attr_t           resmgr_attr;
    dispatch_context_t      *ctp;
    resmgr_connect_funcs_t  connect_func;
    resmgr_io_funcs_t       io_func;
    iofunc_attr_t           attr;
    int                     i;

    // create the dispatch structure
    if ((dpp = dispatch_create ()) == NULL) {
        perror ("Unable to dispatch_create\n");
        exit (EXIT_FAILURE);
    }

    // initialize the various data structures
    memset (&resmgr_attr, 0, sizeof (resmgr_attr));
    resmgr_attr.nparts_max = 1;
    resmgr_attr.msg_max_size = 2048;

    // bind default functions into the outcall tables
    iofunc_func_init (_RESMGR_CONNECT_NFUNCS, &connect_func,
                      _RESMGR_IO_NFUNCS, &io_func);

    // create and initialize the attributes structure
    // for the directory.  Inodes 1-26 are reserved for the
    // files 'a' through 'z'.  The number of bytes is 26
    // because that's how many entries there are.
    iofunc_attr_init (&attr, S_IFDIR | 0555, 0, 0);
    attr.inode = NUM_ENTS + 1;
    attr.nbytes = NUM_ENTS;

    // and for the "a" through "z" names
    for (i = 0; i < NUM_ENTS; i++) {
        iofunc_attr_init (&atoz_attrs [i],
                          S_IFREG | 0444, 0, 0);
        atoz_attrs [i].inode = i + 1;
        atoz_attrs [i].nbytes = 1;
    }

    // add our functions; we're interested only in
    // io_open and io_read
    connect_func.open = my_open;
    io_func.read = my_read;

    // establish a name in the pathname space
    if (resmgr_attach (dpp, &resmgr_attr, "/dev/myRand",
                       _FTYPE_ANY, _RESMGR_FLAG_DIR,
                       &connect_func, &io_func,
                       &attr) == -1) {
        perror ("Unable to resmgr_attach\n");
        exit (EXIT_FAILURE);
    }

    // allocate a context
    ctp = dispatch_context_alloc (dpp);

    // wait here forever, handling messages
    while (1) {
        if ((ctp = dispatch_block (ctp)) == NULL) {
            perror ("Unable to dispatch_block\n");
            exit (EXIT_FAILURE);
        }
        dispatch_handler (ctp);
    }

    // you'll never get here
    return (EXIT_SUCCESS);
}
