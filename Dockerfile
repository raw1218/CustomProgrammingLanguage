
From raw1218/riscv-jupyter:binder



ARG NB_USER=jovyan
ARG NB_UID=1000
ENV USER ${NB_USER}
ENV NB_UID ${NB_UID}
ENV HOME /home/${NB_USER}

RUN adduser --disabled-password \
    --gecos "Default user" \
    --uid ${NB_UID} \
    ${NB_USER}

WORKDIR ${HOME}

RUN python3 -m pip install --no-cache-dir notebook jupyterlab
RUN pip install --no-cache-dir jupyterhub

WORKDIR ${HOME}/code/JupyterNotebook
RUN jupyter kernelspec install /code/JupyterNotebook
WORKDIR ${HOME}
