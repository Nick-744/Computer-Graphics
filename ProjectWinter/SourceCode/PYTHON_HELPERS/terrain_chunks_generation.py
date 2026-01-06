import numpy as np
import os

# ================= CONFIGURATION ================= #
ASSET_BASE  = "../project_winter/assets/worldmap_gaea/"
INPUT_FILES = {
    "land": os.path.join(ASSET_BASE, "terrain_land.obj"),
    "lake": os.path.join(ASSET_BASE, "terrain_lake.obj")
}
OUTPUT_DIR = os.path.join(ASSET_BASE, "chunks/")
INFO_FILE  = os.path.join(OUTPUT_DIR, "chunks_info.txt")

GRID_SIZE   = 8     # 8x8 grid
WORLD_SCALE = 200.0 # Match C++ getTerrainModelMatrix()
# ================================================= #

def load_obj(path: str) -> tuple:
    if not os.path.exists(path): return None;

    (verts, uvs, norms, faces) = ([], [], [], [])
    with open(path, 'r') as f:
        for line in f:
            p = line.split()
            if not p: continue;
            if   p[0] == 'v':  verts.append([float(x) for x in p[1:4]])
            elif p[0] == 'vt':   uvs.append([float(x) for x in p[1:3]])
            elif p[0] == 'vn': norms.append([float(x) for x in p[1:4]])
            elif p[0] == 'f':
                faces.append([[int(i.split('/')[0])-1, 
                               int(i.split('/')[1])-1 if '/' in i else -1, 
                               int(i.split('/')[2])-1 if i.count('/')==2 else -1] for i in p[1:]])
    
    return (np.array(verts), np.array(uvs), np.array(norms), faces);

def save_chunk_obj(path: str, tri_faces: list, verts: np.ndarray, uvs: np.ndarray, norms: np.ndarray) -> None:
    with open(path, 'w') as f_out:
        (v_map, vt_map, vn_map) = ({}, {}, {})
        (new_v, new_vt, new_vn) = ([], [], [])
        for face in tri_faces:
            for (v_idx, vt_idx, vn_idx) in face:
                if v_idx not in v_map:
                    v_map[v_idx] = len(new_v) + 1
                    new_v.append(verts[v_idx])
                if vt_idx != -1 and vt_idx not in vt_map:
                    vt_map[vt_idx] = len(new_vt) + 1
                    new_vt.append(uvs[vt_idx])
                if vn_idx != -1 and vn_idx not in vn_map:
                    vn_map[vn_idx] = len(new_vn) + 1
                    new_vn.append(norms[vn_idx])

        for v in new_v:   f_out.write(f"v {v[0]:.6f} {v[1]:.6f} {v[2]:.6f}\n")
        for vt in new_vt: f_out.write(f"vt {vt[0]:.6f} {vt[1]:.6f}\n")
        for vn in new_vn: f_out.write(f"vn {vn[0]:.6f} {vn[1]:.6f} {vn[2]:.6f}\n")
        for face in tri_faces:
            f_out.write("f " + " ".join([f"{v_map[v[0]]}/{vt_map[v[1]] if v[1]!=-1 else ''}/{vn_map[v[2]] if v[2]!=-1 else ''}" for v in face]) + "\n")

    return;

def main():
    os.makedirs(OUTPUT_DIR, exist_ok = True)
    all_info_lines = []

    for (prefix, path) in INPUT_FILES.items():
        data = load_obj(path)
        if not data: continue;
        (verts, uvs, norms, faces) = data
        
        (min_x, max_x) = (verts[:,0].min(), verts[:,0].max())
        (min_z, max_z) = (verts[:,2].min(), verts[:,2].max())
        (dx, dz)       = ((max_x - min_x) / GRID_SIZE, (max_z - min_z) / GRID_SIZE)

        for row in range(GRID_SIZE):
            for col in range(GRID_SIZE):
                (b_min_x, b_max_x) = (min_x + col*dx, min_x + (col+1)*dx)
                (b_min_z, b_max_z) = (min_z + row*dz, min_z + (row+1)*dz)

                chunk_faces = []
                chunk_verts = []
                for f in faces:
                    centroid = np.mean([verts[f[0][0]], verts[f[1][0]], verts[f[2][0]]], axis = 0)
                    if b_min_x <= centroid[0] < b_max_x and b_min_z <= centroid[2] < b_max_z:
                        chunk_faces.append(f)
                        chunk_verts.extend([verts[f[0][0]], verts[f[1][0]], verts[f[2][0]]])

                if chunk_faces:
                    filename = f"{prefix}_{row}_{col}.obj"
                    save_chunk_obj(os.path.join(OUTPUT_DIR, filename), chunk_faces, verts, uvs, norms)
                    cv             = np.array(chunk_verts)
                    (c_min, c_max) = (cv.min(axis = 0) * WORLD_SCALE, cv.max(axis = 0) * WORLD_SCALE)
                    all_info_lines.append(f"{filename} {c_min[0]} {c_min[1]} {c_min[2]} {c_max[0]} {c_max[1]} {c_max[2]}")

    with open(INFO_FILE, 'w') as f:
        f.write("\n".join(all_info_lines))
    print(f"Done. Info saved to {INFO_FILE}")

    return;

if __name__ == "__main__":
    main()
