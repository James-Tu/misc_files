OBJS = div64.o lib1funcs.o ctype.o muldi3.o printf.o string.o vsprintf.o
LIB = libc.a

$(LIB):$(OBJS)
	${AR} -r -o $@ $^

%.o:%.c
	${CC} $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(LIB)