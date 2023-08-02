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

COPY . ${HOME}
RUN pip install --no-cache notebook jupyterlab

RUN jupyter --paths


RUN jupyter kernelspec install ${HOME}/JupyterNotebook
RUN jupyter --paths

RUN cd /code && ls



