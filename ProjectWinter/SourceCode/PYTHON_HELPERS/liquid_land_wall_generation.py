import numpy as np
from collections import defaultdict
import os

# ================= CONFIGURATION ================= #
INPUT_LAKE_OBJ  = "../project_winter/assets/worldmap_gaea/terrain_lake.obj"
INPUT_RIVER_OBJ = "../project_winter/assets/worldmap_gaea/terrain_river.obj"
INPUT_LAND_OBJ  = "../project_winter/assets/worldmap_gaea/terrain_land.obj"
OUTPUT_DIR      = "../project_winter/assets/worldmap_gaea/"
OUTPUT_WALL     = os.path.join(OUTPUT_DIR, "terrain_wall.obj")

WALL_DEPTH           = -0.0008 # Vertical extrusion depth
MAP_BOUNDARY_MARGIN  = 1.5     # Distance to ignore edges at the map edge
MATCH_PRECISION      = 6       # Decimal places for vertex matching
GAP_BRIDGE_THRESHOLD = 1.0     # Max distance to bridge gaps
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

    edge_counts   = defaultdict(int)
    directed_data = {}

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
            directed_data[(k1, k2)] = (p1, p2, uvs[vt1_idx] if vt1_idx != -1 else [0,0], 
                                               uvs[vt2_idx] if vt2_idx != -1 else [0,0])

    boundary = {}
    for ((k1, k2), data) in directed_data.items():
        if edge_counts[tuple(sorted((k1, k2)))] < 2:
            # Check map boundary filtering
            if filter_map:
                if (k1[0] <= min_x + MAP_BOUNDARY_MARGIN or k1[0] >= max_x - MAP_BOUNDARY_MARGIN or
                    k1[2] <= min_z + MAP_BOUNDARY_MARGIN or k1[2] >= max_z - MAP_BOUNDARY_MARGIN):
                    continue;
            
            boundary[(k1, k2)] = data

    return boundary;

def bridge_mesh_gaps(active_edges: dict, threshold: float) -> dict:
    (v_out_counts, v_in_counts, v_info) = (defaultdict(int), defaultdict(int), {})
    for ((k1, k2), (p1, p2, uv1, uv2)) in active_edges.items():
        v_out_counts[k1] += 1
        v_in_counts[k2]  += 1

        (v_info[k1], v_info[k2]) = ((p1, uv1), (p2, uv2))

    starts = [v for v in v_in_counts if v_out_counts[v] == 0]
    ends   = [v for v in v_out_counts if v_in_counts[v] == 0]

    new_segments = {}
    for s_v in starts:
        (best_dist, best_e) = (threshold, None)
        for e_v in ends:
            dist = np.linalg.norm(np.array(s_v) - np.array(e_v))
            if dist < best_dist:
                (best_dist, best_e) = (dist, e_v)
        if best_e:
            (p1, uv1) = v_info[s_v]
            (p2, uv2) = v_info[best_e]

            new_segments[(s_v, best_e)] = (p1, p2, uv1, uv2)

    return new_segments;

def main():
    print("Loading meshes...")
    lake_data  = load_obj(INPUT_LAKE_OBJ)
    river_data = load_obj(INPUT_RIVER_OBJ)
    land_data  = load_obj(INPUT_LAND_OBJ)

    if not all([lake_data, river_data, land_data]):
        return;

    # Get boundaries
    print("Finding boundaries...")
    # Get boundaries for both water types and combine their sorted keys
    lake_b  = get_boundary_edges(*lake_data,  filter_map = False)
    river_b = get_boundary_edges(*river_data, filter_map = False)
    
    water_keys_set = {tuple(sorted(k)) for k in lake_b.keys()} | \
                     {tuple(sorted(k)) for k in river_b.keys()}

    # Get land boundaries (this defines wall rotation)
    land_b = get_boundary_edges(*land_data, filter_map=True)
    
    # Filter land edges to only those touching lake OR river
    final_edges = {k: v for k, v in land_b.items() if tuple(sorted(k)) in water_keys_set}
    print(f"Total water-contact edges: {len(final_edges)}")

    # Bridge gaps across the entire system
    bridges = bridge_mesh_gaps(final_edges, GAP_BRIDGE_THRESHOLD)
    final_edges.update(bridges)
    print(f"Bridged {len(bridges)} gaps.")

    # Generate Geometry
    (wall_verts, wall_uvs, wall_faces) = ([], [], [])

    def add_v(pos, uv, down=False):
        p = pos.copy()
        if down: p[1] += WALL_DEPTH
        wall_verts.append(p)
        wall_uvs.append(uv)

        return len(wall_verts);

    for (p1, p2, uv1, uv2) in final_edges.values():
        (v1_t, v2_t) = (add_v(p1, uv1), add_v(p2, uv2))
        (v2_b, v1_b) = (add_v(p2, uv2, down = True), add_v(p1, uv1, down = True))
        wall_faces.append([v1_t, v2_t, v2_b])
        wall_faces.append([v1_t, v2_b, v1_b])

    # Save
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
