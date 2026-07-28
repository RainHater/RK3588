#!/usr/bin/env python3
"""打包并校验 RKPlatform OTA 版本包。"""

from __future__ import annotations

import hashlib
import hmac
import json
import os
import subprocess
import tarfile
import tempfile
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT_DIR = Path(os.environ.get("ROOT_DIR"))
BUILD_DIR = Path(os.environ.get("BUILD_DIR"))
INSTALL_DIR = Path(os.environ.get("INSTALL_DIR"))
HASH_CHUNK_SIZE = 1024 * 1024

def get_version(app_path: Path) -> str:
    """执行可执行程序的 --version 参数并获取版本号。"""

    result = subprocess.run(
        [str(app_path.resolve()), "--version"],
        capture_output=True,
        text=True,
        check=True,
    )
    version = result.stdout.strip()

    return version


def calculate_file_checksum(file_path: Path) -> str:
    """分块读取单个文件并计算 SHA-256 校验值。"""

    digest = hashlib.sha256()
    with file_path.open("rb") as file:
        while chunk := file.read(HASH_CHUNK_SIZE):
            digest.update(chunk)
    return digest.hexdigest()


def calculate_total_checksum(target_dir: Path) -> str:
    """计算 bin 和 lib 中全部普通文件的组合 SHA-256 校验值。"""

    total_digest = hashlib.sha256()

    for directory_name in ("bin", "lib"):
        directory = target_dir / directory_name

        if not directory.is_dir():
            print(f'目录不存在: {directory.resolve()}')
            continue

        for file_path in sorted(directory.rglob("*")):
            if not file_path.is_file() or file_path.is_symlink():
                continue

            relative_path = file_path.relative_to(target_dir).as_posix()
            # 路径也参与计算，避免文件改名后校验值保持不变
            total_digest.update(relative_path.encode("utf-8"))
            total_digest.update(b"\0")

            with file_path.open("rb") as file:
                while chunk := file.read(HASH_CHUNK_SIZE):
                    total_digest.update(chunk)

    return total_digest.hexdigest()


def create_manifest(
    app_path: Path,
    target_dir: Path,
    output_dir: Path,
) -> Path:
    """生成包含版本号和 bin/lib 文件校验信息的 manifest.json。"""

    if not app_path.is_file():
        raise FileNotFoundError(f"可执行程序不存在: {app_path.resolve()}")

    entries: list[dict[str, Any]] = []
    total_size = 0

    for directory_name in ("bin", "lib"):
        directory = target_dir / directory_name
        if not directory.is_dir():
            raise FileNotFoundError(f"目录不存在: {directory.resolve()}")

        for file_path in sorted(directory.rglob("*")):
            relative_path = file_path.relative_to(target_dir).as_posix()

            if file_path.is_symlink():
                entries.append(
                    {
                        "path": relative_path,
                        "type": "symlink",
                        "target": os.readlink(file_path),
                    }
                )
            elif file_path.is_file():
                size = file_path.stat().st_size
                entries.append(
                    {
                        "path": relative_path,
                        "type": "file",
                        "size": size,
                        "mode": f"{file_path.stat().st_mode & 0o7777:04o}",
                        "sha256": calculate_file_checksum(file_path),
                    }
                )
                total_size += size

    manifest = {
        "format": "rkplatform-ota",
        "format_version": 1,
        "program": app_path.name,
        "version": get_version(app_path),
        "created_at": datetime.now(timezone.utc).isoformat(),
        "checksum_algorithm": "sha256",
        "payload_checksum": calculate_total_checksum(target_dir),
        "file_count": sum(entry["type"] == "file" for entry in entries),
        "total_size": total_size,
        "entries": entries,
    }

    output_dir.mkdir(parents=True, exist_ok=True)
    manifest_path = output_dir / "manifest.json"
    manifest_path.write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    return manifest_path


