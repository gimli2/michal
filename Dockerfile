FROM ubuntu:18.04 as michalv2-u18.04
LABEL maintainer="kaloudov@yandex.com"

RUN export DEBIAN_FRONTEND=noninteractive

# fix crazy issue with tzdata on "ubuntu:18.04"
ENV TZ=Europe/Monaco 		
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get -q update && apt-get -yq --no-install-recommends install \
	git bash vim less ca-certificates \
	build-essential \
	libwebp-dev libjpeg-dev libpng-dev libtiff-dev libopencv-dev zlib1g-dev && \
	rm -rf /var/lib/apt/lists/*
