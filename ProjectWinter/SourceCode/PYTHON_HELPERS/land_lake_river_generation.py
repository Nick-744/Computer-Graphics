import numpy as np
from PIL import Image
import os

# ================= CONFIGURATION ================= #
INPUT_OBJ      = "../project_winter/assets/worldmap_gaea/ultra_low_poly_worldmap.obj"
LAKE_MAP_PATH  = "../project_winter/assets/worldmap_gaea/lake_texture.bmp"
RIVER_MAP_PATH = "../project_winter/assets/worldmap_gaea/rivers_texture.bmp"

OUTPUT_DIR   = "../project_winter/assets/worldmap_gaea/"
OUTPUT_LAKE  = os.path.join(OUTPUT_DIR, "terrain_lake.obj")
OUTPUT_RIVER = os.path.join(OUTPUT_DIR, "terrain_river.obj")
OUTPUT_LAND  = os.path.join(OUTPUT_DIR, "terrain_land.obj")

MASK_THRESHOLD        = 0.5
MAX_SUBDIVISION_DEPTH = 4
# ================================================= #

def load_obj_indexed(path: str) -> tuple:
    """Loads OBJ including v, vt, and vn."""
    (verts, uvs, norms) = ([], [], [])
    faces = []
    with open(path, 'r') as f:
        for line in f:
            p = line.split()
            if not p: continue;
            if   p[0] == 'v':  verts.append(np.array([float(x) for x in p[1:4]]))
            elif p[0] == 'vt':   uvs.append(np.array([float(x) for x in p[1:3]]))
            elif p[0] == 'vn': norms.append(np.array([float(x) for x in p[1:4]]))
            elif p[0] == 'f':
                # Parse f v/vt/vn
                face_data = []
                for vertex in p[1:]:
                    parts = vertex.split('/')
                    # v_idx, vt_idx, vn_idx
                    v_i = int(parts[0]) - 1
                    t_i = int(parts[1]) - 1 if len(parts) > 1 and parts[1] else -1
                    n_i = int(parts[2]) - 1 if len(parts) > 2 and parts[2] else -1
                    face_data.append((v_i, t_i, n_i))
                faces.append(face_data)

    return (np.array(verts), np.array(uvs), np.array(norms), faces);

def save_triangles_to_obj(path: str, triangles: list) -> None:
    """Saves OBJ in strict format: v... vt... vn... f..."""
    if not triangles: return;
    
    (unique_v, unique_vt, unique_vn) = ([], [], [])
    # Map (v_tuple, vt_tuple, vn_tuple) -> index
    vertex_map   = {}
    face_indices = []

    for tri in triangles:
        current_face = []
        for v_data in tri:
            # v_data is (pos, uv, norm)
            v_t  = tuple(np.round(v_data[0], 6))
            vt_t = tuple(np.round(v_data[1], 6))
            vn_t = tuple(np.round(v_data[2], 6))
            key  = (v_t, vt_t, vn_t)

            if key not in vertex_map:
                vertex_map[key] = len(unique_v) + 1
                unique_v.append(v_t)
                unique_vt.append(vt_t)
                unique_vn.append(vn_t)
            current_face.append(vertex_map[key])
        face_indices.append(current_face)

    with open(path, 'w') as f:
        for v in unique_v: f.write(f"v {v[0]:.6f} {v[1]:.6f} {v[2]:.6f}\n")
        for vt in unique_vt: f.write(f"vt {vt[0]:.6f} {vt[1]:.6f}\n")
        for vn in unique_vn: f.write(f"vn {vn[0]:.6f} {vn[1]:.6f} {vn[2]:.6f}\n")
        for fi in face_indices: f.write(f"f {fi[0]}/{fi[0]}/{fi[0]} {fi[1]}/{fi[1]}/{fi[1]} {fi[2]}/{fi[2]}/{fi[2]}\n")

    return;

def load_image_mask(path: str) -> np.ndarray:
    return np.array(Image.open(path).convert('L')) / 255.0;

def sample_mask_at_pos(pos: np.ndarray, mask: np.ndarray, bounds: tuple) -> float:
    (min_x, max_x, min_z, max_z) = bounds

    u = np.clip((pos[0] - min_x) / (max_x - min_x or 1), 0, 1)
    v = np.clip((pos[2] - min_z) / (max_z - min_z or 1), 0, 1)

    return mask[int(v * (mask.shape[0]-1)), int(u * (mask.shape[1]-1))];

def lerp_v_data(d1: tuple, d2: tuple, t: float) -> tuple:
    # Interpolate Position and UV
    pos = d1[0] + t * (d2[0] - d1[0])
    uv  = d1[1] + t * (d2[1] - d1[1])
    
    # Interpolate Normal
    norm_raw = d1[2] + t * (d2[2] - d1[2])
    
    # RE-NORMALIZATION: Ensures the normal is exactly 1.0 units long!
    mag  = np.linalg.norm(norm_raw)
    norm = norm_raw / mag if mag != 0 else norm_raw
    
    return (pos, uv, norm);

