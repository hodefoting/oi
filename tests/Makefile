check: outputs
	diff -su reference output
clean:
	rm -f output/*
	rm -rf zn-db*
outputs: *.zn ../zn
	@echo -n Tests:
	@for a in `for a in *.zn;do echo $$a |sed s/.zn//;done`;do \
	  echo -n $$a" "; ../zn $$a.zn > output/$$a;\
		rm -rf zn-db*;\
	done ; echo 
../zn:
	make -C ..
sync:
	cp output/* reference
