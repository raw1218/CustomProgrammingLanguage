
From raw1218/riscv-jupyter:binder

WORKDIR /code
COPY . /code

RUN jupyter kernelspec install /code/JupyterNotebook




