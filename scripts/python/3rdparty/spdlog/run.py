import os
import sys

sys.path.insert(0, os.environ.get('ROOT_DIR'))

from scripts.python.compile.utils import *

NAME = "spdlog"

src_inc_path = Path(THIRD_PARTY_DIR) / Path(NAME) / Path("include") / Path(NAME)
dec_inc_path = Path(INSTALL_DIR) / Path("include") / Path(NAME)

CopyFolder(src_inc_path, dec_inc_path)
