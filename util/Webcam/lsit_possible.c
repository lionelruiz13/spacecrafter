#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>

int main()
{
    int fd = v4l2_open("/dev/video0", O_RDWR);
    if (fd != -1)
    {
        struct v4l2_fmtdesc fmtdesc;
        memset(&fmtdesc,0,sizeof(fmtdesc));
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        while (ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc) == 0)
        {    
            printf("%s\n", fmtdesc.description);
            fmtdesc.index++;
        }
        v4l2_close(fd);
    }
}
