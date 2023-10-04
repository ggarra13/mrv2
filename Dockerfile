# syntax=docker/dockerfile:1

FROM rockylinux:8

# LABEL about the custom image
LABEL maintainer="ggara13@gmail.com"
LABEL description="This is a custom Docker Image for mrv2."

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
		   libXScrnSaver-devel dpkg doxygen zlib-devel libffi-devel \
		   openssl-devel tk-devel tcl-devel swig subversion
		   
#
# Install USD dependencies
#
RUN dnf -y install python39 libXt-devel

#
# Install Wayland dependencies (currently broken in NVIDIA driver)
#
RUN dnf -y install autoconf wayland-devel wayland-protocols-devel cairo-devel \
		   libxkbcommon-devel dbus-devel mesa-libGLU-devel gtk3-devel

#
# Set Work Directory (where we put the repository)
#
WORKDIR /src

#
# Copy the package extract script to root
#
COPY ./etc/entrypoint.sh /entrypoint.sh

# Make Executable
RUN chmod +x /entrypoint.sh

RUN chmod a+rwx /src

ENTRYPOINT ["/entrypoint.sh"]
