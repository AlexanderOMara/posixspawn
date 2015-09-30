build:
	mkdir -p dist
	clang -Wall -mmacosx-version-min=10.6 -arch x86_64 -arch i386 -o dist/posixspawn src/posixspawn.c

clean:
	rm -r ./dist/*
