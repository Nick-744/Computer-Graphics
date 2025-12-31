import numpy as np
from collections import defaultdict
import os

# ================= CONFIGURATION ================= #
INPUT_LAKE_OBJ = "../project_winter/assets/worldmap_gaea/terrain_lake.obj"
INPUT_LAND_OBJ = "../project_winter/assets/worldmap_gaea/terrain_land.obj"
OUTPUT_DIR     = "../project_winter/assets/worldmap_gaea/"
OUTPUT_WALL    = os.path.join(OUTPUT_DIR, "terrain_wall.obj")

WALL_DEPTH          = -0.08 # Vertical extrusion depth
MAP_BOUNDARY_MARGIN = 1.5   # Distance to ignore edges at the map edge
MATCH_PRECISION     = 6     # Decimal places for vertex matching (the "small error")
# ================================================= #

def load_obj(path: str) -> tuple:
    (verts, uvs, faces) = ([], [], [])
    if not os.path.exists(path):
        print(f"Error: {path} not found.")
        return None;
    
    with open(path, 'r') as f:
        for line in f:
            p = line.split()
            if not p: continue;
            if   p[0] == 'v':  verts.append(np.array([float(x) for x in p[1:4]]))
            elif p[0] == 'vt':   uvs.append(np.array([float(x) for x in p[1:3]]))
            elif p[0] == 'f':
                face = []
                for v_str in p[1:]:
                    parts = v_str.split('/')
                    # Store (v_idx, vt_idx)
                    face.append([int(parts[0])-1, int(parts[1])-1 if len(parts) > 1 else -1])
                faces.append(face)

    return (np.array(verts), np.array(uvs), faces);

def get_boundary_edges(verts: np.ndarray, uvs: np.ndarray, faces: list, filter_map: bool = True) -> dict:
    """Returns a dict of {(v1_pos, v2_pos): (v1_data, v2_data)} for boundary edges."""

    edge_counts = defaultdict(int)
    edge_data   = {}

    # Map bounds for filtering
    (min_x, max_x) = (verts[:, 0].min(), verts[:, 0].max())
    (min_z, max_z) = (verts[:, 2].min(), verts[:, 2].max())

    for f in faces:
        for i in range(len(f)):
            (v1_idx, vt1_idx) = f[i]
            (v2_idx, vt2_idx) = f[(i+1) % len(f)]
            
            (p1, p2) = (verts[v1_idx], verts[v2_idx])
            # Key uses rounded coordinates for robust matching
            (k1, k2) = (tuple(np.round(p1, MATCH_PRECISION)), tuple(np.round(p2, MATCH_PRECISION)))
            
            # Sort keys to make edge direction-independent for counting
            edge_key               = tuple(sorted((k1, k2)))
            edge_counts[edge_key] += 1
            # Keep original direction for face orientation
            edge_data[(k1, k2)] = (p1, p2, uvs[vt1_idx] if vt1_idx != -1 else [0,0], 
                                           uvs[vt2_idx] if vt2_idx != -1 else [0,0])

    boundary = {}
    for edge_key, count in edge_counts.items():
        if count < 2:
            # Check map boundary filtering
            if filter_map:
                p_check = edge_key[0]
                if (p_check[0] <= min_x + MAP_BOUNDARY_MARGIN or p_check[0] >= max_x - MAP_BOUNDARY_MARGIN or
                    p_check[2] <= min_z + MAP_BOUNDARY_MARGIN or p_check[2] >= max_z - MAP_BOUNDARY_MARGIN):
                    continue;
            
            # Retrieve the directed version (either k1,k2 or k2,k1 exists in edge_data)
            if edge_key in edge_data:
                boundary[edge_key] = edge_data[edge_key]
            else:
                boundary[edge_key] = edge_data[(edge_key[1], edge_key[0])]
                
    return boundary;

def main():
    print("Loading meshes...")
    lake_data = load_obj(INPUT_LAKE_OBJ)
    land_data = load_obj(INPUT_LAND_OBJ)
    
    if not lake_data or not land_data:
        return;

    # Step 1 & 2: Get boundaries
    print("Finding boundaries...")
    lake_edges = get_boundary_edges(*lake_data, filter_map=False)
    land_edges = get_boundary_edges(*land_data, filter_map=True)

    # Step 3: Keep the intersection
    common_keys = set(lake_edges.keys()) & set(land_edges.keys())
    print(f"Lake edges: {len(lake_edges)} | Land edges: {len(land_edges)}")
    print(f"Intersection: {len(common_keys)} common edges found.")

    # Step 4: Generate Geometry
    (wall_verts, wall_uvs, wall_faces) = ([], [], [])
    
    def add_v(pos, uv, down=False):
        p = pos.copy()
        if down: p[1] += WALL_DEPTH
        wall_verts.append(p)
        wall_uvs.append(uv)

        return len(wall_verts);

    for key in common_keys:
        # Use data from land or lake (they are identical at this point)
        (p1, p2, uv1, uv2) = land_edges[key]

        idx_t1 = add_v(p1, uv1)
        idx_t2 = add_v(p2, uv2)
        idx_b2 = add_v(p2, uv2, down=True)
        idx_b1 = add_v(p1, uv1, down=True)

        wall_faces.append([idx_t1, idx_t2, idx_b2])
        wall_faces.append([idx_t1, idx_b2, idx_b1])

    # Step 5: Save
    with open(OUTPUT_WALL, 'w') as f:
        for v in wall_verts: f.write(f"v {v[0]:.6f} {v[1]:.6f} {v[2]:.6f}\n")
        for vt in wall_uvs:  f.write(f"vt {vt[0]:.6f} {vt[1]:.6f}\n")
        f.write("vn 0.0 1.0 0.0\n")
        for face in wall_faces:
            f.write(f"f {face[0]}/{face[0]}/1 {face[1]}/{face[1]}/1 {face[2]}/{face[2]}/1\n")

    print(f"Successfully saved intersection wall to: {OUTPUT_WALL}")

    return;

if __name__ == "__main__":
    main()
