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

def FindPlaceholders(s):
    pattern = r'\${(.*?)}'
    
    def repl(match):
        var_name = match.group(1)
        return os.environ.get(var_name, match.group(0))

    return re.sub(pattern, repl, s)

def LoadFileEnvINI(ini_path: str):
    ini_args = []
    print('参数配置')
    with open(ini_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            line = FindPlaceholders(line)
            ini_args.append(line)
            print(line)
    return ini_args

def CMakeConfigure(scripts_dir: str, src_dir: str, build_dir: str):
    ini_path = Path(scripts_dir) / Path('arg.ini')
    # ini_args = LoadFileINI(str(ini_path), 'cmake')
    ini_args = LoadFileEnvINI(ini_path)

    print(f'src_dir: {src_dir}')
    print(f'build_dir: {build_dir}')

    cmake_cmd = ["cmake"] + ini_args + [str(build_dir)] + [str(src_dir)]
    result = subprocess.run(cmake_cmd, cwd=build_dir)

    if result.returncode == 0:
        print("CMake 配置成功！")
    else:
        raise RuntimeError(f"CMake 配置失败，命令返回码 {result.returncode}")
    
    return result.returncode

def Configure(scripts_dir: str, src_dir: str, build_dir: str):
    ini_path = Path(scripts_dir) / Path('arg.ini')

    ini_args = LoadFileEnvINI(ini_path)

    env = os.environ.copy()
    cmd = [f'{src_dir}/configure'] + ini_args

    result = subprocess.run(cmd, cwd=build_dir, env=env)

    if result.returncode == 0:
        print("Configure 配置成功！")
    else:
        raise RuntimeError(f"Configure 配置失败，命令返回码 {result.returncode}")
    return result.returncode


def MakeBuild(build_dir: str):
    result = subprocess.run(f"make -j{THREAD_NUM}", cwd = build_dir, shell=True)

    if result.returncode == 0:
        print("Make 编译成功！")
    else:
        raise RuntimeError(f"Make 配置失败，命令返回码 {result.returncode}")

def MakeInstall(build_dir: str):
    result = subprocess.run(["make", "install"], cwd=build_dir)

    if result.returncode == 0:
        print("Make 安装成功！")
    else:
        raise RuntimeError(f"Make 安装失败，命令返回码 {result.returncode}")

def CopyFolder(src: Path, dst: Path, patterns=None):
    if not src.exists():
        print(f"警告：源目录 {src} 不存在，跳过复制")
        return

    dst.mkdir(parents=True, exist_ok=True)

    for item in src.iterdir():
        target = dst / item.name

        if item.is_dir():
            CopyFolder(item, target, patterns)
        else:
            if patterns is None or any(item.match(p) for p in patterns):
                shutil.copy2(item, target)

    print(f'{src} 复制到 {dst}')

def CopyFile(src: Path, dst: Path):
    if not src.exists():
        print(f"警告：源文件 {src} 不存在，跳过复制")
        return

    if src.is_dir():
        print(f"警告：{src} 是目录，不能用 CopyFile")
        return

    if dst.exists() and dst.is_dir():
        dst = dst / src.name
    else:
        dst.parent.mkdir(parents=True, exist_ok=True)

    shutil.copy2(src, dst)
    print(f'{src} 复制到 {dst}')
