clean:
	rm -rf html latex

ifdef USE_DOCKER
docker-image:
	docker build . -t pt-doc-builder

html/index.html latex/Makefile: docker-image
	docker run -i --rm -v ..:/workdir -w /workdir/doc pt-doc-builder doxygen Doxyfile

latex/refman.pdf: latex/Makefile docker-image
	docker run -i --rm -v .:/workdir -w /workdir/latex pt-doc-builder make
else
html/index.html latex/Makefile:
	doxygen Doxyfile

latex/refman.pdf: latex/Makefile
	$(MAKE) -C latex
endif

pt-refman.pdf: latex/refman.pdf
	cp latex/refman.pdf pt-refman.pdf
