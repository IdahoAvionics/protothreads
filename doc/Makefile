clean:
	rm -rf html latex

docker-image:
	docker build . -t pt-doc-builder

html/index.html, latex/Makefile: docker-image
	docker run -i --rm -v ..:/workdir -w /workdir/doc pt-doc-builder doxygen Doxyfile

latex/refman.pdf: latex/Makefile docker-image
	docker run -i --rm -v .:/workdir -w /workdir/latex pt-doc-builder make

pt-refman.pdf: latex/refman.pdf
	cp latex/refman.pdf pt-refman.pdf
