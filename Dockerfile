
From raw1218/riscv-jupyter:latest


WORKDIR /

CMD ["jupyter", "notebook", "--ip", "0.0.0.0", "--no-browser", "--allow-root"]

EXPOSE 8888



