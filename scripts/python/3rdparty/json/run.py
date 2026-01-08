import os
import sys

sys.path.insert(0, os.environ.get('ROOT_DIR'))

from scripts.python.compile.utils import *

NAME = "json"

src_inc_path = Path(THIRD_PARTY_DIR) / Path(NAME) / Path("include") / Path("nlohmann") / Path("json.hpp")
dec_inc_path = Path(INSTALL_DIR) / Path("include") / Path(NAME) / Path("json.hpp")

CopyFile(src_inc_path, dec_inc_path)
