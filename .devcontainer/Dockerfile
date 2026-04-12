FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

# use this Dockerfile to install additional tools you might need, e.g.
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#     && apt-get -y install --no-install-recommends <your-package-list-here>

# RUN python3 -m pip install -r requirements.txt
# RUN [[ ! -z $GNUPG_KEY  ]] && gpg --verbose --batch --import <(echo $GNUPG_KEY|base64 -d) && echo 'pinentry-mode loopback' >> ~/.gnupg/gpg.conf
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends python3-pip python3-venv \
    && apt-get clean && rm -rf /var/lib/apt/lists/*
