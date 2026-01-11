import os
import sys

sys.path.insert(0, os.environ.get('ROOT_DIR'))

from scripts.python.compile.utils import *

NAME = "rga"

def CreateFilePC(install_path):
    pkgconfig_path = Path(install_path) / Path('lib') / Path('pkgconfig') / Path('librga.pc')
    with open(pkgconfig_path, "w") as f:
        f.write(
            f"prefix={install_path}\n"
            "exec_prefix=${prefix}\n"
            "libdir=${prefix}/lib\n"
            "includedir=${prefix}/include/" + NAME +  "\n\n"
            "Name: " + NAME + "\n"
            "Description: Rockchip RGA Library\n"
            "Version: 1.0\n"
            "Libs: -L${libdir} -lrga\n"
            "Cflags: -I${includedir}\n"
        )

src_inc_path = Path(THIRD_PARTY_DIR) / Path(NAME) / Path("include")
dec_inc_path = Path(INSTALL_DIR) / Path("include") / Path('rga')

src_lib_path = Path(THIRD_PARTY_DIR) / Path(NAME) / Path("libs") / Path("Linux") / Path('gcc-aarch64')
dec_lib_path = Path(INSTALL_DIR) / Path("lib")

CopyFolder(src_inc_path, dec_inc_path)
CopyFolder(src_lib_path, dec_lib_path)
CreateFilePC(os.getenv("INSTALL_DIR"))
