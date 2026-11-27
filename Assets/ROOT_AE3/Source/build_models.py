# Builds the Advanced Equipment device models in Blender and exports one FBX per model.
# Run inside Blender (Text Editor or blender --background --python build_models.py [-- model ...]).
# Model names after "--" build only those (laptop, router, usb, battery, solar, server, cable, segment).
# Conventions: meters, Z up in Blender (exported Y up), origin at the bottom center,
# collision meshes prefixed UCX_ (convex), screen material AE3_Screen uses UV 0-1 for the render target.
import bpy
import bmesh
import math
import os
import sys
from mathutils import Vector

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")


def clear():
    for o in list(bpy.data.objects):
        bpy.data.objects.remove(o, do_unlink=True)
    for c in list(bpy.data.collections):
        bpy.data.collections.remove(c)
    for m in list(bpy.data.meshes):
        bpy.data.meshes.remove(m)


def mat(name, rgb, metal=0.0, rough=0.6, emit=None):
    m = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    m.use_nodes = True
    b = m.node_tree.nodes.get("Principled BSDF")
    b.inputs["Base Color"].default_value = (*rgb, 1)
    b.inputs["Metallic"].default_value = metal
    b.inputs["Roughness"].default_value = rough
    if emit:
        b.inputs["Emission Color"].default_value = (*emit, 1)
        b.inputs["Emission Strength"].default_value = 2.0
    return m


def materials():
    return {
        "plastic": mat("AE3_Plastic_Dark", (0.035, 0.037, 0.04), 0.0, 0.55),
        "olive": mat("AE3_Plastic_Olive", (0.12, 0.13, 0.08), 0.0, 0.6),
        "metal": mat("AE3_Metal", (0.55, 0.55, 0.57), 1.0, 0.35),
        "keys": mat("AE3_Keys", (0.015, 0.015, 0.017), 0.0, 0.7),
        "screen": mat("AE3_Screen", (0.01, 0.01, 0.012), 0.0, 0.1),
        "led": mat("AE3_LED", (0.1, 0.8, 0.2), 0.0, 0.3, emit=(0.1, 1, 0.2)),
        "cells": mat("AE3_SolarCells", (0.02, 0.05, 0.15), 0.6, 0.15),
        "rubber": mat("AE3_Rubber", (0.02, 0.02, 0.02), 0.0, 0.9),
        # variant colour: bumpers, stripes, logo plates (orange / coyote / ranger green per prefab variant)
        "accent": mat("AE3_Accent", (0.9, 0.35, 0.05), 0.0, 0.5),
        "ledb": mat("AE3_LED_Blue", (0.1, 0.4, 1.0), 0.0, 0.3, emit=(0.1, 0.4, 1.0)),
    }


def new_coll(name):
    c = bpy.data.collections.new(name)
    bpy.context.scene.collection.children.link(c)
    return c


def box(name, size, loc, material, col, bevel=0.0, rot=(0, 0, 0)):
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    for v in bm.verts:
        v.co = Vector((v.co.x * size[0], v.co.y * size[1], v.co.z * size[2]))
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    bm.free()
    ob = bpy.data.objects.new(name, me)
    ob.location = loc
    ob.rotation_euler = rot
    col.objects.link(ob)
    if material:
        me.materials.append(material)
    if bevel > 0:
        mod = ob.modifiers.new("Bevel", 'BEVEL')
        mod.width = bevel
        mod.segments = 2
        mod.limit_method = 'ANGLE'
    return ob


def cyl(name, r, depth, loc, material, col, rot=(0, 0, 0), verts=16):
    bm = bmesh.new()
    bmesh.ops.create_cone(bm, cap_ends=True, segments=verts, radius1=r, radius2=r, depth=depth)
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    bm.free()
    ob = bpy.data.objects.new(name, me)
    ob.location = loc
    ob.rotation_euler = rot
    col.objects.link(ob)
    me.materials.append(material)
    return ob


def torus(name, major, minor, loc, material, col, rot=(0, 0, 0), seg=32, ring=8):
    """Torus around the Z axis."""
    verts, faces = [], []
    for i in range(seg):
        a = i * 2 * math.pi / seg
        for j in range(ring):
            b = j * 2 * math.pi / ring
            r = major + minor * math.cos(b)
            verts.append((r * math.cos(a), r * math.sin(a), minor * math.sin(b)))
    for i in range(seg):
        for j in range(ring):
            i2, j2 = (i + 1) % seg, (j + 1) % ring
            faces.append((i * ring + j, i2 * ring + j, i2 * ring + j2, i * ring + j2))
    me = bpy.data.meshes.new(name)
    me.from_pydata(verts, [], faces)
    ob = bpy.data.objects.new(name, me)
    ob.location = loc
    ob.rotation_euler = rot
    col.objects.link(ob)
    me.materials.append(material)
    return ob


