from __future__ import annotations

import json
import math
import random
import struct
from dataclasses import dataclass
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter


ROOT = Path(__file__).resolve().parent.parent
OUT_DIR = ROOT / "assets" / "models" / "rabbite"
GLTF_PATH = OUT_DIR / "rabbite.gltf"
BIN_PATH = OUT_DIR / "rabbite.bin"
FUR_TEX_PATH = OUT_DIR / "rabbite_fur_basecolor.png"
FACE_TEX_PATH = OUT_DIR / "rabbite_face_basecolor.png"
INNER_EAR_TEX_PATH = OUT_DIR / "rabbite_inner_ear_basecolor.png"


@dataclass
class MeshData:
    positions: list[float]
    normals: list[float]
    uvs: list[float]
    indices: list[int]


def clamp_channel(value: float) -> int:
    return max(0, min(255, int(round(value))))


def normalize3(x: float, y: float, z: float) -> tuple[float, float, float]:
    length = math.sqrt((x * x) + (y * y) + (z * z))
    if length <= 0.000001:
        return 0.0, 1.0, 0.0
    return x / length, y / length, z / length


def quat_from_euler(rx: float, ry: float, rz: float) -> list[float]:
    cx = math.cos(rx * 0.5)
    sx = math.sin(rx * 0.5)
    cy = math.cos(ry * 0.5)
    sy = math.sin(ry * 0.5)
    cz = math.cos(rz * 0.5)
    sz = math.sin(rz * 0.5)

    x = (sx * cy * cz) - (cx * sy * sz)
    y = (cx * sy * cz) + (sx * cy * sz)
    z = (cx * cy * sz) - (sx * sy * cz)
    w = (cx * cy * cz) + (sx * sy * sz)

    length = math.sqrt((x * x) + (y * y) + (z * z) + (w * w))
    if length <= 0.000001:
        return [0.0, 0.0, 0.0, 1.0]

    return [x / length, y / length, z / length, w / length]


def flatten_vec3(values: list[tuple[float, float, float]]) -> list[float]:
    out: list[float] = []
    for x, y, z in values:
        out.extend([x, y, z])
    return out


def flatten_vec4(values: list[list[float]]) -> list[float]:
    out: list[float] = []
    for x, y, z, w in values:
        out.extend([x, y, z, w])
    return out


def build_uv_sphere(lat_steps: int = 8, lon_steps: int = 12) -> MeshData:
    positions: list[float] = []
    normals: list[float] = []
    uvs: list[float] = []
    indices: list[int] = []

    for lat in range(lat_steps + 1):
        v = lat / lat_steps
        theta = v * math.pi
        sin_theta = math.sin(theta)
        cos_theta = math.cos(theta)

        for lon in range(lon_steps + 1):
            u = lon / lon_steps
            phi = u * math.tau
            cos_phi = math.cos(phi)
            sin_phi = math.sin(phi)

            x = 0.5 * sin_theta * cos_phi
            y = 0.5 * cos_theta
            z = 0.5 * sin_theta * sin_phi
            nx, ny, nz = normalize3(x, y, z)

            positions.extend([x, y, z])
            normals.extend([nx, ny, nz])
            uvs.extend([u, 1.0 - v])

    ring = lon_steps + 1
    for lat in range(lat_steps):
        for lon in range(lon_steps):
            a = (lat * ring) + lon
            b = a + ring
            c = b + 1
            d = a + 1
            indices.extend([a, b, d, d, b, c])

    return MeshData(positions=positions, normals=normals, uvs=uvs, indices=indices)


