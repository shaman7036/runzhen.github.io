DST := /usr/local/nginx/html/blog/

all:
	rm -rf $(DST)
	docker run --rm --volume="$(CURDIR):/srv/jekyll" --volume="$(DST):/tmp" runzhen/blog:v2

build:
	docker build -t runzhen/blog:v2 .
