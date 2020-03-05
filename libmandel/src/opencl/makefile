
HEADERS := $(patsubst %.cl,%.h,$(wildcard *.cl))


all:
	$(HEADERS)

%.h: %.cl
	xxd -i $< > $@

