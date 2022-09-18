build:
	mkdir -p dist
	clang -Wall -mmacosx-version-min=10.6 -arch arm64 -arch x86_64 -o dist/posixspawn src/posixspawn.c

clean:
	rm -rf dist
