all: \
	dist/posixspawn \
	dist/posixspawn.sha256

dist:
	mkdir -p dist

dist/posixspawn: dist
	clang -Wall -mmacosx-version-min=10.6 -arch arm64 -arch x86_64 -o $@ src/posixspawn.c

dist/posixspawn.sha256: dist/posixspawn
	cd dist && shasum -a 256 -b posixspawn > posixspawn.sha256 && cat posixspawn.sha256

clean:
	rm -rf dist
