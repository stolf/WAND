SUBDIR=Ethernet-Over-UDP clientsrc wand wansd
CFLAGS=-Iinclude/ -g -Wall -O3
CXXFLAGS=-Iinclude/ -g -Wall -O3

all: lib/daemons.o
	for i in $(SUBDIR); do \
		$(MAKE) -C $$i all; \
	done

lib/daemons.o: src/daemons.cc include/daemons.h
	$(CXX) $(CXXFLAGS) -c src/daemons.cc -o lib/daemons.o

clean:
	rm -f lib/*.o src/*~ include/*~
	for i in $(SUBDIR); do \
		$(MAKE) -C $$i clean; \
	done

install: all
	@echo -n Installing Etud...
	@install -D --group=root --mode=555 --owner=root \
		Ethernet-Over-UDP/Etud \
		/usr/local/lib/Etud && echo Done
	@echo -n Installing Drivers...
	@install -D --group=root --mode=555 --owner=root \
		Ethernet-Over-UDP/drivers/ethertap.so \
		/usr/local/lib/wand/drivers/ethertap.so && echo Done
	@echo -n Installing wand_startup.sh...
	@install -D --group=root --mode=555 --owner=root \
		startup.sh \
		/usr/local/lib/sbin/wand_startup.sh && echo Done
	@echo -n Installing wand...
	@install -D --group=root --mode=555 --owner=root \
		wand/wand \
		/usr/local/sbin/wand && echo Done
	@echo -n Installing wansd...
	@install -D --group=root --mode=555 --owner=root \
		wansd/wansd \
		/usr/local/sbin/wansd && echo Done
	@echo -n Installing sample configuration...
	@install -D --group=root --mode=555 --owner=root \
		wand.conf.original \
		/usr/local/etc/wand.$$(hostname).conf.original && echo Done
		


