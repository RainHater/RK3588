import os
import sys

sys.path.insert(0, os.environ.get('ROOT_DIR'))

from scripts.python.compile.utils import *

NAME = "spdlog"

build_dir, ok_file = GetFilePathOK(NAME)
scripts_dir = GetScriptsDir(NAME)
src_dir = GetSrcDir(NAME)
res = FindFileOK(build_dir, ok_file)

if not res:
    print('未编译开始编译...')
    CMakeConfigure(scripts_dir, src_dir, build_dir)
    MakeBuild(build_dir)
    MakeInstall(build_dir)
    CreateFileOK(ok_file)
else:
    print(f'已编译过!')
print(f'{NAME} 构建编译完')

# src_inc_path = Path(THIRD_PARTY_DIR) / Path(NAME) / Path("include") / Path(NAME)
# dec_inc_path = Path(INSTALL_DIR) / Path("include") / Path(NAME)

# CopyFolder(src_inc_path, dec_inc_path)
