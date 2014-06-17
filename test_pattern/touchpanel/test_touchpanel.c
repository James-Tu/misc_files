#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <linux/input.h>
#include <linux/fb.h>

#define FRAMEBUFFER_DEV "/dev/fb0"

#define test_bit(bit, array)    (array[bit/8] & (1<<(bit%8)))

enum ts_event_state{
    TS_IDLE,
    TS_X_REPORTED,
    TS_Y_REPORTED
};

static void usage(char *name)
{
    printf("Usage : \n");
    printf("\t%s event_device_node\n", name);
}

int main(int argc,char *argv[])
{
    int ts_fd = -1;
    int ret;
    unsigned char name[128];

    printf("Touchpanel test program.\n");

    if (argc != 2){
        usage(argv[0]);
        return -EINVAL;
    }

    ts_fd = open(argv[1], O_RDONLY);
    if (ts_fd < 0){
        printf("Fail to open device %s\n", argv[1]);
        return -EINVAL;
    }

    unsigned char key_bitmask[(KEY_MAX + 7) / 8];
    memset(key_bitmask, 0, sizeof(key_bitmask));
    printf("Getting keys...\n");
    ret = ioctl(ts_fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask);
    if (ret < 0){
        printf("Fail to do ioctl EVIOCGBIT(EV_KEY)\n");
        goto exit;
    }

    unsigned char abs_bitmask[(ABS_MAX + 7) / 8];
    memset(abs_bitmask, 0, sizeof(abs_bitmask));
    printf("Getting absolute controllers...\n");
    ret=ioctl(ts_fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask);
    if (ret < 0){
        printf("Fail to do ioctl EVIOCGBIT(EV_ABS)\n");
        goto exit;
    }

    if (test_bit(BTN_TOUCH, key_bitmask) &&
        test_bit(ABS_X, abs_bitmask) &&
        test_bit(ABS_Y,
            abs_bitmask)){
        printf("It's a single touch driver\n");
    }else{
        printf("It does not seem like a touch driver. Please try other devices\n");
        goto exit;
    }

    memset(name, 0, sizeof(name));
    ret = ioctl(ts_fd, EVIOCGNAME(sizeof(name) - 1), &name);
    if (ret < 0){
        printf("Fail to do ioctl EVIOCGNAME\n");
        goto exit;
    }
    printf("device name %s\n", name);

    /* frame buffer */
    int fb = open(FRAMEBUFFER_DEV, O_RDWR);
    if (fb < 0){
        printf("Fail to open frame buffer device %s\n", FRAMEBUFFER_DEV);
        goto exit;
    }

    struct fb_fix_screeninfo fb_fix;
    struct fb_var_screeninfo fb_var;
    ret = ioctl(fb, FBIOGET_VSCREENINFO, &fb_var);
    if (ret < 0){
        printf("Fail to do IOCTL FBIOGET_VSCREENINFO %d\n", ret);
        goto exit;
    }
    ret = ioctl(fb, FBIOGET_FSCREENINFO, &fb_fix);
    if (ret < 0){
        printf("Fail to do IOCTL FBIOGET_FSCREENINFO %d\n", ret);
        goto exit;
    }
    unsigned short * fb_mem = (unsigned short *) (-1);
    fb_mem = mmap(NULL,
                 fb_fix.smem_len,
                 PROT_READ | PROT_WRITE,
                 MAP_SHARED,
                 fb,
                 0);
    if ((unsigned short *) (-1) == fb_mem){
        printf("Fail to do mmap \n");
        goto exit;
    }

    memset(fb_mem, 0, fb_fix.smem_len);

    printf("[%d  %d]\n",  fb_var.xres, fb_var.yres);

    struct input_event iev;
    enum ts_event_state state = TS_IDLE;
    int x, y;
    while (1){
        ret = read(ts_fd, &iev, sizeof(iev));
        if (ret != sizeof(iev)){
            printf("Fail to read input event %d\n", ret);
            goto exit;
        }
        if (iev.type == EV_ABS){
            switch (state) {
                case TS_IDLE:
                    if (iev.code == ABS_X){
                        x = iev.value;
                        state = TS_X_REPORTED;
                    }else{
                        if (iev.code == ABS_Y){
                            y = iev.value;
                            //printf("x1=%d,y1=%d\n", x, y);
                            fb_mem[fb_var.xres *y + x] = 0xffff;
                            state = TS_IDLE;
                        }else{
                            state = TS_IDLE;
                            printf("TS_IDLE state, no ABS_X event!ive code=%d,ive value=%d\n",iev.code, iev.value);
                        }
                    }
                break;
                case TS_X_REPORTED:
                    if (iev.code == ABS_Y){
                        y = iev.value;
                        state = TS_IDLE;
                        /* draw_point */
                        //printf("x3=%d,y1=%d\n", x, y);
                        fb_mem[fb_var.xres *y + x] = 0xffff;
                    }else{
                        if (iev.code == ABS_X){
                            //printf("x4=%d,y2=%d\n", x, y);
                            x = iev.value;
                            state = TS_X_REPORTED;
                            fb_mem[fb_var.xres *y + x] = 0xffff;
                        }else{
                            state = TS_X_REPORTED;
                            printf("TS_X_REPORTED state, no ABS_Y event!iev code =%d,iev value=%d\n", iev.code, iev.value);
                        }
                    }
                break;
                default:
                    printf("Unexpected state and event!\n");
                    printf("time %d:%d\ttype 0x%x code 0x%x value 0x%x\n",
                        iev.time.tv_sec,
                        iev.time.tv_usec,
                        iev.type, iev.code,
                        iev.value);
                break;
            }
        }
    }

    exit:
    if (fb_mem != (unsigned short *) (-1)){
        munmap(fb_mem, fb_fix.smem_len);
    }
    if (ts_fd >= 0){
        close(ts_fd);
    }
    if (fb >= 0){
        close(fb);
    }
    return 0;
}


