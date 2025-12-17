ifndef AIO_API
AIO_API := \"https://aioapi.seceurity.place\"
endif

ifndef AIO_APILEN
AIO_APILEN := $(shell printf "%s-2\n" $$(echo -n $(AIO_API) | wc -c) | bc)
endif

CFLAGS = -Wall -std=c99 -O2 -DAIO_API=$(AIO_API) -DAIO_APILEN=$(AIO_APILEN)
LIBS = `pkg-config --libs --cflags json-c vips libcurl libsixel` -lselector


OBJDIR = ./obj

SOURCES = $(wildcard ./**/*.c ./*.c)
OBJECTS = $(patsubst ./%.c,$(OBJDIR)/%.o,$(SOURCES))

TARGET = aio-cli

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

$(OBJDIR)/%.o: ./%.c selector.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LIBS) -c $< -o "$@"

selector.h:
	curl 'https://raw.githubusercontent.com/Euro20179/selector.h/refs/heads/master/selector.h' > selector.h

.PHONY: clean
clean:
	rm -r $(OBJDIR)
	rm -f $(TARGET) $(OBJECTS) ./selector.h
