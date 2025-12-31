from scipy.spatial import ConvexHull
import numpy as np
import os

# ================= CONFIGURATION ================= #
# Paths
TERRAIN_OBJ     = "../project_winter/assets/worldmap_gaea/ultra_low_poly_worldmap.obj"
CABIN_FLOOR_OBJ = "../project_winter/assets/cabin/model/Floor.obj"
CABIN_STAIRS    = "../project_winter/assets/cabin/model/StairsTerrainDetail.obj"
MARINA_OBJ      = "../project_winter/assets/marina/plank_port.obj"
OUTPUT_TXT      = "../project_winter/assets/terrain_triangles_geometry.txt"

# Scale Factor - C++
TERRAIN_SCALE = 200.0
MAX_RADIUS    = 100.0 # Optimization - Distance Limit

# Cabin Transformation Parameters (from C++)
CABIN_POS   = [7.0, 59.2, -0.5]
CABIN_ROT   = 3.0
CABIN_SCALE = 1.0

# Marina Transformation Parameters (from C++)
MARINA_POS   = [-58.2, 58.7, 4.5]
MARINA_ROT   = 1.6
MARINA_SCALE = 0.5
# ================================================= #

def get_model_matrix(pos: np.ndarray, rot_y: float, scale: float) -> np.ndarray:
    # Scale
    S      = np.identity(4)
    S[0,0] = scale; S[1,1] = scale; S[2,2] = scale

    # Rotate Y
    (c, s) = (np.cos(rot_y), np.sin(rot_y))
    R      = np.identity(4)
    R[0,0] = c;  R[0,2] = s
    R[2,0] = -s; R[2,2] = c

    # Translate
    T      = np.identity(4)
    T[0,3] = pos[0]; T[1,3] = pos[1]; T[2,3] = pos[2]

    return T @ R @ S;

def load_obj_triangles(path: str, transform_func: callable = None) -> list:
    if not os.path.exists(path):
        print(f"ERROR: Not found {path}")
        return [];

    vertices  = []
    triangles = []

    with open(path, 'r') as f:
        for line in f:
            parts = line.strip().split()
            if not parts: continue;

            if parts[0] == 'v':
                v = [float(parts[1]), float(parts[2]), float(parts[3])]
                
                # Apply transformation if provided
                if transform_func: v = transform_func(v)
                
                vertices.append(v)

            elif parts[0] == 'f':
                idx1 = int(parts[1].split('/')[0]) - 1
                idx2 = int(parts[2].split('/')[0]) - 1
                idx3 = int(parts[3].split('/')[0]) - 1
                triangles.append((vertices[idx1], vertices[idx2], vertices[idx3]))
    
    return triangles;

def list_optimization(all_triangles: list) -> list:
    # Sort Logic: Distance from (0, 0) in XZ plane
    all_triangles.sort(key = lambda t: (
        ((t[0][0] + t[1][0] + t[2][0]) / 3.0) * ((t[0][0] + t[1][0] + t[2][0]) / 3.0) + 
        ((t[0][2] + t[1][2] + t[2][2]) / 3.0) * ((t[0][2] + t[1][2] + t[2][2]) / 3.0)
    ))

    return all_triangles;

def radius_filter(all_triangles: list, radius: float) -> list:
    filtered = []
    r_sq     = radius * radius
    
    for t in all_triangles:
        # Calculate center of triangle
        cx = (t[0][0] + t[1][0] + t[2][0]) / 3.0
        cy = (t[0][1] + t[1][1] + t[2][1]) / 3.0
        cz = (t[0][2] + t[1][2] + t[2][2]) / 3.0
        
        # Check squared distance (more efficient than sqrt)
        dist_sq = (cx * cx) + (cz * cz) 
        
        if dist_sq <= r_sq:
            filtered.append(t)
            
    return filtered;

def generate_floor_hull(floor_triangles: list) -> list:
    # Extract all vertices from the triangles
    points = []
    for t in floor_triangles:
        points.extend(t) # Add v1, v2, v3
    
    points_np = np.array(points)

    # Compute 2D Convex Hull on XZ plane (ignore Y for shape)
    points_2d = points_np[:, [0, 2]]
    hull      = ConvexHull(points_2d)
    
    # Get the indices of the hull vertices
    hull_indices = hull.vertices
    hull_pts_2d  = points_2d[hull_indices]

    # Determine the Floor Height (Y) - Average...
    floor_y = np.mean(points_np[:, 1])

    # Triangulate the Hull (Triangle Fan method)
    # Connect the first vertex to all other edges to create triangles!
    hull_triangles = []
    
    # Pivot point is the first vertex of the hull
    pivot = [hull_pts_2d[0][0], floor_y, hull_pts_2d[0][1]]

    for i in range(1, len(hull_indices) - 1):
        # Create a triangle from Pivot -> Point i -> Point i+1
        p1 = [hull_pts_2d[i][0],   floor_y, hull_pts_2d[i][1]]
        p2 = [hull_pts_2d[i+1][0], floor_y, hull_pts_2d[i+1][1]]
        
        hull_triangles.append((pivot, p1, p2))

    return hull_triangles;

def main():
    all_triangles = []

    # Load Cabin Floor (Apply Matrix)
    print("Loading Cabin Floor...")
    cabin_matrix = get_model_matrix(CABIN_POS, CABIN_ROT, CABIN_SCALE)
    
    def cabin_transform(v):
        # Convert to homogeneous coordinate (x, y, z, 1)
        vec4 = np.array([v[0], v[1], v[2], 1.0])
        res  = cabin_matrix @ vec4
        return [res[0], res[1], res[2]];

    floor_tris  = load_obj_triangles(CABIN_FLOOR_OBJ, cabin_transform) # Add later to beginning...
    stairs_tris = load_obj_triangles(CABIN_STAIRS, cabin_transform)

    # Load Marina Base (Planks)
    print("Loading Marina Base...")
    marina_matrix = get_model_matrix(MARINA_POS, MARINA_ROT, MARINA_SCALE)

    def marina_transform(v):
        vec4 = np.array([v[0], v[1], v[2], 1.0])
        res  = marina_matrix @ vec4
        return [res[0], res[1], res[2]];

    marina_tris = load_obj_triangles(MARINA_OBJ, marina_transform)

    # Load Terrain (Apply Scale 200!)
    print("Loading Terrain...")
    def terrain_transform(v):
        return [v[0] * TERRAIN_SCALE, v[1] * TERRAIN_SCALE, v[2] * TERRAIN_SCALE];
    
    all_triangles += load_obj_triangles(TERRAIN_OBJ, terrain_transform)
    all_triangles  = list_optimization(all_triangles)
    all_triangles  = radius_filter(all_triangles, MAX_RADIUS)

    # Add floor triangles to the BEGINNING of the list!!!!
    all_triangles = (
        generate_floor_hull(floor_tris) +
        stairs_tris +
        generate_floor_hull(marina_tris) +
        all_triangles
    )

    # Save
    print(f"Exporting {len(all_triangles)} triangles to {OUTPUT_TXT}...")
    with open(OUTPUT_TXT, 'w') as f:
        f.write(f"{len(all_triangles)}\n")
        for t in all_triangles:
            (p1, p2, p3) = t
            f.write(f"{p1[0]:.4f} {p1[1]:.4f} {p1[2]:.4f} "
                    f"{p2[0]:.4f} {p2[1]:.4f} {p2[2]:.4f} "
                    f"{p3[0]:.4f} {p3[1]:.4f} {p3[2]:.4f}\n")

    print("\nDone.")

    return;

if __name__ == "__main__":
    main()
