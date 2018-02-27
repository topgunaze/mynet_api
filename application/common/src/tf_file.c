/**************************************************************
 * 文件名称:  tf_file.c reference to busybox
 * 作           者:  steven.tian
 * 日           期:  2017/02/27
 * 文件描述:  file operate lib
 * 版           本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
**************************************************************/

#include <errno.h>
#include <sys/mman.h>
#include "tf_file.h"

size_t safe_read(int fd, void *buf, size_t count)
{
	size_t n;

	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

size_t safe_write(int fd, const void *buf, size_t count)
{
	size_t n;

	do {
		n = write(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

/*
 * Read all of the supplied buffer from a file.
 * This does multiple reads as necessary.
 * Returns the amount read, or -1 on an error.
 * A short read is returned on an end of file.
 */
size_t full_read(int fd, void *buf, size_t len)
{
	size_t cc;
	size_t total;

	total = 0;

	while (len) {
		cc = safe_read(fd, buf, len);

		if (cc < 0) {
			if (total) {
				/* we already have some! */
				/* user can do another read to know the error code */
				return total;
			}
			return cc; /* read() returns -1 on failure. */
		}
		if (cc == 0)
			break;
		buf = ((char *)buf) + cc;
		total += cc;
		len -= cc;
	}

	return total;
}

/*
 * Write all of the supplied buffer out to a file.
 * This does multiple writes as necessary.
 * Returns the amount written, or -1 on an error.
 */
size_t full_write(int fd, const void *buf, size_t len)
{
	size_t cc;
	size_t total;

	total = 0;

	while (len) {
		cc = safe_write(fd, buf, len);

		if (cc < 0) {
			if (total) {
				/* we already wrote some! */
				/* user can do another write to know the error code */
				return total;
			}
			return cc;  /* write() returns -1 on failure. */
		}

		total += cc;
		buf = ((const char *)buf) + cc;
		len -= cc;
	}

	return total;
}

/* Used by NOFORK applets (e.g. cat) - must not use xmalloc.
 * size < 0 means "ignore write errors", used by tar --to-command
 * size = 0 means "copy till EOF"
 */
size_t full_fd_copy(int src_fd, int dst_fd, size_t size)
{
	int status = -1;
	size_t total = 0;
	int continue_on_write_error = 0;
#if CONFIG_FEATURE_COPYBUF_KB <= 4
	char buffer[CONFIG_FEATURE_COPYBUF_KB * 1024];
	enum { buffer_size = sizeof(buffer) };
#else
	char *buffer;
	int buffer_size;
#endif

	if (size < 0) {
		size = -size;
		continue_on_write_error = 1;
	}

#if CONFIG_FEATURE_COPYBUF_KB > 4
	if (size > 0 && size <= 4 * 1024)
		goto use_small_buf;
	/* We want page-aligned buffer, just in case kernel is clever
	 * and can do page-aligned io more efficiently */
	buffer = mmap(NULL, CONFIG_FEATURE_COPYBUF_KB * 1024,
			PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANON,
			/* ignored: */ -1, 0);
	buffer_size = CONFIG_FEATURE_COPYBUF_KB * 1024;
	if (buffer == MAP_FAILED) {
 use_small_buf:
		buffer = alloca(4 * 1024);
		buffer_size = 4 * 1024;
	}
#endif

	if (src_fd < 0)
		goto out;

	if (!size) {
		size = buffer_size;
		status = 1; /* copy until eof */
	}

	while (1) {
		size_t rd;

		rd = safe_read(src_fd, buffer, size > buffer_size ? buffer_size : size);

		if (!rd) { /* eof - all done */
			status = 0;
			break;
		}
		if (rd < 0) {
			tf_file_err("read error%s\n", __func__);
			break;
		}
		/* dst_fd == -1 is a fake, else... */
		if (dst_fd >= 0) {
			size_t wr = full_write(dst_fd, buffer, rd);
			if (wr < rd) {
				if (!continue_on_write_error) {
					tf_file_err("write error\n");
					break;
				}
				dst_fd = -1;
			}
		}
		total += rd;
		if (status < 0) { /* if we aren't copying till EOF... */
			size -= rd;
			if (!size) {
				/* 'size' bytes copied - all done */
				status = 0;
				break;
			}
		}
	}
 out:

#if CONFIG_FEATURE_COPYBUF_KB > 4
	if (buffer_size != 4 * 1024)
		munmap(buffer, buffer_size);
#endif
	return status ? -1 : total;
}

/* Used by NOFORK applets (e.g. cat) - must not use xmalloc.
 * size < 0 means "ignore write errors", used by tar --to-command
 * size = 0 means "copy till EOF"
 */
size_t full_file_copy(FILE *src, FILE *dst, size_t size)
{
	int src_fd = -1;
	int dst_fd = -1;

	if (!src || !dst)
	{
	    return -1;
	}

	src_fd = fileno(src);
	dst_fd = fileno(dst);

	if ((src_fd < 0) || (dst_fd < 0))
	{
	    tf_file_err("File stream is invalid!\n");
	    return -1;
	}

	return full_fd_copy(src_fd, dst_fd, size);
}

int find_mtd_dev_by_name(char *name, char *mtd, int len)
{
    FILE *fp = NULL;
    int   ret = -1;
    char  lineBuf[STACK_LINE_BUF_MAX_LEN] = {0};
    char  mtdName[MTD_NAME_MAX_LEN] = {0};
    int   mtdNum = 0;
    int   count = 0;

    do
    {
        if (!name || !mtd)
        {
            tf_file_err("Invalid parameters!\n");
            break;
        }

        fp = fopen(PROC_MTD_FILE_NAME, "r+");
        if (!fp)
        {
            tf_file_err("Open %s failed!\n", PROC_MTD_FILE_NAME);
            break;
        }

        while(NULL != fgets(lineBuf, sizeof(lineBuf), fp))
        {
            memset(mtdName, 0, sizeof(mtdName));
            mtdNum = 0;
            
            if((count = sscanf(lineBuf, "mtd%d: %*s %*s \"%[^\"]\"", &mtdNum, mtdName)) == 2)
            {
                //tf_file_err("mtdNum=%d mtdName=%s\n", mtdNum, mtdName);
                if (0 == strcmp(mtdName, name))
                {
                    snprintf(mtd, len, "/dev/mtd%d", mtdNum);
                    tf_file_err("Find line: %s\n", lineBuf);
                    ret = 0;
                    break;
                }
            }
            memset(lineBuf, 0, sizeof(lineBuf));
        }

        if (0 != ret)
            tf_file_err("\"%s\" is not find in %s!\n", name, PROC_MTD_FILE_NAME);
        
    }while(0);

    if (fp)
    {
        fclose(fp);
        fp = NULL;
    }

    return ret;
}

