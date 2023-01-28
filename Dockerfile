# syntax=docker/dockerfile:1

# Download base image ubuntu 16.04
FROM ubuntu:16.04
#
#
# FROM ubuntu:22.04

# LABEL about the custom image
LABEL maintainer="ggara13@gmail.com"
LABEL version="0.4"
LABEL description="This is a custom Docker Image for mrv2."

VOLUME /build
VOLUME /release

# Update Ubuntu Software repository
RUN apt-get update

#
# Install dependencies
#
RUN apt-get install -y xorg-dev libglu1-mesa-dev mesa-common-dev \
	 libx11-dev libxcursor-dev libxinerama-dev libasound2-dev libpulse-dev \
	 libpango1.0-dev cmake ninja-build
RUN rm -rf /var/lib/apt-get/lists/*
RUN apt-get clean

# Import resources
COPY ./etc/entrypoint.sh /entrypoint.sh

# Make Executable
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
