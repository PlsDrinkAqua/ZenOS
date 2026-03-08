DEFAULT_HOST!=../default-host.sh
HOST?=$(DEFAULT_HOST)
HOSTARCH!=../target-triplet-to-arch.sh $(HOST)

CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -I../include
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lgcc

# 用户程序不需要 kernel 专用宏
# 不要加 -D__is_kernel
# 不要链接 -lk

TARGET = userprog

OBJS = \
	userprog.o

LINK_LIST = \
	$(LDFLAGS) \
	$(OBJS) \
	$(LIBS)

.PHONY: all clean
.SUFFIXES: .o .c .S

all: $(TARGET)

$(TARGET): $(OBJS) user.ld
	$(CC) -T user.ld -o $@ $(CFLAGS) $(LINK_LIST)

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	rm -f $(TARGET)
	rm -f $(OBJS) *.d

-include $(OBJS:.o=.d)