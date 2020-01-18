#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <iostream>


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

using namespace std;



void check_controls(int fd) {

    struct v4l2_queryctrl qctrl;

    memset(&qctrl, 0, sizeof(qctrl));

    qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
    while (0 == ioctl (fd, VIDIOC_QUERYCTRL, &qctrl)) {

        printf("ID = %08x\n", qctrl.id);
        /* ... */
        qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }
}




int main()
{
    int fd = open("/dev/video0", O_RDWR);
    if (fd == -1) 
    {   
        perror("open: "); return 1; 
    }


	check_controls(fd);
	
    {   
        // first option
        v4l2_jpegcompression ctrl={0};
        if(ioctl(fd, VIDIOC_G_JPEGCOMP, &ctrl)<0)
        {
            perror("VIDIOC_G_JPEGCOMP:");
        }
        else
        {
            cout<<"QUALITY:"<<ctrl.quality<<endl; 
        }
    }

    {   
        // second option
        v4l2_ext_control extCtrl={0};
        extCtrl.id = V4L2_CID_JPEG_COMPRESSION_QUALITY;
        extCtrl.size = 0;
        extCtrl.value = 100;

        v4l2_ext_controls extCtrls={0};
        extCtrls.controls = &extCtrl;
        extCtrls.count = 1;
        extCtrls.ctrl_class = V4L2_CTRL_CLASS_JPEG;

        if(ioctl(fd, VIDIOC_G_EXT_CTRLS, &extCtrls)<0)
        { 
             perror("VIDIOC_G_EXT_CTRLS:");
        }
    }

    {   
        // third option
        v4l2_ext_control extCtrl={0};
        extCtrl.id = V4L2_CID_JPEG_COMPRESSION_QUALITY;
        extCtrl.size = 0;
        extCtrl.value = 100;

        v4l2_ext_controls extCtrls={0};
        extCtrls.controls = &extCtrl;
        extCtrls.count = 1;
        extCtrls.ctrl_class = V4L2_CID_JPEG_CLASS;

        if(ioctl(fd, VIDIOC_G_EXT_CTRLS, &extCtrls)<0)
        { 
            perror("VIDIOC_G_EXT_CTRLS V4L2_CID_JPEG_CLASS:");
        }
    }

    close(fd);
    return 0;
}
