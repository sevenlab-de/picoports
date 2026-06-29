#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (c) 2026 sevenlab engineering GmbH
#
"""Switch a PicoPorts device into USB firmware upgrade mode."""

from __future__ import annotations

import argparse
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


PICO_PORTS_VID = "a257"
PICO_PORTS_PID = "2013"
PICO_PORTS_DLN2_INTERFACE = "0"
PICO_PORTS_DLN2_OUT_ENDPOINT = "0x01"
FIRMWARE_UPGRADE_MAGIC_HEX = "4649524d5741524555504752414445"  # hex of FIRMWAREUPGRADE


@dataclass(frozen=True)
class UsbDevice:
    path: Path
    busnum: int
    devnum: int
    serial: str
    manufacturer: str
    product: str

    def __str__(self) -> str:
        serial = self.serial or "<no serial>"
        product = self.product or "<no product>"
        return ", ".join(
            [
                f"path = {self.path.name}",
                f"bus = {self.busnum}",
                f"dev = {self.devnum}",
                f"serial = {serial}",
                f"{product}",
            ]
        )


def read_sysfs_str(path: Path) -> str | None:
    try:
        return path.read_text(encoding="ascii").strip()
    except OSError:
        return None


def read_sysfs_int(path: Path) -> int | None:
    value = read_sysfs_str(path)
    if value is None:
        return None

    try:
        return int(value, 10)
    except ValueError:
        return None


def find_picoports_devices(sysfs_root: Path) -> list[UsbDevice] | None:
    devices: list[UsbDevice] = []

    try:
        paths = sysfs_root.iterdir()
    except OSError as exc:
        print(f"Cannot read USB sysfs root {sysfs_root}: {exc}", file=sys.stderr)
        return None

    for path in paths:
        vid = read_sysfs_str(path / "idVendor")
        pid = read_sysfs_str(path / "idProduct")
        if vid is None or pid is None:
            continue
        if vid.lower() != PICO_PORTS_VID or pid.lower() != PICO_PORTS_PID:
            continue

        busnum = read_sysfs_int(path / "busnum")
        devnum = read_sysfs_int(path / "devnum")
        if busnum is None or devnum is None:
            continue

        devices.append(
            UsbDevice(
                path=path,
                busnum=busnum,
                devnum=devnum,
                serial=read_sysfs_str(path / "serial") or "",
                manufacturer=read_sysfs_str(path / "manufacturer") or "",
                product=read_sysfs_str(path / "product") or "",
            )
        )

    return sorted(devices, key=lambda device: (device.busnum, device.devnum))


def select_device(devices: list[UsbDevice], serial: str | None) -> UsbDevice | None:
    if serial is not None:
        matches = [device for device in devices if device.serial == serial]
        if len(matches) == 1:
            return matches[0]
        elif len(matches) > 1:
            print(f"Multiple PicoPorts devices have serial {serial}", file=sys.stderr)
        else:
            print(f"No PicoPorts device with serial {serial} found", file=sys.stderr)
    else:
        if len(devices) == 1:
            return devices[0]
        elif len(devices) > 1:
            print(
                "Multiple PicoPorts devices found; pass a serial number",
                file=sys.stderr,
            )
        else:
            print("No PicoPorts devices found", file=sys.stderr)

    if devices:
        print("Detected PicoPorts devices:", file=sys.stderr)
        for device in devices:
            print(f"  {device}", file=sys.stderr)

    return None


def modeswitch_command(usb_modeswitch: str, device: UsbDevice) -> list[str]:
    return [
        usb_modeswitch,
        "-v",
        f"0x{PICO_PORTS_VID}",
        "-p",
        f"0x{PICO_PORTS_PID}",
        "-b",
        str(device.busnum),
        "-g",
        str(device.devnum),
        "-i",
        PICO_PORTS_DLN2_INTERFACE,
        "-m",
        PICO_PORTS_DLN2_OUT_ENDPOINT,
        "-M",
        FIRMWARE_UPGRADE_MAGIC_HEX,
    ]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "serial",
        nargs="?",
        help="PicoPorts USB serial number; optional when exactly one device exists",
    )
    parser.add_argument(
        "--list",
        action="store_true",
        help="list detected PicoPorts devices and exit",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="print the usb_modeswitch command instead of running it",
    )
    # for testing
    parser.add_argument(
        "--sysfs-root",
        default="/sys/bus/usb/devices",
        type=Path,
        help=argparse.SUPPRESS,
    )
    # for testing
    parser.add_argument(
        "--usb-modeswitch",
        default="usb_modeswitch",
        help=argparse.SUPPRESS,
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    devices = find_picoports_devices(args.sysfs_root)
    if devices is None:
        return 2  # ENOENT

    if args.list:
        for device in devices:
            print(device)

        return 0

    device = select_device(devices, args.serial)
    if device is None:
        return 19  # ENODEV

    command = modeswitch_command(args.usb_modeswitch, device)

    print(f"Switching PicoPorts device: {device}")
    if args.dry_run:
        print(" ".join(command))
        return 0

    try:
        return subprocess.run(command, check=False).returncode
    except FileNotFoundError:
        print(
            f"usb_modeswitch command not found: {args.usb_modeswitch}",
            file=sys.stderr,
        )
        return 2  # ENOENT


if __name__ == "__main__":
    raise SystemExit(main())
