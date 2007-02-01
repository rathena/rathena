
OBJECTS = sample.dll sig.dll pid.dll gui.dll upnp.dll console.dll

ifdef CYGWIN
   PLUGINEXT = dll
else
   PLUGINEXT = so
endif

PLUGINS = $(OBJECTS:%.dll=%.$(PLUGINEXT))
COMMON_H = ../common/plugin.h ../common/cbasetypes.h

txt sql all: $(PLUGINS)

%.$(PLUGINEXT): %.c
	$(CC) $(CFLAGS) -shared -o ../../plugins/$@ $<
	@touch $@

sig.$(PLUGINEXT): sig.c
	$(CC) $(CFLAGS) -shared -o ../../plugins/$@ $< \
	../common/obj/showmsg.o ../common/obj/utils.o \
	../common/obj/minimalloc.o
	@touch $@

gui.$(PLUGINEXT): ../../plugins/gui.conf
upnp.$(PLUGINEXT): ../../plugins/upnp.conf

../../plugins/%.conf: %.txt
	cp -r $< $@

../../plugins/gui.conf: gui.txt
../../plugins/upnp.conf: upnp.txt

depend:
	makedepend -fGNUmakefile -o.$(PLUGINEXT) -Y. -Y../common *.c $(PLUGINS:%=../../plugins/%)
clean:
	rm -rf $(PLUGINS) GNUmakefile

# DO NOT DELETE

sample.$(PLUGINEXT): sample.c $(COMMON_H)
sig.$(PLUGINEXT): sig.c $(COMMON_H)
pid.$(PLUGINEXT): pid.c $(COMMON_H)
gui.$(PLUGINEXT): gui.c $(COMMON_H)
