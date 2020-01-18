/*
 * =====================================================================================
 *
 *       Filename:  camera_jpg.c
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <sys/ioctl.h>

#define CLEAR(x) memset(&(x),0,sizeof(x))
#define MAX_WIDTH (640)
#define MAX_HEIGHT (480)
#define MAX_FPS 	(30)

int main()
{
	int ret;
	int camera_fd;

	camera_fd = open("/dev/video0",O_RDWR);

	struct v4l2_fmtdesc fmt;
	memset(&fmt,0x0,sizeof(fmt));
	fmt.index = 0;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	while ((ret = ioctl(camera_fd, VIDIOC_ENUM_FMT, &fmt)) == 0) {
		fmt.index++;
		printf("{ pixelformat = ''%c%c%c%c'', description = ''%s'' }\n",\
				fmt.pixelformat & 0xFF, (fmt.pixelformat >> 8) & 0xFF, \
				(fmt.pixelformat >> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF, \
				fmt.description);
	}

	struct v4l2_capability cap; 
	ret = ioctl(camera_fd, VIDIOC_QUERYCAP, &cap);
	if(ret < 0)
	{
		printf("get vidieo capability error,error code: %d \n", errno);
		return -1;
	}
	printf("{ Capability: driver:'%s', card:'%s',buf_info:'%s',version:%d,capabilities:0x%x}\n",\
			cap.driver,cap.card,cap.bus_info,cap.version,cap.capabilities);	

	struct v4l2_format tv4l2_format; 
	CLEAR(tv4l2_format);
	tv4l2_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
	tv4l2_format.fmt.pix.width = MAX_WIDTH; 
	tv4l2_format.fmt.pix.height = MAX_HEIGHT; 
	tv4l2_format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG; 
	tv4l2_format.fmt.pix.field = V4L2_FIELD_NONE; // V4L2_FIELD_INTERLACED  //  V4L2_FIELD_TOP
	ret = ioctl(camera_fd, VIDIOC_S_FMT, &tv4l2_format);
	if(ret < 0)
	{
		perror("VIDIOC_S_FMT");
	}
	printf("{ Format width: %d, height:%d}\n",tv4l2_format.fmt.pix.width,tv4l2_format.fmt.pix.height);

	struct v4l2_streamparm *setfps;
	setfps = (struct v4l2_streamparm *)calloc(1,sizeof(struct v4l2_streamparm));
	memset(setfps,0,sizeof(struct v4l2_streamparm));
	setfps->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	setfps->parm.capture.timeperframe.numerator = 1;
	setfps->parm.capture.timeperframe.denominator = 30;
	if(ioctl(camera_fd, VIDIOC_S_PARM,setfps) < 0)
	{
		perror("VIDIOC_S_PARM");
	}

	struct v4l2_requestbuffers v4l2_reqbuf;
	memset(&v4l2_reqbuf,0,sizeof(struct v4l2_requestbuffers));
	v4l2_reqbuf.count = 1;
	v4l2_reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_reqbuf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(camera_fd, VIDIOC_REQBUFS, &v4l2_reqbuf);
	if(ret < 0)
		perror("VIDIOC_REQBUFS");
	printf("{ Reqbuffer count: %d}\n",v4l2_reqbuf.count);

	struct v4l2_buffer map_buffer;
	map_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	map_buffer.index = 0;
	map_buffer.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(camera_fd,VIDIOC_QUERYBUF,&map_buffer);
	if(ret < 0)
		perror("VIDIOC_QUERYBUF");
	printf("{ Querybuffer length:%d, m.offset: %d}\n",map_buffer.length,map_buffer.m.offset);

	void *map_address = mmap(NULL, \
			map_buffer.length, \
			PROT_READ | PROT_WRITE ,\
			MAP_SHARED, \
			camera_fd, map_buffer.m.offset);
	if(map_address == MAP_FAILED){
		printf("mmap failed!\n");
		exit(1);
	}

	struct v4l2_buffer camera_buf;
	camera_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	camera_buf.memory = V4L2_MEMORY_MMAP;
	camera_buf.index = 0;
	if(-1 == ioctl(camera_fd, VIDIOC_QBUF, &camera_buf))
	{
		perror("VIDIOC_QBUF");
		exit(EXIT_FAILURE);
	}

	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == ioctl(camera_fd,VIDIOC_STREAMON,&type))
	{
		perror("VIDIOC_STREAMON");
		exit(EXIT_FAILURE);
	}

	fd_set fds;
	struct timeval tv;
	int camera_index;
	char *filename = malloc(20*sizeof(char));
	for(camera_index = 0;camera_index < MAX_FPS;camera_index++)
	{
		FD_ZERO(&fds);
		FD_SET(camera_fd, &fds);
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		ret = select(camera_fd+1,&fds,NULL,NULL,&tv);
		if(ret == -1)
		{
			perror("select");
			exit(EXIT_FAILURE);
		}
		if(ret == 0)
		{
			fprintf(stderr,"select time out\n");
			exit(EXIT_FAILURE);
		}

		struct v4l2_buffer get_buffer;
		get_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		get_buffer.memory = V4L2_MEMORY_MMAP;
		if(-1 == ioctl(camera_fd,VIDIOC_DQBUF,&get_buffer))
		{
			perror("VIDIOC_DQBUF");
			exit(EXIT_FAILURE);
		}

		printf("{ Get Buffer OK index:%d length: %d time: %ld:%ld}\n",get_buffer.index,get_buffer.length,get_buffer.timestamp.tv_sec,get_buffer.timestamp.tv_usec);

		sprintf(filename,"./camera-%d.jpg",camera_index);	

		//*************************************
		//MJPG
		//*************************************
		FILE * file;
		if((file = fopen(filename,"wb")) == NULL){
		printf("open file failed\n");
		exit(EXIT_FAILURE);
		}

		fwrite(map_address,get_buffer.length,1,file);
		fclose(file);

		if(-1 == ioctl(camera_fd,VIDIOC_QBUF,&get_buffer))
		{
			perror("VIDIOC_QBUF");
			exit(EXIT_FAILURE);
		}
	}

	if(-1 == munmap(map_address,map_buffer.length))
	{
		perror("munmap");	
		exit(EXIT_FAILURE);
	}
	printf("munmap,then exit\n");

	return 0;

}
