# syntax=docker/dockerfile:1

# Download base image ubuntu 16.04
# FROM ubuntu:16.04
FROM ubuntu:22.04

# LABEL about the custom image
LABEL maintainer="ggara13@gmail.com"
LABEL version="0.4"
LABEL description="This is a custom Docker Image for mrv2."


# Update Ubuntu Software repository
RUN apt update
RUN apt upgrade -y

#
# Install dependencies
#
RUN sudo apt install -y xorg-dev libglu1-mesa-dev mesa-common-dev \
         libx11-dev libxcursor-dev libxinerama-dev libasound2-dev libpulse-dev \
         libpango1.0-dev ninja-build
RUN rm -rf /var/lib/apt/lists/*
RUN apt clean


COPY mrv2 /opt/mrv2
WORKDIR /opt/Hmrv2
RUN ./runme.sh
CMD ["./BUILD-Linux-64/Release/install/bin/mrv2.sh"]
