import os

# ================= CONFIGURATION ================= #
# Path to your terrain OBJ
INPUT_OBJ  = "../project_winter/assets/worldmap_gaea/super_low_poly_worldmap.obj"
# Path to save the text file
OUTPUT_TXT = "../project_winter/assets/terrain_triangles_geometry.txt"

# Scale Factor - C++
TERRAIN_SCALE = 200.0
# ================================================= #

def export_terrain():
    print(f"--- Processing {INPUT_OBJ} ---")
    
    vertices  = []
    triangles = []

    if not os.path.exists(INPUT_OBJ):
        print(f"ERROR: Could not find file at {INPUT_OBJ}")
        return;

    # Parse the OBJ file
    with open(INPUT_OBJ, 'r') as f:
        for line in f:
            parts = line.strip().split()
            if not parts: continue

            # Store Vertices (scaled)
            if parts[0] == 'v':
                x = float(parts[1]) * TERRAIN_SCALE
                y = float(parts[2]) * TERRAIN_SCALE
                z = float(parts[3]) * TERRAIN_SCALE
                vertices.append((x, y, z))

            # Store Faces (Triangles)
            elif parts[0] == 'f':
                # We only need the vertex index part...
                idx1 = int(parts[1].split('/')[0]) - 1
                idx2 = int(parts[2].split('/')[0]) - 1
                idx3 = int(parts[3].split('/')[0]) - 1
                
                # Store the actual 3 coordinates for this triangle
                triangles.append((vertices[idx1], vertices[idx2], vertices[idx3]))

    # Save to Text File
    print(f"Exporting {len(triangles)} triangles to {OUTPUT_TXT}...")
    
    with open(OUTPUT_TXT, 'w') as f:
        # Header: Number of triangles - So C++ pre-allocates memory (optimization)!
        f.write(f"{len(triangles)}\n")
        
        # Body: x1 y1 z1 x2 y2 z2 x3 y3 z3
        for tri in triangles:
            (p1, p2, p3) = tri
            line = (f"{p1[0]:.4f} {p1[1]:.4f} {p1[2]:.4f} "
                    f"{p2[0]:.4f} {p2[1]:.4f} {p2[2]:.4f} "
                    f"{p3[0]:.4f} {p3[1]:.4f} {p3[2]:.4f}")
            f.write(line + "\n")

    print("-> Done")

if __name__ == "__main__":
    export_terrain()
