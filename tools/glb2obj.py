# Convert a glTF binary (.glb) to a triangulated OBJ + JPG textures for the simple OBJ loader.
import sys, trimesh, numpy as np
src, dst_obj, tex_prefix, scale = sys.argv[1], sys.argv[2], sys.argv[3], float(sys.argv[4])
s = trimesh.load(src, force='scene')
lines = ["# Converted from %s (glTF -> OBJ)" % src.split('/')[-1]]
off = 0
saved = False
for node in s.graph.nodes_geometry:
    T, gname = s.graph[node]
    g = s.geometry[gname].copy()
    g.apply_transform(T)
    g.apply_scale(scale)
    m = g.visual.material
    if not saved:
        m.baseColorTexture.convert('RGB').resize((1024,1024)).save(tex_prefix + "_diffuse.jpg", quality=90)
        if getattr(m, 'emissiveTexture', None) is not None:
            m.emissiveTexture.convert('RGB').resize((1024,1024)).save(tex_prefix + "_emissive.jpg", quality=90)
            print("emissive saved")
        saved = True
    uv = g.visual.uv
    n = g.vertex_normals
    lines.append("o " + gname)
    for p in g.vertices: lines.append("v %.5f %.5f %.5f" % tuple(p))
    for t in uv: lines.append("vt %.5f %.5f" % (t[0], t[1]))
    for q in n: lines.append("vn %.4f %.4f %.4f" % tuple(q))
    for f in g.faces + 1 + off:
        lines.append("f " + " ".join("%d/%d/%d" % (i, i, i) for i in f))
    off += len(g.vertices)
open(dst_obj, "w").write("\n".join(lines) + "\n")
print(dst_obj, "verts", off)