def plane(name, w, h, loc, material, col, rot=(0, 0, 0)):
    """Single quad with UV 0-1, facing +Z before rotation."""
    me = bpy.data.meshes.new(name)
    verts = [(-w / 2, -h / 2, 0), (w / 2, -h / 2, 0), (w / 2, h / 2, 0), (-w / 2, h / 2, 0)]
    me.from_pydata(verts, [], [(0, 1, 2, 3)])
    me.uv_layers.new(name="UVMap")
    uv = me.uv_layers.active.data
    for i, c in enumerate([(0, 0), (1, 0), (1, 1), (0, 1)]):
        uv[i].uv = c
    ob = bpy.data.objects.new(name, me)
    ob.location = loc
    ob.rotation_euler = rot
    col.objects.link(ob)
    me.materials.append(material)
    return ob


def join(objs, name):
    bpy.ops.object.select_all(action='DESELECT')
    for o in objs:
        bpy.context.view_layer.objects.active = o
        o.select_set(True)
        for m in list(o.modifiers):
            bpy.ops.object.modifier_apply(modifier=m.name)
        o.select_set(False)
    for o in objs:
        o.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]
    bpy.ops.object.join()
    o = bpy.context.view_layer.objects.active
    o.name = name
    o.data.name = name
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    return o


def uv_box(o):
    """Box-projected UVs for everything except the screen quad."""
    bpy.ops.object.select_all(action='DESELECT')
    o.select_set(True)
    bpy.context.view_layer.objects.active = o
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.cube_project(cube_size=0.5)
    bpy.ops.object.mode_set(mode='OBJECT')


def collider(name, size, loc, col, rot=(0, 0, 0)):
    return box("UCX_" + name, size, loc, None, col, rot=rot)


def add_lods(col, ratio):
    """Visual meshes become <name>_LOD0; a copy <name>_LOD1 decimated to ratio of the faces (none at 1) is added."""
    for o in [o for o in col.objects if o.type == 'MESH' and not o.name.startswith("UCX_")]:
        base = o.name
        o.name = base + "_LOD0"
        lod = o.copy()
        lod.data = o.data.copy()
        lod.name = base + "_LOD1"
        col.objects.link(lod)
        if ratio >= 1:
            continue
        bpy.ops.object.select_all(action='DESELECT')
        bpy.context.view_layer.objects.active = lod
        lod.select_set(True)
        mod = lod.modifiers.new("decimate", 'DECIMATE')
        mod.ratio = ratio
        bpy.ops.object.modifier_apply(modifier=mod.name)


def export(col, name, lod_ratio=0.4):
    path = os.path.join(OUT, name)
    os.makedirs(path, exist_ok=True)
    add_lods(col, lod_ratio)
    bpy.ops.object.select_all(action='DESELECT')
    for o in col.objects:
        o.select_set(True)
    bpy.ops.export_scene.fbx(filepath=os.path.join(path, name + ".fbx"), use_selection=True, apply_unit_scale=True,
                             axis_forward='-Z', axis_up='Y', mesh_smooth_type='FACE', add_leaf_bones=False,
                             bake_anim=False, object_types={'MESH'})


