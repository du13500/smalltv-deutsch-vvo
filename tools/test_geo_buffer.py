#!/usr/bin/env python3
"""Compatibility entry point for the real-framework Geo regression test."""
from pathlib import Path
import runpy
runpy.run_path(str(Path(__file__).with_name("test_geo_transfer.py")),run_name="__main__")
