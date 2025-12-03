import numpy as np
from PIL import Image
from scipy.interpolate import griddata
import random

# ================= CONFIGURATION ================= #
INPUT_OBJ  = "assets/worldmap_gaea/super_low_poly_worldmap.obj"
SOIL_MAP   = "assets/worldmap_gaea/soil_texture.bmp"
OUTPUT_TXT = "assets/grass_positions.txt"

# Terrain Logic
TERRAIN_SCALE = 200.0  # The scale used in C++
GRASS_COUNT   = 15000  # How many blades you want
# ================================================= #

def load_mesh(path):
    print(f"Loading {path}...")
    vertices = []
    with open(path, 'r') as f:
        for line in f:
            if line.startswith('v '):
                parts = line.strip().split()
                # Store raw (x, y, z)
                vertices.append([float(parts[1]), float(parts[2]), float(parts[3])])
    return np.array(vertices)

def bake_grass():
    # 1. Load Mesh Data
    verts = load_mesh(INPUT_OBJ)
    
    # Get bounds
    min_x, max_x = np.min(verts[:, 0]), np.max(verts[:, 0])
    min_z, max_z = np.min(verts[:, 2]), np.max(verts[:, 2])
    min_y = np.min(verts[:, 1])
    
    width = max_x - min_x
    depth = max_z - min_z
    
    print(f"Mesh Bounds X: [{min_x}, {max_x}]")
    print(f"Mesh Bounds Z: [{min_z}, {max_z}]")

    # 2. Prepare Interpolator (Linear interpolation over the mesh)
    # We use X and Z to predict Y
    print("Building terrain interpolator...")
    # This function allows us to ask "What is Y at this X,Z?"
    # We don't pre-rasterize; we interpret raw vertex data.
    
    # 3. Load Soil Map for Masking
    print(f"Loading Soil Map: {SOIL_MAP}")
    soil_img = Image.open(SOIL_MAP).convert('L') # Grayscale
    soil_w, soil_h = soil_img.size
    soil_data = np.array(soil_img) / 255.0 # Normalize 0..1

    print(f"Generating {GRASS_COUNT} grass positions...")
    
    valid_points = []
    
    # Generate in batches to save time
    batch_size = GRASS_COUNT * 2 
    
    while len(valid_points) < GRASS_COUNT:
        # A. Random Raw X/Z
        rand_x = np.random.uniform(min_x, max_x, batch_size)
        rand_z = np.random.uniform(min_z, max_z, batch_size)
        
        # B. Check Soil Mask
        # Map raw X/Z to UV 0..1
        u = (rand_x - min_x) / width
        v = (rand_z - min_z) / depth
        
        # Map UV to Pixel Coords
        # NOTE: Images are Top-Down (Y=0 is top). 3D Z is usually Bottom-Up.
        # We flip V here to match standard texturing.
        px_x = (u * (soil_w - 1)).astype(int)
        px_y = ((1 - v) * (soil_h - 1)).astype(int)
        
        # Sample soil (Vectorized lookup)
        soil_vals = soil_data[px_y, px_x]
        
        # Keep only indices where soil > 0.4 (Grass)
        mask = soil_vals > 0.4
        
        valid_x = rand_x[mask]
        valid_z = rand_z[mask]
        
        if len(valid_x) == 0: continue
        
        # C. Interpolate Height for valid points
        # usage: griddata(points, values, xi, method)
        valid_y = griddata(
            verts[:, [0, 2]], # Inputs (X, Z)
            verts[:, 1],      # Output (Y)
            (valid_x, valid_z), 
            method='linear',
            fill_value=min_y
        )
        
        # D. Apply Scale and Store
        # We multiply by TERRAIN_SCALE now so C++ doesn't have to do math.
        for i in range(len(valid_x)):
            vx = valid_x[i] * TERRAIN_SCALE
            vy = valid_y[i] * TERRAIN_SCALE
            vz = valid_z[i] * TERRAIN_SCALE
            
            # Sink it slightly so roots are hidden
            vy -= 0.2 
            
            valid_points.append(f"{vx:.4f} {vy:.4f} {vz:.4f}")
            
            if len(valid_points) >= GRASS_COUNT:
                break
                
        print(f"  ... collected {len(valid_points)} / {GRASS_COUNT}")

    # 4. Save to File
    print(f"Saving to {OUTPUT_TXT}...")
    with open(OUTPUT_TXT, 'w') as f:
        f.write("\n".join(valid_points))
    print("Done!")

if __name__ == "__main__":
    bake_grass()
