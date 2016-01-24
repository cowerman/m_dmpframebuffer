#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h> // /usr/include/linux/fb.h

#include "bmp.h"

#define DEV_PATH "/dev/graphics/fb0"
#define DEST_PATH "/data/local/tmp/out.raw"

typedef unsigned int u32;
typedef signed int s32;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned char u8;
typedef signed char s8;

#define RGB888(r, g, b) ( (r) << 16 | (g) << 8 | (b) )
#define RGB565(r, g, b) ( (r) >> 3 << 11 | (g) >> 2 << 5 | (b) >> 3 )

struct fb_info {
	struct fb_fix_screeninfo fix;
	struct fb_var_screeninfo var;
	int fd;
	u8 *addr; //mmap映射后得到的显存首地址
	size_t x, y; //包含可能存在的隐藏像素，真实的分辨率
	size_t pixel_size; //每个像素占多少字节
	void (*draw_pixel)(struct fb_info *, ssize_t x, ssize_t y, u32 color); //根据色深选择挂接对应的画点方法，其中color始终采用RGB888的格式
};
static struct fb_info fb;

static void draw_pixel_rgb565(struct fb_info *info, ssize_t x, ssize_t y, u32 color)
{
	*((u16*)info->addr + y * info->x + x) = RGB565((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
}

static void draw_pixel_rgb888(struct fb_info *info, ssize_t x, ssize_t y, u32 color)
{
	*((u32*)info->addr + y * info->x + x) = color;
}

static void draw_pixel_8bit(struct fb_info *info, ssize_t x, ssize_t y, u32 color)
{
	*((u8*)info->addr + y * info->x + x) = (u8)color;
}

static void draw_rect(struct fb_info *info, ssize_t x, ssize_t y, size_t xlen, size_t ylen, u32 color)
{
	size_t ix, iy;

	for (ix = x; ix < x + xlen; ++ix) {
		for (iy = y; iy < y + ylen; ++iy) {
			fb.draw_pixel(info, ix, iy, color);
		}
	}
}

static int show_bmp(struct fb_info *info, const char *bmp_path)
{
	FILE *fp = fopen(bmp_path, "r");
	if (!fp) {
		perror("fopen bmp");
		return -1;
	}

	//读出bmp文件头
	struct bmp_file bmp_head = {0};
	fread(&bmp_head, sizeof(bmp_head), 1, fp);

	//将bmp的文件操作指针定位到像素数据的入口
	fseek(fp, bmp_head.offset, SEEK_SET);

	u32 buf = 0; //存放一个像素颜色值的临时缓冲
	size_t ix, iy;
	for (iy = 0; iy < info->y; ++iy) {
		//定位文件中当前行所对应的像素入口
		fseek(fp, bmp_head.offset + (info->y - 1 - iy) * info->fix.line_length, SEEK_SET);
		for (ix = 0; ix < info->x; ++ix) {
			//读取一个像素
			fread(&buf, 4, 1, fp);
			buf >>= 8;
			info->draw_pixel(info, ix, iy, buf);
		}

	}

	fclose(fp);
}
/*
int  fileCopy(int src, int dest)  
{  
int len;
int buf[1536];
   while((len=read(src,buf, sizeof(buf))) > 0){
	  len = write(dest, buf, len);             
	  if (len < 0) {
		perror("writ to file erro");
		return 1;
	  }
   }
return 0;  
}  
*/
/*
int  fileCopy(int src, int dest)  
{  
int len;
int buf;
   while((len=read(src,&buf, sizeof(buf))) > 0){
	  len = write(dest, &buf, len);             
	  if (len < 0) {
		perror("writ to file erro");
		return 1;
	  }
   }
return 0;  
}  
*/
int main(int argc, char **argv)
{
	FILE *out, *in;
	int len;
	char buf[4096];

	
	 if((out = fopen(DEST_PATH, "w+")) == 0) {
		perror("open dest");
		return 1;
	 }
	 if((in = fopen(DEV_PATH, "r")) == 0) {
		perror("open sourc");
		return 1;
	 }

	//1 打开/dev/fb0
	fb.fd = open(DEV_PATH, O_RDWR);
	if (fb.fd < 0) {
		perror("open dev");
		goto err_open_dev;
	}


	//2 使用ioctl来获取驱动中的显示参数，并打印。根据得到的显示参数，选择正确的draw_pixel版本来挂接
	if (ioctl(fb.fd, FBIOGET_VSCREENINFO, &fb.var) < 0) {
		perror("ioctl get var");
		goto err_ioctl;
	}

	if (ioctl(fb.fd, FBIOGET_FSCREENINFO, &fb.fix) < 0) {
		perror("ioctl get fix");
		goto err_ioctl;
	}

	fb.pixel_size = fb.var.bits_per_pixel / 8;
	fb.x = fb.fix.line_length / fb.pixel_size;
	fb.y = fb.var.yres;

	while ((len = fread(buf, 1, sizeof buf, in)) > 0)
		    fwrite(buf, 1, len, out);
	                                                
	printf("write out.raw ok\n");
/*
	//挂接画点的操作方法
	switch (fb.pixel_size) {
	case 1:
		fb.draw_pixel = draw_pixel_8bit;
		break;
	case 2:
		fb.draw_pixel = draw_pixel_rgb565;
		break;
	case 4:
		fb.draw_pixel = draw_pixel_rgb888;
		break;
	default:
		fprintf(stderr, "err pixel_size %u\n", fb.pixel_size);
		break;
	}
*/
	//打印显示参数
	printf("xres=%u, yres=%u, xres_virtual=%u, yres_virtual=%u\n", fb.var.xres, fb.var.yres, fb.var.xres_virtual, fb.var.yres_virtual);
	printf("bits_per_pixel=%u, true_x=%u\n", fb.var.bits_per_pixel, fb.x);
	printf("red: offset=%u, lengh=%u, msb_right=%u\n", fb.var.red.offset, fb.var.red.length, fb.var.red.msb_right);
	printf("green: offset=%u, lengh=%u, msb_right=%u\n", fb.var.green.offset, fb.var.green.length, fb.var.green.msb_right);
	printf("blue: offset=%u, lengh=%u, msb_right=%u\n", fb.var.blue.offset, fb.var.blue.length, fb.var.blue.msb_right);
/*
	//3 mmap
	fb.addr = (u8*)mmap(NULL, fb.fix.smem_len, PROT_WRITE, MAP_SHARED, fb.fd, 0);
	if (fb.addr == MAP_FAILED) {
		perror("mmap");
		goto err_mmap;
	}

	//4 画图
	if (argc == 1) {
		draw_rect(&fb, 10, 10, 300, 300, RGB888(255, 0, 0));
	} else {
		if (show_bmp(&fb, argv[1]) < 0) {
			fprintf(stderr, "cannot open bmp file\n");
			goto err_open_bmp;
		}
	}
*/
	//5 暂停进程
	pause();

	munmap(fb.addr, fb.fix.smem_len);
	close(fb.fd);

	return 0;

err_open_bmp:
	munmap(fb.addr, fb.fix.smem_len);
err_mmap:
err_ioctl:
	close(fb.fd);
err_open_dev:
	return -1;
}
