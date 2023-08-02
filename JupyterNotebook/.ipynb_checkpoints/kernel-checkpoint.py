import os
import tempfile
import subprocess
from ipykernel.kernelbase import Kernel


import subprocess
import tempfile
class MyLanguageKernel(Kernel):

    banner = "My custom kernel v1.0"

    implementation = 'MyLanguage'
    implementation_version = '0.1'
    language_info = {
        'name': 'mylang',
        'mimetype': 'text/plain',
        'file_extension': '.txt',
    }

    def compile_and_run(self, code):
        # Define paths
        COMPILER_PATH = "/code/testprogram"
        ASSEMBLER_PATH = "riscv32-unknown-elf-as"
        GCC_PATH = "riscv32-unknown-elf-gcc"
        VM_PATH = "qemu-riscv32"

        # Create temporary files
        with tempfile.NamedTemporaryFile(suffix=".txt", delete=False) as input_file:
            input_file.write(code.encode())
            input_file_path = input_file.name

        output_s_path = tempfile.mktemp(suffix=".s")
        output_o_path = tempfile.mktemp(suffix=".o")
        output_path = tempfile.mktemp()

        # Compile the code using your custom compiler
        subprocess.run([COMPILER_PATH, input_file_path, output_s_path])

        # Assemble the .s file
        subprocess.run([ASSEMBLER_PATH, "-march=rv32i", "-o", output_o_path, output_s_path])

        # Link the .o file
        subprocess.run([GCC_PATH, "-o", output_path, output_o_path])

        # Run the compiled code with the VM
        result = subprocess.run([VM_PATH, output_path], stdout=subprocess.PIPE)

        return result.stdout.decode()



    def do_execute(self, code, silent, store_history=True, user_expressions=None, allow_stdin=False):
        output = self.compile_and_run(code)
        if not silent:
            self.send_response(self.iopub_socket, 'stream', {
                'name': 'stdout',
                'text': output
            })
        return {
            'status': 'ok',
            'execution_count': self.execution_count,
            'payload': [],
            'user_expressions': {},
        }

if __name__ == '__main__':
    from ipykernel.kernelapp import IPKernelApp
    IPKernelApp.launch_instance(kernel_class=MyLanguageKernel)



















class MyKernel(Kernel):
    implementation = 'MyLanguage'
    implementation_version = '1.0'
    language = 'mylanguage'
    language_version = '1.0'
    language_info = {
        'name': 'mylanguage',
        'mimetype': 'text/plain',
        'file_extension': '.txt',
    }
    banner = "My language kernel"

    def do_execute(self, code, silent, store_history=True, user_expressions=None, allow_stdin=False):
        # We're not using history, user expressions, or stdin
        assert not store_history
        assert not user_expressions
        assert not allow_stdin

        # Write the code to a temporary file
        with tempfile.NamedTemporaryFile(delete=False, mode='w') as temp_input:
            temp_input.write(code)

        # Prepare paths for your compiler and Spike
        compiler_path = "../Language/testprogram"
        spike_path = "../VM/riscv-isa-sim/build/spike"

        # Call your compiler
        temp_output = tempfile.NamedTemporaryFile(delete=False).name
        compile_process = subprocess.Popen([compiler_path, temp_input.name, temp_output])
        compile_process.wait()

        # Ensure your compiler exited successfully
        if compile_process.returncode != 0:
            return {
                'status': 'error',
                'execution_count': self.execution_count,
                'ename': '', 'evalue': 'Failed to compile code',
                'traceback': []
            }

        # Call Spike
        spike_process = subprocess.Popen([spike_path, 'pk', temp_output], stdout=subprocess.PIPE)
        spike_output, _ = spike_process.communicate()

        # Ensure Spike exited successfully
        if spike_process.returncode != 0:
            return {
                'status': 'error',
                'execution_count': self.execution_count,
                'ename': '', 'evalue': 'Failed to execute code with Spike',
                'traceback': []
            }

        # If not silent, send the output from Spike back to the Jupyter notebook
        if not silent:
            stream_content = {'name': 'stdout', 'text': spike_output.decode('utf-8')}
            self.send_response(self.iopub_socket, 'stream', stream_content)

        return {
            'status': 'ok',
            'execution_count': self.execution_count,
            'payload': [],
            'user_expressions': {},
        }

if __name__ == '__main__':
    from ipykernel.kernelapp import IPKernelApp
    IPKernelApp.launch_instance(kernel_class=MyKernel)
