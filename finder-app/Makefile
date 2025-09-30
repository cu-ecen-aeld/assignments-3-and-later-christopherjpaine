# Set default goal
.DEFAULT_GOAL := all

# Make writer app
all:
	$(CC) $(CFLAGS) $(INCLUDES) -o writer writer.c $(LDFLAGS)

# clean writer app
clean:
	rm -f writer

install: all
	sudo cp writer /usr/bin/writer
	sudo cp finder.sh /usr/bin/finder.sh
	sudo mkdir -p /etc/finder-app
	sudo cp -r conf /etc/finder-app/conf

uninstall:
	sudo rm /usr/bin/writer
	sudo rm /usr/bin/finder.sh
	sudo rm -r /etc/finder-app