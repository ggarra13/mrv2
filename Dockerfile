# syntax=docker/dockerfile:1

#
# Download base image ubuntu 16.04 which is the minimum one that can compile
# mrv2
# However, if you are building it not for distribution, we recommend you use:
#
# FROM ubuntu:latest or ubuntu:22.04
#
FROM ubuntu:16.04

# LABEL about the custom image
LABEL maintainer="ggara13@gmail.com"
LABEL description="This is a custom Docker Image for mrv2."

VOLUME /release

# Update Ubuntu Software repository
RUN apt-get update

#
# Install dependencies
#
RUN apt-get install -y xorg-dev libglu1-mesa-dev mesa-common-dev \
	 libx11-dev libxcursor-dev libxinerama-dev libasound2-dev libpulse-dev \
	 libpango1.0-dev wget git ninja-build
RUN rm -rf /var/lib/apt-get/lists/*
RUN apt-get clean

#
# Clone the mrv2 reposiory
#
RUN git clone https://github.com/ggarra13/mrv2.git
WORKDIR /mrv2
RUN git fetch

#
# Get the a new enough enough cmake (as the one in ubuntu 16.04 is too old)
#
RUN wget https://github.com/Kitware/CMake/releases/download/v3.25.2/cmake-3.25.2-linux-x86_64.tar.gz
RUN tar -xf cmake-3.25.2-linux-x86_64.tar.gz

# Run the build
RUN export PATH="/mrv2/cmake-3.25.2-linux-x86_64/bin:$PATH" && ./runme.sh

# # Create the .deb, .rpm and tar.gz packages
# RUN ./runmeq.sh -t package

# Clear release directory where packages will be stored
RUN rm -rf /release/*

# # Move it to the release volume
# RUN echo "Moving packagest to /release volume"
# RUN mv BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.deb    /release
# RUN mv BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.rpm    /release
# RUN mv BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.tar.gz /release
