#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <iostream>
using namespace std;

int main()
{
    int fd = open("/dev/video0", O_RDWR);
    if (fd == -1) 
    {   
        perror("open: "); return 1; 
    }

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