def compress_ota_package(
    target_dir: Path,
    manifest_path: Path,
    output_dir: Path,
) -> tuple[Path, Path]:
    """将 manifest.json、bin 和 lib 压缩为 OTA 包并生成整包校验文件。"""

    if not manifest_path.is_file():
        raise FileNotFoundError(f"OTA 清单不存在: {manifest_path.resolve()}")

    for directory_name in ("bin", "lib"):
        directory = target_dir / directory_name
        if not directory.is_dir():
            raise FileNotFoundError(f"打包目录不存在: {directory.resolve()}")

    with manifest_path.open("r", encoding="utf-8") as file:
        manifest = json.load(file)

    program = manifest.get("program")
    version = manifest.get("version")
    if not program or not version:
        raise ValueError("OTA 清单缺少 program 或 version")

    output_dir.mkdir(parents=True, exist_ok=True)
    package_path = output_dir / f"{program}-{version}-ota.tar.gz"
    checksum_path = Path(f"{package_path}.sha256")

    file_descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{package_path.name}.",
        suffix=".tmp",
        dir=output_dir,
    )
    os.close(file_descriptor)
    temporary_path = Path(temporary_name)

    try:
        with tarfile.open(temporary_path, "w:gz") as archive:
            archive.add(manifest_path, arcname="manifest.json")
            archive.add(target_dir / "bin", arcname="bin", recursive=True)
            archive.add(target_dir / "lib", arcname="lib", recursive=True)
        temporary_path.replace(package_path)
    finally:
        temporary_path.unlink(missing_ok=True)

    package_checksum = calculate_file_checksum(package_path)
    checksum_path.write_text(
        f"{package_checksum}  {package_path.name}\n",
        encoding="ascii",
    )
    return package_path, checksum_path


def verify_ota_package(
    package_path: Path,
    checksum_path: Path | None = None,
) -> bool:
    """使用 .sha256 文件校验 OTA 压缩包，并检查必要的包内文件。"""

    if not package_path.is_file():
        raise FileNotFoundError(f"OTA 压缩包不存在: {package_path.resolve()}")

    if checksum_path is None:
        checksum_path = Path(f"{package_path}.sha256")
    if not checksum_path.is_file():
        raise FileNotFoundError(f"OTA 校验文件不存在: {checksum_path.resolve()}")

    checksum_parts = checksum_path.read_text(encoding="ascii").split()
    if not checksum_parts:
        raise ValueError(f"OTA 校验文件内容为空: {checksum_path.resolve()}")

    expected_checksum = checksum_parts[0].lower()
    if len(expected_checksum) != 64 or any(
        character not in "0123456789abcdef" for character in expected_checksum
    ):
        raise ValueError(f"OTA 校验文件格式错误: {checksum_path.resolve()}")

    actual_checksum = calculate_file_checksum(package_path)
    if not hmac.compare_digest(actual_checksum, expected_checksum):
        raise ValueError(
            "OTA 压缩包校验失败: "
            f"期望 {expected_checksum}，实际 {actual_checksum}"
        )

    with tarfile.open(package_path, "r:gz") as archive:
        member_names = set(archive.getnames())
        required_members = {"manifest.json", "bin", "lib"}
        missing_members = required_members - member_names
        if missing_members:
            missing_text = ", ".join(sorted(missing_members))
            raise ValueError(f"OTA 压缩包缺少必要内容: {missing_text}")

        manifest_file = archive.extractfile("manifest.json")
        if manifest_file is None:
            raise ValueError("无法读取 OTA 压缩包中的 manifest.json")
        json.load(manifest_file)

    return True


if __name__ == "__main__":
    app_path = INSTALL_DIR / "bin" / "app"
    version = get_version(app_path)
    ota_output_dir = BUILD_DIR / "ota" / version
    manifest_path = create_manifest(app_path, INSTALL_DIR, ota_output_dir)
    print(f"OTA 清单: {manifest_path}")

    package_path, checksum_path = compress_ota_package(
        INSTALL_DIR,
        manifest_path,
        ota_output_dir,
    )
    print(f"OTA 压缩包: {package_path}")
    print(f"OTA 校验文件: {checksum_path}")

    verify_ota_package(package_path, checksum_path)
    print("OTA 压缩包校验通过")
