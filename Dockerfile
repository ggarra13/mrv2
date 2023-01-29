# syntax=docker/dockerfile:1

FROM rockylinux:8

# LABEL about the custom image
LABEL maintainer="ggara13@gmail.com"
LABEL description="This is a custom Docker Image for mrv2."

VOLUME /packages

#
# Install repositories
#
RUN dnf -y install dnf-plugins-core epel-release
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
		   libXScrnSaver-devel dpkg gettext libvpx-devel

#
# Clone the mrv2 repository (last tag)
#
RUN REPO=https://github.com/ggarra13/mrv2.git && \
    git clone $REPO --single-branch --branch \
    $(git ls-remote --tags --refs $REPO | tail -n1 | cut -d/ -f3)

#
# Clone the mrv2 reposiory (latest)
#
#RUN REPO=https://github.com/ggarra13/mrv2.git && git clone $REPO

#
# Set Work Directory
#
WORKDIR /mrv2

#
# Run the build.  Use -G Ninja for faster but not so interactive builds
#
RUN ./runme.sh -G 'Unix Makefiles'

# Create the .deb, .rpm and tar.gz packages
RUN ./runmeq.sh -t package

# Copy the package extract script to root

COPY ./etc/extract.sh /extract.sh

# Make Executable
RUN chmod +x /extract.sh

ENTRYPOINT ["/extract.sh"]
