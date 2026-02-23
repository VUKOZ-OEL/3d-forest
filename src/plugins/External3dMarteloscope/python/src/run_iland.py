# run_iland.py
# ------------------------------------------------------------
# Safe iLand runner using multiprocessing (Qt-safe)
# ------------------------------------------------------------

from __future__ import annotations

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
def run_iland(xml_path: bytes, years: int, *, timeout: int | None = None) -> dict:
    """
    Run iLand safely in a separate process.

    Parameters
    ----------
    xml_path : bytes
        UTF-8 encoded path to iLand XML
        e.g. str(path).encode("utf-8")
    years : int
        Number of simulation years
    timeout : int | None
        Optional timeout in seconds

    Returns
    -------
    dict
        {
            "ok": bool,
            "return_code": int | None,
            "error": str | None
        }
    """

    if not isinstance(xml_path, (bytes, bytearray)):
        raise TypeError("xml_path must be bytes (use str(path).encode('utf-8'))")

    # --- Locate DLL (adjust if needed) ---
    DLL_NAME = "iland.dll"

    if getattr(sys, "frozen", False):
        base_dir = Path(sys.executable).parent
    else:
        base_dir = Path(__file__).resolve().parent

    dll_path = str((base_dir / DLL_NAME).resolve()) # CHECK CORRECT PATH
    #dll_path = "C:/Users/krucek/Documents/GitHub/VUK/3d-forest/out/install/x64-Debug/bin/ILandModel.dll"

    if not os.path.exists(dll_path):
        raise FileNotFoundError(f"iLand DLL not found: {dll_path}")

    # --- IPC queue ---
    result_queue: mp.Queue = mp.Queue()

    # --- Spawn process ---
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
