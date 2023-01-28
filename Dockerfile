# syntax=docker/dockerfile:1

FROM rockylinux:8

# LABEL about the custom image
LABEL maintainer="ggara13@gmail.com"
LABEL description="This is a custom Docker Image for mrv2."

VOLUME /release

#
#
#
RUN dnf -y install dnf-plugins-core
RUN dnf -y install epel-release
RUN dnf config-manager --set-enabled powertools

#
# Update dnf database
#
RUN dnf makecache --refresh

#
# Install bundles
#
RUN dnf -y groupinstall "Development Tools"

#
# Install dependencies
#
RUN dnf -y install git wget cmake pango-devel gettext ninja-build \
		   libglvnd-devel alsa-lib-devel pulseaudio-libs-devel \
		   libXScrnSaver-devel

#
# Clone the mrv2 reposiory
#
RUN git clone https://github.com/ggarra13/mrv2.git
WORKDIR /mrv2
RUN git fetch

#
# Run the build
#
RUN ./runme.sh

# Create the .deb, .rpm and tar.gz packages
RUN ./runmeq.sh -t package

# Clear release directory where packages will be stored
RUN rm -rf /release/*

# # Move it to the release volume
RUN echo "Moving packagest to /release volume"
RUN ls /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/
RUN mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.deb    /release
RUN mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.rpm    /release
RUN mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.tar.gz /release