def laptop(M, open_lid):
    name = "ROOT_AE3_Laptop_Open" if open_lid else "ROOT_AE3_Laptop_Closed"
    col = new_coll(name)
    W, D, T = 0.34, 0.24, 0.022
    parts = [box("base", (W, D, T), (0, 0, T / 2), M["plastic"], col, bevel=0.003)]
    # keyboard
    kw, kd = 0.018, 0.016
    for r in range(5):
        for c in range(14):
            x = -W / 2 + 0.032 + c * (kw + 0.0035)
            y = 0.018 + r * (kd + 0.0035)
            parts.append(box("k", (kw, kd, 0.002), (x + kw / 2, -D / 2 + y + 0.05, T + 0.001), M["keys"], col))
    parts.append(box("pad", (0.09, 0.055, 0.0008), (0, -D / 2 + 0.035, T + 0.0004), M["keys"], col))
    parts.append(box("led", (0.004, 0.004, 0.001), (W / 2 - 0.02, -D / 2 + 0.01, T + 0.0005), M["led"], col))
    parts.append(box("ledb", (0.004, 0.004, 0.001), (W / 2 - 0.03, -D / 2 + 0.01, T + 0.0005), M["ledb"], col))
    for sy in (-1, 1):
        parts.append(box("stripe", (W - 0.05, 0.003, T * 0.5), (0, sy * (D / 2 + 0.0012), T / 2), M["accent"], col))
    for sx in (-1, 1):
        parts.append(box("stripe", (0.003, D - 0.05, T * 0.5), (sx * (W / 2 + 0.0012), 0.03, T / 2), M["accent"], col))
    for sx in (-1, 1):
        for sy in (-1, 1):
            parts.append(box("bumper", (0.026, 0.026, T + 0.002), (sx * (W / 2 - 0.011), sy * (D / 2 - 0.011), T / 2), M["accent"], col, bevel=0.003))
    for sx in (-1, 1):
        parts.append(box("usb", (0.002, 0.012, 0.005), (sx * (W / 2 + 0.0028), 0.0, T / 2), M["metal"], col))
        parts.append(box("foot", (0.03, 0.01, 0.002), (sx * 0.12, -0.09, -0.001), M["rubber"], col))
        parts.append(box("foot", (0.03, 0.01, 0.002), (sx * 0.12, 0.09, -0.001), M["rubber"], col))
    lt = 0.012
    if open_lid:
        ang = math.radians(-105)
        pivot = Vector((0, D / 2, T))
        lid = box("lid", (W, lt, D), (0, 0, 0), M["plastic"], col, bevel=0.003)
        hinge = cyl("hinge", 0.006, W * 0.8, (0, D / 2 - 0.004, T), M["metal"], col, rot=(0, math.radians(90), 0))
        parts.append(hinge)
        # lid stands up from the hinge, tilted back by 15 degrees
        tilt = math.radians(15)
        cy = D / 2 + math.sin(tilt) * D / 2
        cz = T + math.cos(tilt) * D / 2
        lid.location = (0, cy, cz)
        lid.rotation_euler = (-tilt, 0, 0)
        parts.append(lid)
        back = lt / 2 + 0.0008
        parts.append(box("logo", (0.24, 0.001, 0.15), (0, cy + math.cos(tilt) * back, cz - math.sin(tilt) * back), M["accent"], col, rot=(-tilt, 0, 0)))
        off = lt / 2 + 0.0006
        screen = plane("AE3_ScreenQuad", 0.3, 0.1875, (0, cy - math.cos(tilt) * off, cz + math.sin(tilt) * off), M["screen"], col, rot=(math.radians(90) - tilt, 0, 0))
        o = join(parts, name)
        uv_box(o)
        # attach screen after UV unwrap so it keeps its 0-1 UVs
        bpy.ops.object.select_all(action='DESELECT')
        screen.select_set(True)
        o.select_set(True)
        bpy.context.view_layer.objects.active = o
        bpy.ops.object.join()
        o = bpy.context.view_layer.objects.active
        o.name = name
        collider(name + "_base", (W, D, T), (0, 0, T / 2), col)
        collider(name + "_lid", (W, lt, D), (0, cy, cz), col, rot=(-tilt, 0, 0))
    else:
        parts.append(box("lid", (W, D, lt), (0, 0, T + lt / 2 + 0.001), M["plastic"], col, bevel=0.003))
        parts.append(box("logo", (0.24, 0.15, 0.001), (0, 0, T + lt + 0.0015), M["accent"], col))
        for sx in (-1, 1):
            for sy in (-1, 1):
                parts.append(box("lidbumper", (0.026, 0.026, lt + 0.002), (sx * (W / 2 - 0.011), sy * (D / 2 - 0.011), T + lt / 2 + 0.001), M["accent"], col, bevel=0.003))
        o = join(parts, name)
        uv_box(o)
        collider(name, (W, D, T + lt), (0, 0, (T + lt) / 2), col)
    export(col, name)
    return o


