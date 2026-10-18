# run_iland.py
# ------------------------------------------------------------
# Safe iLand runner using multiprocessing (Qt-safe)
# ------------------------------------------------------------
from __future__ import annotations
from numba import char

import ctypes
import multiprocessing as mp
from pathlib import Path
import os
import sys


# ------------------------------------------------------------
# INTERNAL: iLand execution (runs in child process ONLY)
# ------------------------------------------------------------
def _iland_worker(xml_path_bytes: bytes, years: int, dll_path: str, result_queue: mp.Queue):
    """
    This function runs INSIDE a separate process.
    Qt (QCoreApplication) is created and destroyed here safely.
    """
    try:
        # --- Load DLL inside the child process ---
        iland = ctypes.CDLL(dll_path)

        # --- Expected iLand C API ---
        # adjust names if needed
        iland.runilandmodel.argtypes = [ctypes.c_char_p, ctypes.c_int]
        iland.runilandmodel.restype = ctypes.c_int

        # --- Run model ---
        ret = iland.runilandmodel(xml_path_bytes, int(years))

        result_queue.put({
            "ok": True,
            "return_code": int(ret),
        })

    except Exception as e:
        result_queue.put({
            "ok": False,
            "error": str(e),
        })


# ------------------------------------------------------------
# PUBLIC API
# ------------------------------------------------------------
def run_iland(
    xml_path: bytes,
    years: int,
    *,
    bin_path: str | Path,
    timeout: int | None = None,
) -> dict:

    if not isinstance(xml_path, (bytes, bytearray)):
        raise TypeError("xml_path must be bytes (use str(path).encode('utf-8'))")

    DLL_NAME = "ILandModel.dll"

    bin_path = Path(bin_path)

    if not bin_path.exists():
        raise FileNotFoundError(f"iLand bin folder not found: {bin_path}")

    dll_path = bin_path / DLL_NAME

    if not dll_path.exists():
        raise FileNotFoundError(f"iLand DLL not found: {dll_path}")

    dll_path = str(dll_path.resolve())

    result_queue: mp.Queue = mp.Queue()

    proc = mp.Process(
        target=_iland_worker,
        args=(xml_path, years, dll_path, result_queue),
        daemon=False,
    )

    proc.start()
    proc.join(timeout=timeout)

    if proc.is_alive():
        proc.terminate()
        proc.join()
        return {
            "ok": False,
            "return_code": None,
            "error": "iLand process timeout",
        }

    if result_queue.empty():
        return {
            "ok": False,
            "return_code": None,
            "error": "No response from iLand process",
        }

    return result_queue.get()
