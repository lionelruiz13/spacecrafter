/* V4L2 video picture grabber
   Copyright (C) 2009 Mauro Carvalho Chehab <mchehab@kernel.org>
*/

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


#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer {
        void   *start;
        size_t length;
};

static void xioctl(int fh, int request, void *arg)
{
        int r;

        do {
                r = v4l2_ioctl(fh, request, arg);
        } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

        if (r == -1) {
                fprintf(stderr, "error %d, %s\\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}

int main(int argc, char **argv)
{
        struct v4l2_format              fmt;
        struct v4l2_buffer              buf;
        struct v4l2_requestbuffers      req;
        enum v4l2_buf_type              type;
        fd_set                          fds;
        struct timeval                  tv;
        int                             r, fd = -1;
        unsigned int                    i, n_buffers;
        char                            *dev_name = "/dev/video0";
        char                            out_name[256];
        FILE                            *fout;
        struct buffer                   *buffers;

        int                             size_x=640 , size_y= 480;

        fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
        if (fd < 0) {
                perror("Cannot open device");
                exit(EXIT_FAILURE);
        }

		//preparation à l'interrogation de la webcam
        CLEAR(fmt);
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width       = size_x;
        fmt.fmt.pix.height      = size_y;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
        fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

		//interrogation de la webcam
        xioctl(fd, VIDIOC_S_FMT, &fmt);

		//analyse des infos de la webcam
        if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24) {
                printf("Libv4l didn't accept RGB24 format. Can't proceed.\\n");
                exit(EXIT_FAILURE);
        }
        if ((fmt.fmt.pix.width != size_x) || (fmt.fmt.pix.height != size_y))
			printf("Warning: driver is sending image at %dx%d\\n", fmt.fmt.pix.width, fmt.fmt.pix.height);

		// interrogation de req, préparation des buffers
        CLEAR(req);
        req.count = 2;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;
        xioctl(fd, VIDIOC_REQBUFS, &req);

        buffers = calloc(req.count, sizeof(*buffers));
        
        printf("Nombre de buffers effectifs : %i\n", req.count);

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                CLEAR(buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = n_buffers;

                xioctl(fd, VIDIOC_QUERYBUF, &buf);

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start = v4l2_mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,fd, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start) {
                        perror("mmap");
                        exit(EXIT_FAILURE);
                }
        }

        for (i = 0; i < n_buffers; ++i) {
                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;
                xioctl(fd, VIDIOC_QBUF, &buf);
        }
        

        // poiur modifier 
struct v4l2_queryctrl queryctrl;
struct v4l2_control control;


//~ //premier test
//~ memset(&queryctrl, 0, sizeof(queryctrl));
//~ queryctrl.id = V4L2_CID_BRIGHTNESS;

//~ if (-1 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {
    //~ if (errno != EINVAL) {
        //~ perror("VIDIOC_QUERYCTRL");
        //~ exit(EXIT_FAILURE);
    //~ } else {
        //~ printf("V4L2_CID_BRIGHTNESS is not supportedn");
    //~ }
//~ } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    //~ printf("V4L2_CID_BRIGHTNESS is not supportedn");
//~ } else {
    //~ memset(&control, 0, sizeof (control));
    //~ control.id = V4L2_CID_BRIGHTNESS;
    //~ control.value = 100;

    //~ if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control)) {
        //~ perror("VIDIOC_S_CTRL");
        //~ exit(EXIT_FAILURE);
    //~ }
//~ }

//secodn test
memset(&control, 0, sizeof(control));
control.id = V4L2_CID_CONTRAST;

if (0 == ioctl(fd, VIDIOC_G_CTRL, &control)) {
    control.value = 95;

    /* The driver may clamp the value or return ERANGE, ignored here */

    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control)
        && errno != ERANGE) {
        perror("VIDIOC_S_CTRL");
        exit(EXIT_FAILURE);
    }
/* Ignore if V4L2_CID_CONTRAST is unsupported */
} else if (errno != EINVAL) {
    perror("VIDIOC_G_CTRL");
    exit(EXIT_FAILURE);
}


        //prepartion du type
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(fd, VIDIOC_STREAMON, &type);

        //boucle de capture
        for (i = 0; i < 30; i++) {
                do {
                        FD_ZERO(&fds);
                        FD_SET(fd, &fds);

                        /* Timeout. */
                        tv.tv_sec = 1;
                        tv.tv_usec = 0;

                        r = select(fd + 1, &fds, NULL, NULL, &tv);
                } while ((r == -1 && (errno = EINTR)));
                if (r == -1) {
                        perror("select");
                        return errno;
                }

				//recupération d'une frame de la webcam
                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                xioctl(fd, VIDIOC_DQBUF, &buf);

				//sauvegarde ou affichage ...
                sprintf(out_name, "out%03d.ppm", i);
                fout = fopen(out_name, "w");
                if (!fout) {
                        perror("Cannot open image");
                        exit(EXIT_FAILURE);
                }
                printf("Index du buffer : %i de taille %i\n", buf.index, buf.bytesused);
                fprintf(fout, "P6\n%d %d 255\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
                
                
                fwrite(buffers[buf.index].start, buf.bytesused, 1, fout);
                
                fclose(fout);

				// important, libère le driver pour capturer une nouvelle frame
                xioctl(fd, VIDIOC_QBUF, &buf);
        }
		
		// supersion des tampons 
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(fd, VIDIOC_STREAMOFF, &type);
        for (i = 0; i < n_buffers; ++i)
                v4l2_munmap(buffers[i].start, buffers[i].length);
        v4l2_close(fd);

        return 0;
}
