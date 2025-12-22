import os
import re
import shutil
import subprocess
import configparser
from pathlib import Path
from typing import Tuple

ROOT_DIR = os.environ.get('ROOT_DIR')
BUILD_DIR = os.environ.get('BUILD_DIR')
INSTALL_DIR = os.environ.get('INSTALL_DIR')
THREAD_NUM = os.environ.get('THREAD_NUM')
THIRD_PARTY_DIR = os.environ.get('THIRD_PARTY_DIR')
THIRD_PARTY_PYTHON_DIR = os.environ.get('THIRD_PARTY_PYTHON_DIR')

def GetFilePathOK(name: str) -> Tuple[Path, Path]:
    dir = Path(BUILD_DIR) / Path(name)
    path = dir / Path(".build_ok")

    return dir, path

def GetSrcDir(name: str) -> Path:
    dir = Path(THIRD_PARTY_DIR) / Path(name)

    return dir

def GetScriptsDir(name: str) -> Path:
    dir = Path(THIRD_PARTY_PYTHON_DIR) / Path(name)

    return dir

def FindFileOK(dir: Path, path: Path) -> bool:
    is_res = False
    
    if path.exists():
        is_res = True
    else:
        is_res = False
        dir.mkdir(parents=True, exist_ok=True)
    
    return is_res

def CreateFileOK(path: Path):
    path.touch()
    print(f'已经创建文件: {path}')

def ExpandVars(text: str) -> str:
    return re.sub(r'\$\{([^}]+)\}', lambda m: os.environ.get(m.group(1), m.group(0)), text)

def LoadFileINI(ini_path: str, mode: str) -> list:
    config = configparser.ConfigParser()
    config.optionxform = str
    config.read(ini_path)
    cmake_args = []

    for key, val in config[mode].items():
        if mode == 'cmake':
            text = ExpandVars(f"-D{key}={val}")
        elif mode == 'configure':
            if val:
                text = ExpandVars(f"--{key}={val}")
            else:
                text = ExpandVars(f"--{key}")
        cmake_args.append(text)
    return cmake_args

def CMakeConfigure(scripts_dir: str, src_dir: str, build_dir: str):

    ini_path = Path(scripts_dir) / Path('arg.ini')
    ini_args = LoadFileINI(str(ini_path), 'cmake')

    print(f'CMake 命令参数:')
    for cmd in ini_args:
        print(cmd)

    cmake_cmd = ["cmake"] + ini_args + [str(build_dir)] + [str(src_dir)]
    
    result = subprocess.run(cmake_cmd, cwd=src_dir)

    if result.returncode == 0:
        print("CMake 配置成功！")
    else:
        print("CMake 配置失败！")
        print("错误码:", result.returncode)

def Configure(scripts_dir: str, src_dir: str, build_dir: str):
    ini_path = Path(scripts_dir) / Path('arg.ini')
    ini_args = LoadFileINI(str(ini_path), 'configure')

    cmd = [f'{src_dir}/configure'] + ini_args

    print(f'cmd: {cmd}')

    result = subprocess.run(cmd, cwd=build_dir)

    if result.returncode == 0:
        print("Configure 配置成功！")
    else:
        print("Configure 配置失败！")
        print("错误码:", result.returncode)


def MakeBuild(build_dir: str):
    result = subprocess.run(f"make -j{THREAD_NUM}", cwd = build_dir, shell=True)

    if result.returncode == 0:
        print("Make 编译成功！")
    else:
        print("Make 编译失败！")
        print("错误码:", result.returncode)

def MakeInstall(build_dir: str):
    result = subprocess.run(["make", "install"], cwd=build_dir)

    if result.returncode == 0:
        print("Make 安装成功！")
    else:
        print("Make 安装失败！")
        print("错误码:", result.returncode)

def CopyFolder(src: Path, dst: Path, patterns=None):
    if not src.exists():
        print(f"警告：源目录 {src} 不存在，跳过复制")
        return

    dst.mkdir(parents=True, exist_ok=True)

    for item in src.iterdir():
        target = dst / item.name

        if item.is_dir():
            # 递归复制子目录
            CopyFolder(item, target, patterns)
        else:
            # 如果指定 patterns，只复制匹配文件
            if patterns is None or any(item.match(p) for p in patterns):
                shutil.copy2(item, target)

    print(f'{src} 复制到 {dst}')
