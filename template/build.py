import os 
import shutil
import multiprocessing

def clean_build():
  if os.path.exists("build"):
    shutil.rmtree("build")
  os.mkdir("build")
    
def cpy_kernel():
  if os.path.exists("kernel"):
    shutil.copytree("kernel", "build/kernel")
  
def build_project():
  cur_dir = os.getcwd()
  binary_dir = os.path.join(cur_dir, 'build')
  a = os.system('cmake -S "%s" -B "%s"' % (cur_dir, binary_dir))
  b = os.system('cmake --build "%s" --parallel %s --config Debug' % (binary_dir, multiprocessing.cpu_count() >> 1))
  
clean_build()
cpy_kernel()
build_project()