def router(M):
    name = "ROOT_AE3_Router"
    col = new_coll(name)
    parts = [box("body", (0.2, 0.14, 0.032), (0, 0, 0.016), M["olive"], col, bevel=0.004),
             box("band", (0.204, 0.144, 0.012), (0, 0, 0.008), M["accent"], col),
             box("logo", (0.15, 0.09, 0.001), (0, 0, 0.0325), M["accent"], col)]
    for i in range(4):
        parts.append(box("led", (0.006, 0.003, 0.003), (-0.06 + i * 0.02, -0.0705, 0.022), M["led"] if i % 2 == 0 else M["ledb"], col))
    for sx in (-1, 1):
        parts.append(cyl("antenna", 0.005, 0.16, (sx * 0.08, 0.06, 0.11), M["rubber"], col))
        parts.append(cyl("cap", 0.0065, 0.02, (sx * 0.08, 0.06, 0.19), M["accent"], col))
        parts.append(cyl("joint", 0.008, 0.02, (sx * 0.08, 0.06, 0.035), M["metal"], col))
    for i in range(4):
        parts.append(box("port", (0.012, 0.004, 0.009), (-0.05 + i * 0.025, 0.0705, 0.014), M["metal"], col))
    o = join(parts, name)
    uv_box(o)
    collider(name, (0.2, 0.14, 0.032), (0, 0, 0.016), col)
    export(col, name)


def usb(M):
    name = "ROOT_AE3_UsbStick"
    col = new_coll(name)
    parts = [box("body", (0.045, 0.018, 0.008), (0, 0, 0.004), M["accent"], col, bevel=0.0015),
             box("grip", (0.02, 0.0185, 0.0085), (-0.008, 0, 0.004), M["plastic"], col),
             box("plug", (0.012, 0.012, 0.0045), (0.0285, 0, 0.004), M["metal"], col),
             box("ring", (0.006, 0.004, 0.003), (-0.025, 0, 0.004), M["metal"], col)]
    o = join(parts, name)
    uv_box(o)
    collider(name, (0.057, 0.018, 0.008), (0.006, 0, 0.004), col)
    export(col, name)


def battery(M):
    name = "ROOT_AE3_BatteryPack"
    col = new_coll(name)
    parts = [box("body", (0.32, 0.2, 0.16), (0, 0, 0.08), M["olive"], col, bevel=0.008),
             box("lidtrim", (0.33, 0.21, 0.012), (0, 0, 0.16), M["plastic"], col, bevel=0.003),
             box("handle", (0.2, 0.025, 0.02), (0, 0, 0.215), M["accent"], col, bevel=0.004),
             box("band", (0.324, 0.204, 0.05), (0, 0, 0.07), M["accent"], col)]
    for sx in (-1, 1):
        parts.append(box("post", (0.02, 0.02, 0.045), (sx * 0.09, 0, 0.19), M["plastic"], col))
        parts.append(cyl("terminal", 0.012, 0.02, (sx * 0.13, 0.06, 0.176), M["metal"], col))
    parts.append(box("gauge", (0.06, 0.004, 0.025), (0, -0.101, 0.11), M["led"], col))
    o = join(parts, name)
    uv_box(o)
    collider(name, (0.33, 0.21, 0.235), (0, 0, 0.1175), col)
    export(col, name)


def solar(M):
    name = "ROOT_AE3_SolarPanel"
    col = new_coll(name)
    tilt = math.radians(35)
    pw, ph, pt = 1.0, 0.65, 0.035
    cz = 0.42
    parts = [box("frame", (pw, ph, pt), (0, 0, cz), M["plastic"], col, rot=(tilt, 0, 0)),
             # cells 0.5 mm above the frame top, offset along the tilted panel normal
             box("cells", (pw - 0.04, ph - 0.04, 0.004), (0, -math.sin(tilt) * (pt / 2 + 0.0025), cz + math.cos(tilt) * (pt / 2 + 0.0025)), M["cells"], col, rot=(tilt, 0, 0))]
    for sx in (-1, 1):
        parts.append(box("legfront", (0.03, 0.03, 0.26), (sx * 0.42, -0.2, 0.13), M["plastic"], col))
        parts.append(box("legback", (0.03, 0.03, 0.58), (sx * 0.42, 0.22, 0.29), M["plastic"], col))
        parts.append(box("skid", (0.04, 0.6, 0.02), (sx * 0.42, 0.0, 0.01), M["metal"], col))
    parts.append(box("junction", (0.12, 0.05, 0.08), (0, 0.24, 0.3), M["accent"], col))
    # accent rim along the lower panel edge
    parts.append(box("rim", (pw, 0.03, pt + 0.004), (0, -math.cos(tilt) * (ph / 2 - 0.015), cz - math.sin(tilt) * (ph / 2 - 0.015)), M["accent"], col, rot=(tilt, 0, 0)))
    for sx in (-1, 1):
        parts.append(box("footcap", (0.05, 0.05, 0.03), (sx * 0.42, -0.3, 0.015), M["accent"], col))
        parts.append(box("footcap", (0.05, 0.05, 0.03), (sx * 0.42, 0.3, 0.015), M["accent"], col))
    o = join(parts, name)
    uv_box(o)
    collider(name + "_panel", (pw, ph, pt), (0, 0, cz), col, rot=(tilt, 0, 0))
    collider(name + "_base", (0.9, 0.6, 0.3), (0, 0, 0.15), col)
    export(col, name, 1.0)


