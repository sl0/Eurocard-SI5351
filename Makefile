#!/usr/bin/env make -f

r.pdf: README.md
	pandoc -o r.pdf README.md

clean:
	rm -f r.pdf