def build_box() -> MeshData:
    positions: list[float] = []
    normals: list[float] = []
    uvs: list[float] = []
    indices: list[int] = []

    faces = [
        ((1.0, 0.0, 0.0), [(0.5, -0.5, -0.5), (0.5, 0.5, -0.5), (0.5, 0.5, 0.5), (0.5, -0.5, 0.5)]),
        ((-1.0, 0.0, 0.0), [(-0.5, -0.5, 0.5), (-0.5, 0.5, 0.5), (-0.5, 0.5, -0.5), (-0.5, -0.5, -0.5)]),
        ((0.0, 1.0, 0.0), [(-0.5, 0.5, -0.5), (-0.5, 0.5, 0.5), (0.5, 0.5, 0.5), (0.5, 0.5, -0.5)]),
        ((0.0, -1.0, 0.0), [(-0.5, -0.5, 0.5), (-0.5, -0.5, -0.5), (0.5, -0.5, -0.5), (0.5, -0.5, 0.5)]),
        ((0.0, 0.0, 1.0), [(0.5, -0.5, 0.5), (0.5, 0.5, 0.5), (-0.5, 0.5, 0.5), (-0.5, -0.5, 0.5)]),
        ((0.0, 0.0, -1.0), [(-0.5, -0.5, -0.5), (-0.5, 0.5, -0.5), (0.5, 0.5, -0.5), (0.5, -0.5, -0.5)]),
    ]
    face_uv = [(0.0, 1.0), (0.0, 0.0), (1.0, 0.0), (1.0, 1.0)]

    for face_index, (normal, verts) in enumerate(faces):
        base = face_index * 4
        for vert, uv in zip(verts, face_uv):
            positions.extend(vert)
            normals.extend(normal)
            uvs.extend(uv)
        indices.extend([base, base + 1, base + 2, base, base + 2, base + 3])

    return MeshData(positions=positions, normals=normals, uvs=uvs, indices=indices)


def build_plane_yz() -> MeshData:
    positions = [
        0.0, -0.5, -0.5,
        0.0, 0.5, -0.5,
        0.0, 0.5, 0.5,
        0.0, -0.5, 0.5,
    ]
    normals = [
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
    ]
    uvs = [
        0.0, 1.0,
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
    ]
    indices = [0, 1, 2, 0, 2, 3]
    return MeshData(positions=positions, normals=normals, uvs=uvs, indices=indices)


class BufferBuilder:
    def __init__(self) -> None:
        self.data = bytearray()
        self.buffer_views: list[dict] = []
        self.accessors: list[dict] = []

    def _align4(self) -> None:
        while len(self.data) % 4 != 0:
            self.data.append(0)

    def _append_view(self, payload: bytes, target: int | None = None) -> int:
        self._align4()
        offset = len(self.data)
        self.data.extend(payload)

        view = {
            "buffer": 0,
            "byteOffset": offset,
            "byteLength": len(payload),
        }
        if target is not None:
            view["target"] = target

        index = len(self.buffer_views)
        self.buffer_views.append(view)
        return index

    @staticmethod
    def _component_min_max(values: list[float], width: int) -> tuple[list[float], list[float]]:
        mins = [float("inf")] * width
        maxs = [float("-inf")] * width

        for item_index in range(0, len(values), width):
            for component in range(width):
                value = float(values[item_index + component])
                mins[component] = min(mins[component], value)
                maxs[component] = max(maxs[component], value)

        rounded_mins = [round(value, 6) for value in mins]
        rounded_maxs = [round(value, 6) for value in maxs]
        return rounded_mins, rounded_maxs

    def add_float_accessor(
        self,
        values: list[float],
        width: int,
        accessor_type: str,
        target: int | None = None,
    ) -> int:
        payload = struct.pack(f"<{len(values)}f", *values)
        view_index = self._append_view(payload, target=target)
        count = len(values) // width
        mins, maxs = self._component_min_max(values, width)

        accessor = {
            "bufferView": view_index,
            "componentType": 5126,
            "count": count,
            "type": accessor_type,
            "min": mins,
            "max": maxs,
        }

        accessor_index = len(self.accessors)
        self.accessors.append(accessor)
        return accessor_index

    def add_u16_accessor(
        self,
        values: list[int],
        target: int | None = None,
    ) -> int:
        payload = struct.pack(f"<{len(values)}H", *values)
        view_index = self._append_view(payload, target=target)

        accessor = {
            "bufferView": view_index,
            "componentType": 5123,
            "count": len(values),
            "type": "SCALAR",
            "min": [min(values) if values else 0],
            "max": [max(values) if values else 0],
        }

        accessor_index = len(self.accessors)
        self.accessors.append(accessor)
        return accessor_index


def add_mesh(
    meshes: list[dict],
    builder: BufferBuilder,
    name: str,
    mesh_data: MeshData,
    material_index: int,
) -> int:
    position_accessor = builder.add_float_accessor(mesh_data.positions, 3, "VEC3", target=34962)
    normal_accessor = builder.add_float_accessor(mesh_data.normals, 3, "VEC3", target=34962)
    uv_accessor = builder.add_float_accessor(mesh_data.uvs, 2, "VEC2", target=34962)
    index_accessor = builder.add_u16_accessor(mesh_data.indices, target=34963)

    mesh = {
        "name": name,
        "primitives": [
            {
                "attributes": {
                    "POSITION": position_accessor,
                    "NORMAL": normal_accessor,
                    "TEXCOORD_0": uv_accessor,
                },
                "indices": index_accessor,
                "material": material_index,
            }
        ],
    }

    mesh_index = len(meshes)
    meshes.append(mesh)
    return mesh_index