def server(M):
    name = "ROOT_AE3_PortableServer"
    col = new_coll(name)
    W, D, H = 0.55, 0.48, 0.3
    parts = [box("case", (W, D, H), (0, 0, H / 2), M["olive"], col, bevel=0.012),
             box("frontpanel", (W - 0.06, 0.006, H - 0.06), (0, -D / 2 - 0.002, H / 2), M["plastic"], col),
             box("topplate", (W - 0.16, D - 0.16, 0.004), (0, 0, H + 0.002), M["accent"], col)]
    for i in range(3):
        parts.append(box("bay", (W - 0.12, 0.004, 0.05), (0, -D / 2 - 0.006, 0.07 + i * 0.075), M["keys"], col))
        parts.append(box("led", (0.008, 0.003, 0.008), (W / 2 - 0.08, -D / 2 - 0.008, 0.07 + i * 0.075), M["led"], col))
        parts.append(box("ledb", (0.008, 0.003, 0.008), (W / 2 - 0.095, -D / 2 - 0.008, 0.07 + i * 0.075), M["ledb"], col))
    for sx in (-1, 1):
        parts.append(box("handle", (0.02, 0.2, 0.03), (sx * (W / 2 + 0.01), 0, H - 0.04), M["metal"], col))
        parts.append(box("latch", (0.06, 0.02, 0.04), (sx * 0.15, -D / 2 - 0.006, H - 0.03), M["metal"], col))
        for sy in (-1, 1):
            for sz in (0.03, H - 0.03):
                parts.append(box("corner", (0.07, 0.07, 0.07), (sx * (W / 2 - 0.02), sy * (D / 2 - 0.02), sz), M["accent"], col, bevel=0.01))
    o = join(parts, name)
    uv_box(o)
    collider(name, (W + 0.04, D + 0.02, H), (0, 0, H / 2), col)
    export(col, name)


def cable(M):
    """Coiled network cable with a strap (accent) and an RJ45 plug on a short tail."""
    name = "ROOT_AE3_NetworkCable"
    col = new_coll(name)
    r = 0.0035
    parts = []
    for i, (dx, dy) in enumerate([(0, 0), (0.004, 0.003), (-0.003, 0.004), (0.002, -0.004)]):
        parts.append(torus("loop", 0.075, r, (dx, dy, r + i * 0.0055), M["plastic"], col, rot=(0, 0, i * 0.7)))
    parts.append(box("strap", (0.03, 0.03, 0.03), (0.075, 0, 0.012), M["accent"], col, bevel=0.003))
    parts.append(cyl("tail", r, 0.05, (0.075, -0.035, 0.004), M["plastic"], col, rot=(math.radians(90), 0, 0), verts=8))
    parts.append(box("boot", (0.012, 0.016, 0.01), (0.075, -0.066, 0.006), M["plastic"], col))
    parts.append(box("plug", (0.012, 0.014, 0.008), (0.075, -0.081, 0.006), M["metal"], col))
    o = join(parts, name)
    uv_box(o)
    collider(name, (0.17, 0.17, 0.03), (0, 0, 0.015), col)
    export(col, name)


def cable_segment(M):
    """Cable piece for cable lines: 0.25 m from the origin along Reforger +Z (Blender -Y), radius 5 mm, no collider."""
    name = "ROOT_AE3_CableSegment"
    col = new_coll(name)
    o = cyl("segment", 0.005, 0.25, (0, -0.125, 0), M["plastic"], col, rot=(math.radians(90), 0, 0), verts=8)
    o.name = name
    uv_box(o)
    export(col, name, lod_ratio=1)


BUILDERS = {
    "laptop": lambda M: (laptop(M, True), laptop(M, False)),
    "router": router,
    "usb": usb,
    "battery": battery,
    "solar": solar,
    "server": server,
    "cable": cable,
    "segment": cable_segment,
}


def build(only=None):
    clear()
    M = materials()
    for key, fn in BUILDERS.items():
        if not only or key in only:
            fn(M)
    return [c.name for c in bpy.data.collections]


if __name__ == "__main__":
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    build(argv)
