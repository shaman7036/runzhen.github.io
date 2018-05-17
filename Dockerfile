FROM runzhen/blog:v1

MAINTAINER runzhen

WORKDIR /srv/jekyll

ENTRYPOINT chmod 777 /tmp && jekyll build --destination  /tmp/
