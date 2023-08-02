From riscv-qemu:latest

WORKDIR / 


COPY . /code



RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    python3-setuptools \
    python3-wheel \
    build-essential

RUN pip3 install jupyter

WORKDIR /code/JupyterNotebook

RUN jupyter kernelspec install ./ --user --name=customlang


