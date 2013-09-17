.PHONY: msp430_cc3000

msp430_cc3000_repo ?= https://github.com/pabigot/cc3000
msp430_cc3000:
	-rm -rf $@
	git clone $($@_repo) $@ 
	(cd $@ && ./configure \
		--host=msp430 \
		--libdir=`readlink -m ../../../obj` \
		--includedir=`readlink -m ../../../libxively/comm_layers/$@`)
	$(MAKE) -C $@ install

