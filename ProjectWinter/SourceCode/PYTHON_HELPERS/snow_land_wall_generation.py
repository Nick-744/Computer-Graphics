import numpy as np
from collections import defaultdict
import os

# ================= CONFIGURATION ================= #
INPUT_LAND_OBJ = "../project_winter/assets/worldmap_gaea/terrain_land.obj"
OUTPUT_DIR     = "../project_winter/assets/worldmap_gaea/"
OUTPUT_WALL    = os.path.join(OUTPUT_DIR, "terrain_wall.obj")

WALL_DEPTH          = -0.0008 # How deep the wall goes
MAP_BOUNDARY_MARGIN =  1.5    # Skip edges within this distance of the map limits
# ================================================= #

def load_obj(path: str) -> tuple:
    (verts, uvs, norms, faces) = ([], [], [], [])
    if not os.path.exists(path):
        print(f"Error: {path} not found.")

        return None;
    
    with open(path, 'r') as f:
        for line in f:
            p = line.split()
            if not p: continue;
            if   p[0] == 'v':  verts.append(np.array([float(x) for x in p[1:4]]))
            elif p[0] == 'vt':   uvs.append(np.array([float(x) for x in p[1:3]]))
            elif p[0] == 'vn': norms.append(np.array([float(x) for x in p[1:4]]))
            elif p[0] == 'f':
                face = []
                for v_str in p[1:]:
                    parts = v_str.split('/')
                    face.append([int(parts[0])-1, 
                                 int(parts[1])-1 if len(parts)>1 else -1, 
                                 int(parts[2])-1 if len(parts)>2 else -1])
                faces.append(face)

    return (np.array(verts), np.array(uvs), np.array(norms), faces);

def main():
    data = load_obj(INPUT_LAND_OBJ)
    if not data: return;
    (verts, uvs, _, faces) = data

    (min_x, max_x) = (verts[:,0].min(), verts[:,0].max())
    (min_z, max_z) = (verts[:,2].min(), verts[:,2].max())

    # --- STEP 1: Weld Vertices by Position --- #
    # Round to 6 decimal places to handle float precision issues...
    pos_to_unique_id   = {}
    v_idx_to_unique_id = {}
    
    for (i, v) in enumerate(verts):
        key = tuple(np.round(v, 6))
        if key not in pos_to_unique_id:
            pos_to_unique_id[key] = len(pos_to_unique_id)
        v_idx_to_unique_id[i] = pos_to_unique_id[key]

    # --- STEP 2: Find Boundary Edges using Unique IDs --- #
    edge_counts  = defaultdict(int)
    edge_to_data = {} # Stores original vertex data for the wall geometry

    for f in faces:
        for i in range(3):
            (v1_idx, v2_idx) = (f[i][0], f[(i+1)%3][0])
            
            # Map original indices to our "Unique Spatial ID"
            (u1, u2) = (v_idx_to_unique_id[v1_idx], v_idx_to_unique_id[v2_idx])

            # Sorted tuple for counting (directionless)
            edge_key               = tuple(sorted((u1, u2)))
            edge_counts[edge_key] += 1
            
            # Save the raw data for the wall (directed)
            edge_to_data[(u1, u2)] = (f[i], f[(i+1)%3])

    # --- STEP 3: Filter for Internal Shorelines --- #
    boundary_edges = []
    for (edge_key, count) in edge_counts.items():
        if count == 1:
            # Check the coordinates of one of the unique IDs
            # We find the original vertex index that matches this ID
            orig_v_idx = next(idx for idx, uid in v_idx_to_unique_id.items() if uid == edge_key[0])
            v_pos      = verts[orig_v_idx]
            
            is_at_map_edge = (v_pos[0] <= min_x + MAP_BOUNDARY_MARGIN or 
                              v_pos[0] >= max_x - MAP_BOUNDARY_MARGIN or 
                              v_pos[2] <= min_z + MAP_BOUNDARY_MARGIN or 
                              v_pos[2] >= max_z - MAP_BOUNDARY_MARGIN)
            
            if not is_at_map_edge:
                boundary_edges.append(edge_key)

    print(f"Found {len(boundary_edges)} clean internal shoreline edges.")

    # --- STEP 4: Generate Geometry --- #
    (wall_verts, wall_uvs, wall_faces) = ([], [], [])
    UP_NORMAL = "vn 0.000000 1.000000 0.000000\n"

    def add_v(v_idx: int, vt_idx: int, down: bool = False) -> int:
        pos = verts[v_idx].copy()
        if down: pos[1] += WALL_DEPTH

        uv = uvs[vt_idx] if vt_idx != -1 else np.array([0.0, 0.0])

        wall_verts.append(pos)
        wall_uvs.append(uv)

        return len(wall_verts);

    for (u1, u2) in boundary_edges:
        # Use directed data to ensure triangles face the water
        if (u1, u2) in edge_to_data:
            (v1_data, v2_data) = edge_to_data[(u1, u2)]
        else:
            (v1_data, v2_data) = edge_to_data[(u2, u1)]

        idx_t1 = add_v(v1_data[0], v1_data[1])
        idx_t2 = add_v(v2_data[0], v2_data[1])
        idx_b2 = add_v(v2_data[0], v2_data[1], down = True)
        idx_b1 = add_v(v1_data[0], v1_data[1], down = True)

        wall_faces.append([idx_t1, idx_t2, idx_b2])
        wall_faces.append([idx_t1, idx_b2, idx_b1])

    # --- STEP 5: Save ---
    with open(OUTPUT_WALL, 'w') as f:
        for v in wall_verts: f.write(f"v {v[0]:.6f} {v[1]:.6f} {v[2]:.6f}\n")
        for vt in wall_uvs:  f.write(f"vt {vt[0]:.6f} {vt[1]:.6f}\n")
        f.write(UP_NORMAL)
        
        for face in wall_faces:
            f.write(f"f {face[0]}/{face[0]}/1 {face[1]}/{face[1]}/1 {face[2]}/{face[2]}/1\n")

    print(f"Wall generated: {OUTPUT_WALL}")

    return;

if __name__ == "__main__":
    main()