def intersect_edge_data(d1: tuple, d2: tuple, v1: float, v2: float, threshold: float) -> tuple:
    t = (threshold - v1) / (v2 - v1)

    return lerp_v_data(d1, d2, t);

def split_triangle_linear(d0: tuple, d1: tuple, d2: tuple, v0: float, v1: float, v2: float, threshold: float) -> tuple:
    (tris_below, tris_above) = ([], [])

    s     = [v0 >= threshold, v1 >= threshold, v2 >= threshold]
    count = sum(s)
    if   count == 0: tris_below.append((d0, d1, d2))
    elif count == 3: tris_above.append((d0, d1, d2))
    elif count == 1:
        if   s[0]: (A, B, C, vA, vB, vC) = (d0, d1, d2, v0, v1, v2)
        elif s[1]: (A, B, C, vA, vB, vC) = (d1, d2, d0, v1, v2, v0)
        else:      (A, B, C, vA, vB, vC) = (d2, d0, d1, v2, v0, v1)
        iAB = intersect_edge_data(A, B, vA, vB, threshold)
        iAC = intersect_edge_data(A, C, vA, vC, threshold)
        tris_above.append((A, iAB, iAC))
        tris_below.extend([(iAB, B, C), (iAB, C, iAC)])
    else:
        if   not s[0]: (A, B, C, vA, vB, vC) = (d0, d1, d2, v0, v1, v2)
        elif not s[1]: (A, B, C, vA, vB, vC) = (d1, d2, d0, v1, v2, v0)
        else:          (A, B, C, vA, vB, vC) = (d2, d0, d1, v2, v0, v1)
        iAB = intersect_edge_data(A, B, vA, vB, threshold)
        iAC = intersect_edge_data(A, C, vA, vC, threshold)
        tris_below.append((A, iAB, iAC))
        tris_above.extend([(iAB, B, C), (iAB, C, iAC)])

    return (tris_below, tris_above);

def adaptive_split(d0: tuple, d1: tuple, d2: tuple, mask: np.ndarray, bounds: tuple, threshold: float, depth: int) -> tuple:
    val          = [sample_mask_at_pos(d[0], mask, bounds) for d in [d0, d1, d2]]
    needs_subdiv = (any(v >= threshold for v in val) != all(v >= threshold for v in val))
    
    if (not needs_subdiv) and (depth < MAX_SUBDIVISION_DEPTH):
        # Check midpoints for the "wiggle" error
        for pair in [(d0,d1), (d1,d2), (d2,d0)]:
            m_pos = (pair[0][0] + pair[1][0]) / 2
            if (sample_mask_at_pos(m_pos, mask, bounds) >= threshold) != (val[0] >= threshold):
                needs_subdiv = True
                break;

    if needs_subdiv and (depth < MAX_SUBDIVISION_DEPTH):
        (m01, m12, m20) = (lerp_v_data(d0,d1,0.5), lerp_v_data(d1,d2,0.5), lerp_v_data(d2,d0,0.5))
        (res_b, res_a) = ([], [])
        for st in [(d0,m01,m20), (m01,d1,m12), (m20,m12,d2), (m01,m12,m20)]:
            (b, a) = adaptive_split(st[0], st[1], st[2], mask, bounds, threshold, depth+1)
            res_b.extend(b)
            res_a.extend(a)

        return res_b, res_a;

    return split_triangle_linear(d0, d1, d2, val[0], val[1], val[2], threshold);

def main():
    (verts, uvs, norms, faces) = load_obj_indexed(INPUT_OBJ)

    l_mask = load_image_mask(LAKE_MAP_PATH)
    r_mask = load_image_mask(RIVER_MAP_PATH)
    bounds = (verts[:,0].min(), verts[:,0].max(), verts[:,2].min(), verts[:,2].max())

    # Create Initial Triangles with Data: (pos, uv, norm)
    current_tris = []
    for f in faces:
        v_data = []
        for i in range(3):
            p = verts[f[i][0]]
            t =   uvs[f[i][1]] if f[i][1] != -1 else np.array([0.0, 0.0])
            n = norms[f[i][2]] if f[i][2] != -1 else np.array([0.0, 1.0, 0.0])
            v_data.append((p, t, n))
        current_tris.append(tuple(v_data))

    print("Splitting Lakes...")
    (lake_final, land_temp) = ([], [])
    for tri in current_tris:
        (b, a) = adaptive_split(tri[0], tri[1], tri[2], l_mask, bounds, MASK_THRESHOLD, 0)
        lake_final.extend(a)
        land_temp.extend(b)

    print("Splitting Rivers...")
    (river_final, land_final) = ([], [])
    for tri in land_temp:
        (b, a) = adaptive_split(tri[0], tri[1], tri[2], r_mask, bounds, MASK_THRESHOLD, 0)
        river_final.extend(a)
        land_final.extend(b)

    os.makedirs(OUTPUT_DIR, exist_ok = True)
    save_triangles_to_obj(OUTPUT_LAKE,  lake_final)
    save_triangles_to_obj(OUTPUT_RIVER, river_final)
    save_triangles_to_obj(OUTPUT_LAND,  land_final)
    print("Done.")

    return;

if __name__ == "__main__":
    main()
