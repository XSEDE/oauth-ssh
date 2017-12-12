CC=gcc

CFLAGS=-Wall -fPIC -ansi -pedantic -g -O1
DEST=/lib64/security

all: pam_oauth2.so

jsmn/libjsmn.a:
	$(MAKE) CFLAGS="$(CFLAGS)" -C jsmn

pam_oauth2.so: pam_oauth2.o jsmn/libjsmn.a
	$(CC) -shared $^ -lcurl -o $@

install: pam_oauth2.so
	if ! test -d $(DEST); then mkdir $(DEST); fi
	chmod 755 pam_oauth2.so
	cp -a pam_oauth2.so $(DEST)

clean:
	rm -f *.o *.so
