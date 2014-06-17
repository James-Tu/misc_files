OBJS = net_core.o tftp.o
LIB = libnet.a

$(LIB):$(OBJS)
	${AR} -r -o $@ $^

%.o:%.c
	${CC} $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(LIB)