def make_fur_texture(path: Path) -> None:
    size = 128
    image = Image.new("RGBA", (size, size))
    pixels = image.load()

    for y in range(size):
        for x in range(size):
            u = x / size
            v = y / size
            wave_a = math.sin(math.tau * u)
            wave_b = math.cos(math.tau * v)
            wave_c = math.sin(math.tau * (u + v) * 2.0)
            noise = (wave_a * 0.55) + (wave_b * 0.35) + (wave_c * 0.25)

            base_r = 230 + (noise * 10.0)
            base_g = 171 + (noise * 7.0)
            base_b = 194 + (noise * 8.0)

            pixels[x, y] = (
                clamp_channel(base_r),
                clamp_channel(base_g),
                clamp_channel(base_b),
                255,
            )

    draw = ImageDraw.Draw(image)
    draw.ellipse((16, 18, 118, 114), outline=(255, 215, 225, 45), width=3)

    rng = random.Random(7)
    for _ in range(90):
        px = rng.randint(0, size - 1)
        py = rng.randint(0, size - 1)
        radius = rng.randint(1, 3)
        tint = rng.randint(-16, 16)
        color = (
            clamp_channel(238 + tint),
            clamp_channel(182 + (tint // 2)),
            clamp_channel(204 + tint),
            80,
        )
        draw.ellipse((px - radius, py - radius, px + radius, py + radius), fill=color)

    image = image.filter(ImageFilter.GaussianBlur(radius=0.4))
    image.save(path)


def make_face_texture(path: Path) -> None:
    image = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)

    draw.ellipse((20, 28, 56, 70), fill=(255, 255, 255, 255))
    draw.ellipse((72, 28, 108, 70), fill=(255, 255, 255, 255))

    draw.ellipse((32, 40, 46, 58), fill=(36, 24, 36, 255))
    draw.ellipse((82, 40, 96, 58), fill=(36, 24, 36, 255))
    draw.ellipse((36, 44, 40, 48), fill=(255, 255, 255, 190))
    draw.ellipse((86, 44, 90, 48), fill=(255, 255, 255, 190))

    draw.ellipse((56, 58, 72, 72), fill=(238, 143, 168, 255))
    draw.line((64, 70, 64, 84), fill=(115, 73, 88, 255), width=3)
    draw.arc((46, 78, 64, 94), start=305, end=110, fill=(115, 73, 88, 255), width=3)
    draw.arc((64, 78, 82, 94), start=70, end=235, fill=(115, 73, 88, 255), width=3)

    draw.rectangle((57, 84, 63, 96), fill=(250, 248, 244, 255))
    draw.rectangle((65, 84, 71, 96), fill=(250, 248, 244, 255))

    draw.ellipse((12, 62, 26, 76), fill=(255, 180, 198, 180))
    draw.ellipse((102, 62, 116, 76), fill=(255, 180, 198, 180))

    image = image.filter(ImageFilter.GaussianBlur(radius=0.3))
    image.save(path)


def make_inner_ear_texture(path: Path) -> None:
    image = Image.new("RGBA", (128, 256), (0, 0, 0, 0))
    mask = Image.new("L", (128, 256), 0)
    mask_draw = ImageDraw.Draw(mask)
    mask_draw.rounded_rectangle((18, 18, 110, 238), radius=48, fill=235)
    mask = mask.filter(ImageFilter.GaussianBlur(radius=6.5))

    gradient = Image.new("RGBA", (128, 256), (0, 0, 0, 0))
    grad_pixels = gradient.load()
    for y in range(256):
        t = y / 255.0
        outer = (
            252 - (t * 9.0),
            228 - (t * 14.0),
            220 - (t * 10.0),
            255,
        )
        for x in range(128):
            grad_pixels[x, y] = (
                clamp_channel(outer[0]),
                clamp_channel(outer[1]),
                clamp_channel(outer[2]),
                outer[3],
            )

    image.alpha_composite(gradient)

    blush = Image.new("RGBA", (128, 256), (0, 0, 0, 0))
    blush_draw = ImageDraw.Draw(blush)
    blush_draw.rounded_rectangle((34, 26, 94, 226), radius=32, fill=(245, 162, 187, 230))
    blush = blush.filter(ImageFilter.GaussianBlur(radius=7.5))
    image.alpha_composite(blush)

    image.putalpha(mask)
    image.save(path)


def node(
    name: str,
    *,
    mesh: int | None = None,
    translation: list[float] | None = None,
    rotation: list[float] | None = None,
    scale: list[float] | None = None,
    children: list[int] | None = None,
) -> dict:
    out = {"name": name}
    if mesh is not None:
        out["mesh"] = mesh
    if translation is not None:
        out["translation"] = translation
    if rotation is not None:
        out["rotation"] = rotation
    if scale is not None:
        out["scale"] = scale
    if children:
        out["children"] = children
    return out


def add_animation(
    animations: list[dict],
    builder: BufferBuilder,
    name: str,
    channels: list[dict],
) -> None:
    animation_samplers: list[dict] = []
    animation_channels: list[dict] = []

    for channel in channels:
        path = channel["path"]
        times = channel["times"]
        values = channel["values"]
        node_index = channel["node"]

        if path == "rotation":
            output_accessor = builder.add_float_accessor(values, 4, "VEC4")
        else:
            output_accessor = builder.add_float_accessor(values, 3, "VEC3")

        input_accessor = builder.add_float_accessor(times, 1, "SCALAR")
        sampler_index = len(animation_samplers)
        animation_samplers.append(
            {
                "input": input_accessor,
                "output": output_accessor,
                "interpolation": "LINEAR",
            }
        )
        animation_channels.append(
            {
                "sampler": sampler_index,
                "target": {
                    "node": node_index,
                    "path": path,
                },
            }
        )

    animations.append(
        {
            "name": name,
            "samplers": animation_samplers,
            "channels": animation_channels,
        }
    )


def build_gltf() -> dict:
    builder = BufferBuilder()

    fur_material = 0
    face_material = 1
    inner_ear_material = 2

    meshes: list[dict] = []
    sphere_mesh = add_mesh(meshes, builder, "FurSphere", build_uv_sphere(), fur_material)
    box_mesh = add_mesh(meshes, builder, "OuterEarBox", build_box(), fur_material)
    face_mesh = add_mesh(meshes, builder, "FaceCard", build_plane_yz(), face_material)
    inner_ear_mesh = add_mesh(meshes, builder, "InnerEarCard", build_plane_yz(), inner_ear_material)

    nodes: list[dict] = [
        node("RabbiteRoot", children=[1]),
        node("Body", mesh=sphere_mesh, translation=[0.0, 0.65, 0.0], scale=[1.15, 0.85, 1.0], children=[2, 9, 11, 12, 13, 14]),
        node("HeadPivot", translation=[0.62, 0.30, 0.0], children=[3, 4, 5, 7]),
        node("Head", mesh=sphere_mesh, scale=[0.58, 0.52, 0.56]),
        node("Face", mesh=face_mesh, translation=[0.34, -0.02, 0.0], scale=[1.0, 0.48, 0.48]),
        node("EarL", mesh=box_mesh, translation=[0.04, 0.38, 0.24], rotation=quat_from_euler(0.08, 0.0, -0.24), scale=[0.16, 0.80, 0.10], children=[6]),
        node("EarLInner", mesh=inner_ear_mesh, translation=[0.52, 0.02, 0.0], scale=[1.0, 0.82, 0.64]),
        node("EarR", mesh=box_mesh, translation=[0.04, 0.38, -0.24], rotation=quat_from_euler(0.08, 0.0, 0.24), scale=[0.16, 0.80, 0.10], children=[8]),
        node("EarRInner", mesh=inner_ear_mesh, translation=[0.52, 0.02, 0.0], scale=[1.0, 0.82, 0.64]),
        node("TailPivot", translation=[-0.78, 0.05, 0.0], children=[10]),
        node("Tail", mesh=sphere_mesh, translation=[-0.22, 0.0, 0.0], scale=[0.24, 0.24, 0.24]),
        node("FootFL", mesh=sphere_mesh, translation=[0.44, -0.43, 0.23], scale=[0.24, 0.14, 0.24]),
        node("FootFR", mesh=sphere_mesh, translation=[0.44, -0.43, -0.23], scale=[0.24, 0.14, 0.24]),
        node("FootBL", mesh=sphere_mesh, translation=[-0.38, -0.43, 0.20], scale=[0.24, 0.14, 0.24]),
        node("FootBR", mesh=sphere_mesh, translation=[-0.38, -0.43, -0.20], scale=[0.24, 0.14, 0.24]),
    ]

    animations: list[dict] = []

    attack_times = [0.0, 0.22, 0.42, 0.68, 0.92, 1.12, 1.36, 1.60]
    attack_root_y = [0.0, 0.02, -0.03, 0.05, 0.21, 0.16, 0.04, 0.0]
    attack_root_rot = [0.22, -0.18, 0.26, -0.12, 0.0, 0.08, -0.04, 0.0]
    attack_body_scale = [
        (1.15, 0.82, 1.00),
        (1.18, 0.78, 1.02),
        (1.12, 0.88, 0.96),
        (1.20, 0.76, 1.04),
        (1.00, 1.05, 0.92),
        (0.96, 1.08, 0.90),
        (1.10, 0.90, 0.98),
        (1.15, 0.85, 1.00),
    ]
    attack_head_rot = [0.15, -0.20, 0.28, -0.34, 0.18, -0.45, 0.12, 0.0]
    attack_tail_rot = [0.0, -0.18, 0.30, -0.36, 0.82, 2.18, 0.60, 0.05]
    attack_ear_l = [0.10, -0.16, 0.12, -0.22, 0.18, -0.34, 0.10, 0.0]
    attack_ear_r = [-0.10, 0.16, -0.12, 0.22, -0.18, 0.34, -0.10, 0.0]

    add_animation(
        animations,
        builder,
        "tail_swing_attack",
        [
            {
                "node": 0,
                "path": "translation",
                "times": attack_times,
                "values": flatten_vec3([(0.0, y, 0.0) for y in attack_root_y]),
            },
            {
                "node": 0,
                "path": "rotation",
                "times": attack_times,
                "values": flatten_vec4([quat_from_euler(0.0, 0.0, angle) for angle in attack_root_rot]),
            },
            {
                "node": 1,
                "path": "scale",
                "times": attack_times,
                "values": flatten_vec3(attack_body_scale),
            },
            {
                "node": 2,
                "path": "rotation",
                "times": attack_times,
                "values": flatten_vec4([quat_from_euler(0.0, 0.0, angle) for angle in attack_head_rot]),
            },
            {
                "node": 9,
                "path": "rotation",
                "times": attack_times,
                "values": flatten_vec4([quat_from_euler(0.0, 0.0, angle) for angle in attack_tail_rot]),
            },
            {
                "node": 5,
                "path": "rotation",
                "times": attack_times,
                "values": flatten_vec4([quat_from_euler(0.08, 0.0, -0.24 + offset) for offset in attack_ear_l]),
            },
            {
                "node": 7,
                "path": "rotation",
                "times": attack_times,
                "values": flatten_vec4([quat_from_euler(0.08, 0.0, 0.24 + offset) for offset in attack_ear_r]),
            },
        ],
    )

    flop_times = [0.0, 0.28, 0.56, 0.84, 1.12, 1.40, 1.72, 2.00]
    flop_root_y = [0.0, 0.05, 0.0, 0.03, 0.0, 0.04, 0.0, 0.0]
    flop_root_rot = [0.55, -0.48, 0.62, -0.42, 0.52, -0.34, 0.18, 0.0]
    flop_body_scale = [
        (1.10, 0.90, 1.00),
        (1.20, 0.76, 1.04),
        (1.08, 0.94, 0.98),
        (1.16, 0.80, 1.02),
        (1.10, 0.90, 1.00),
        (1.18, 0.78, 1.03),
        (1.12, 0.88, 1.00),
        (1.15, 0.85, 1.00),
    ]
    flop_head_rot = [0.18, -0.55, 0.24, -0.42, 0.30, -0.25, 0.12, 0.0]
    flop_tail_rot = [0.0, 0.52, -0.64, 0.46, -0.50, 0.38, -0.16, 0.0]
    flop_ear_l = [0.24, -0.18, 0.30, -0.16, 0.22, -0.12, 0.06, 0.0]
    flop_ear_r = [-0.24, 0.18, -0.30, 0.16, -0.22, 0.12, -0.06, 0.0]

    add_animation(
        animations,
        builder,
        "ground_flop",
        [
            {
                "node": 0,
                "path": "translation",
                "times": flop_times,
                "values": flatten_vec3([(0.0, y, 0.0) for y in flop_root_y]),
            },
            {
                "node": 0,
                "path": "rotation",
                "times": flop_times,
                "values": flatten_vec4([quat_from_euler(0.0, 0.0, angle) for angle in flop_root_rot]),
            },
            {
                "node": 1,
                "path": "scale",
                "times": flop_times,
                "values": flatten_vec3(flop_body_scale),
            },
            {
                "node": 2,
                "path": "rotation",
                "times": flop_times,
                "values": flatten_vec4([quat_from_euler(0.0, 0.0, angle) for angle in flop_head_rot]),
            },
            {
                "node": 9,
                "path": "rotation",
                "times": flop_times,
                "values": flatten_vec4([quat_from_euler(0.0, 0.0, angle) for angle in flop_tail_rot]),
            },
            {
                "node": 5,
                "path": "rotation",
                "times": flop_times,
                "values": flatten_vec4([quat_from_euler(0.08, 0.0, -0.24 + offset) for offset in flop_ear_l]),
            },
            {
                "node": 7,
                "path": "rotation",
                "times": flop_times,
                "values": flatten_vec4([quat_from_euler(0.08, 0.0, 0.24 + offset) for offset in flop_ear_r]),
            },
        ],
    )

    gltf = {
        "asset": {
            "version": "2.0",
            "generator": "Codex Rabbite Builder",
        },
        "scene": 0,
        "scenes": [
            {
                "name": "RabbiteScene",
                "nodes": [0],
            }
        ],
        "nodes": nodes,
        "meshes": meshes,
        "samplers": [
            {
                "magFilter": 9729,
                "minFilter": 9987,
                "wrapS": 10497,
                "wrapT": 10497,
            }
        ],
        "images": [
            {"uri": FUR_TEX_PATH.name},
            {"uri": FACE_TEX_PATH.name},
            {"uri": INNER_EAR_TEX_PATH.name},
        ],
        "textures": [
            {"sampler": 0, "source": 0},
            {"sampler": 0, "source": 1},
            {"sampler": 0, "source": 2},
        ],
        "materials": [
            {
                "name": "RabbiteFur",
                "pbrMetallicRoughness": {
                    "baseColorTexture": {"index": 0},
                    "metallicFactor": 0.0,
                    "roughnessFactor": 1.0,
                },
                "extensions": {"KHR_materials_unlit": {}},
            },
            {
                "name": "RabbiteFace",
                "pbrMetallicRoughness": {
                    "baseColorTexture": {"index": 1},
                    "metallicFactor": 0.0,
                    "roughnessFactor": 1.0,
                },
                "extensions": {"KHR_materials_unlit": {}},
                "alphaMode": "BLEND",
                "doubleSided": True,
            },
            {
                "name": "RabbiteInnerEar",
                "pbrMetallicRoughness": {
                    "baseColorTexture": {"index": 2},
                    "metallicFactor": 0.0,
                    "roughnessFactor": 1.0,
                },
                "extensions": {"KHR_materials_unlit": {}},
                "alphaMode": "BLEND",
                "doubleSided": True,
            },
        ],
        "buffers": [
            {
                "uri": BIN_PATH.name,
                "byteLength": len(builder.data),
            }
        ],
        "bufferViews": builder.buffer_views,
        "accessors": builder.accessors,
        "animations": animations,
        "extensionsUsed": ["KHR_materials_unlit"],
    }

    BIN_PATH.write_bytes(builder.data)
    return gltf


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    make_fur_texture(FUR_TEX_PATH)
    make_face_texture(FACE_TEX_PATH)
    make_inner_ear_texture(INNER_EAR_TEX_PATH)

    gltf = build_gltf()
    GLTF_PATH.write_text(json.dumps(gltf, indent=2), encoding="utf-8")

    print(f"Wrote {GLTF_PATH}")
    print(f"Wrote {BIN_PATH}")
    print(f"Wrote {FUR_TEX_PATH}")
    print(f"Wrote {FACE_TEX_PATH}")
    print(f"Wrote {INNER_EAR_TEX_PATH}")


if __name__ == "__main__":
    main()
