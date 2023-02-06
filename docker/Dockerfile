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
		   libXScrnSaver-devel dpkg gettext  

#
# Install additional codecs (an example, we are not yet using it)
#
RUN dnf -y install libvpx-devel

#
# Install Wayland dependencies (currently broken in NVIDIA driver)
#
RUN dnf -y install autoconf wayland-devel wayland-protocols-devel cairo-devel \
		   libxkbcommon-devel dbus-devel mesa-libGLU-devel gtk3-devel

#
# Clone the mrv2 repository (last tag)
#
RUN REPO=https://github.com/ggarra13/mrv2.git && \
    TAG=$(git ls-remote --tags --refs $REPO | tail -n1 | cut -d/ -f3) && \
    echo "Cloning tag ${TAG}..." && \
    git clone $REPO --single-branch --branch ${TAG}

#
# Set Work Directory (where we put the repository)
#
WORKDIR /mrv2


# Copy the package extract script to root
COPY ./etc/entrypoint.sh /entrypoint.sh

# Make Executable
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
