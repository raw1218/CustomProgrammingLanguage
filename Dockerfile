
From raw1218/riscv-jupyter:binder



ARG NB_USER
ARG NB_UID
ENV USER ${NB_USER}
ENV HOME /home/${NB_USER}

RUN adduser --disabled-password \
    --gecos "Default user" \
    --uid ${NB_UID} \
    ${NB_USER}
WORKDIR ${HOME}

RUN pip install --no-cache notebook jupyterlab

WORKDIR ${HOME}/code/JupyterNotebook
RUN jupyter kernelspec install /code/JupyterNotebook
WORKDIR ${HOME}


