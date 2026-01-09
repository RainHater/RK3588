import os
import sys

sys.path.insert(0, os.environ.get('ROOT_DIR'))

from scripts.python.compile.utils import *

NAME = "rknpu2"

common_path = Path("runtime") / Path("RK3588") / Path("Linux") / Path("librknn_api")

src_inc_path = Path(THIRD_PARTY_DIR) / Path(NAME) / Path(common_path) / Path("include")
dec_inc_path = Path(INSTALL_DIR) / Path("include") / Path(NAME)

src_lib_path = Path(THIRD_PARTY_DIR) / Path(NAME) / Path(common_path) / Path("aarch64")
dec_lib_path = Path(INSTALL_DIR) / Path("lib")

CopyFolder(src_inc_path, dec_inc_path, patterns=["*.h"])
CopyFolder(src_lib_path, dec_lib_path, patterns=["*.so"])
