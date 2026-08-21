#!/usr/bin/env python3
"""Render a (binary or ASCII) STL to a PNG preview using matplotlib.

Usage: python tools/render_stl.py <input.stl> <output.png> [--elev 25] [--azim -60]

Only third-party dependency is matplotlib (install in an isolated venv,
see tools/requirements-mdi.txt for the pattern). STL parsing is stdlib.
"""
import argparse
import struct
import sys


def load_stl_triangles(path):
    with open(path, "rb") as f:
        data = f.read()
    # ASCII STL starts with "solid" and contains "facet"; binary has an
    # 80-byte header then a uint32 triangle count.
    if data[:5] == b"solid" and b"facet" in data[:512]:
        tris = []
        cur = []
        for line in data.decode("ascii", "replace").splitlines():
            parts = line.split()
            if len(parts) == 4 and parts[0] == "vertex":
                cur.append([float(parts[1]), float(parts[2]), float(parts[3])])
                if len(cur) == 3:
                    tris.append(cur)
                    cur = []
        return tris
    (count,) = struct.unpack_from("<I", data, 80)
    tris = []
    off = 84
    for _ in range(count):
        vals = struct.unpack_from("<12f", data, off)  # normal + 3 vertices
        tris.append([list(vals[3:6]), list(vals[6:9]), list(vals[9:12])])
        off += 50
    return tris


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("stl")
    ap.add_argument("png")
    ap.add_argument("--elev", type=float, default=25.0)
    ap.add_argument("--azim", type=float, default=-60.0)
    ap.add_argument("--size", type=int, default=1200)
    args = ap.parse_args()

    tris = load_stl_triangles(args.stl)
    if not tris:
        sys.exit("no triangles parsed")

    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from mpl_toolkits.mplot3d.art3d import Poly3DCollection

    fig = plt.figure(figsize=(args.size / 150, args.size / 150), dpi=150)
    ax = fig.add_subplot(111, projection="3d")
    mesh = Poly3DCollection(tris, facecolor="#9db4c8", edgecolor="#33424f",
                            linewidth=0.15, alpha=1.0)
    ax.add_collection3d(mesh)

    xs = [v[0] for t in tris for v in t]
    ys = [v[1] for t in tris for v in t]
    zs = [v[2] for t in tris for v in t]
    cx, cy, cz = (min(xs) + max(xs)) / 2, (min(ys) + max(ys)) / 2, (min(zs) + max(zs)) / 2
    r = max(max(xs) - min(xs), max(ys) - min(ys), max(zs) - min(zs)) / 2
    ax.set_xlim(cx - r, cx + r)
    ax.set_ylim(cy - r, cy + r)
    ax.set_zlim(cz - r, cz + r)
    ax.set_box_aspect((1, 1, 1))
    ax.view_init(elev=args.elev, azim=args.azim)
    ax.set_axis_off()
    fig.subplots_adjust(0, 0, 1, 1)
    fig.savefig(args.png, transparent=True)
    print(f"{len(tris)} triangles -> {args.png}")


if __name__ == "__main__":
    main()
