#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <linux/input.h>

#define test_bit(bit, array)    (array[bit/8] & (1<<(bit%8)))


void handle_key_event(struct input_event *iev)
{
	printf("key event:    event.code: %d		event.value: %d\n", iev->code, iev->value);

	switch(iev->code)
	{
		case KEY_1:		printf("KEY_1:	%s\n", iev->value? "press" : "release");	break;
	}
}

void handle_rel_event(struct input_event *iev)
{
	printf("rel event:    event.code: %d		event.value: %d\n", iev->code, iev->value);
}

void handle_abs_event(struct input_event *iev)
{
	printf("abs event:    event.code: %d		event.value: %d\n", iev->code, iev->value);
}

int main(int argc,char *argv[])
{
	int fd = 0;
	int ret = 0;
	char name[128];

	if(2 != argc)
	{
		printf("./input_test ");
		printf("param1  input dev node:		example: /dev/event0");
		return -1;
	}

	printf("open dev: %s\n", argv[1]);

	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
	{
		printf("Fail to open device %s\n", argv[1]);
		return -EINVAL;
	}

	memset(name, 0, sizeof(name));
	ret = ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name);
	if (ret < 0)
	{
		printf("Fail to do ioctl EVIOCGNAME\n");
	}

	printf("device name %s\n", name);

#if 0
	unsigned char key_bitmask[(KEY_MAX + 7) / 8];
	memset(key_bitmask, 0, sizeof(key_bitmask));
	printf("Getting keys...\n");
	ret = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask);
	if (ret < 0){
	printf("Fail to do ioctl EVIOCGBIT(EV_KEY)\n");
	goto exit;
	}


	if (test_bit(BTN_TOUCH, key_bitmask)){
	printf("It's a single keypad driver\n");
	}else{
	printf("It does not seem like a keypad driver. Please try other devices\n");
	goto exit;
	}
#endif

	struct input_event iev;
	while (1)
	{
		ret = read(fd, &iev, sizeof(iev));
		if (ret != sizeof(iev))
		{
			printf("Fail to read input event %d\n", ret);
			goto exit;
		}

//		printf("event.type: %d    event.code: %d		event.value: %d\n", iev.type, iev.code, iev.value);

		if (iev.type == EV_KEY)
		{
			// key event:  key board , button,  key pad
			handle_key_event(&iev);
		}
		else if(iev.type == EV_REL)
		{
			// rel event: mouse
			handle_rel_event(&iev);
		}
		else if(iev.type == EV_ABS)
		{
			//abs event: touch panel
			handle_abs_event(&iev);			
		}
	}

exit : 
	close(fd);

	return 0;
}

