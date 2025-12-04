import numpy as np
from PIL import Image
from scipy.interpolate import LinearNDInterpolator
from scipy.ndimage import binary_dilation
import sys

# ================= CONFIGURATION ================= #
INPUT_OBJ  = "../project_winter/assets/worldmap_gaea/super_low_poly_worldmap.obj"
OUTPUT_TXT = "../project_winter/assets/grass_positions.txt"

# Masks
LAKE_MAP  = "../project_winter/assets/worldmap_gaea/lake_texture.bmp"
RIVER_MAP = "../project_winter/assets/worldmap_gaea/rivers_texture.bmp"

# Terrain Logic
TERRAIN_SCALE = 200.0
GRASS_COUNT   = 3000

MIN_HEIGHT = 75.0
MAX_HEIGHT = 90.0

EXPAND_WATER_PIXELS = 20
WATER_THRESHOLD     = 0.1

GRASS_Y_LIFT = -0.1
FLIP_IMAGE_Y = False
# ================================================= #

def load_mesh(path):
    print(f"Loading {path}...")
    vertices = []
    try:
        with open(path, 'r') as f:
            for line in f:
                if line.startswith('v '):
                    parts = line.strip().split()
                    vertices.append([float(parts[1]), float(parts[2]), float(parts[3])])
    except FileNotFoundError:
        print(f"ERROR: Could not find mesh at {path}")
        sys.exit(1)

    return np.array(vertices);

def load_image_data(path, target_size=None):
    try:
        img = Image.open(path).convert('L')
        if target_size and img.size != target_size:
            img = img.resize(target_size)
        return np.array(img) / 255.0
    except FileNotFoundError:
        print(f"ERROR: Could not find texture at {path}")
        sys.exit(1)

    return;

def bake_grass():
    verts = load_mesh(INPUT_OBJ)
    
    (min_x, max_x) = (np.min(verts[:, 0]), np.max(verts[:, 0]))
    (min_z, max_z) = (np.min(verts[:, 2]), np.max(verts[:, 2]))
    width = max_x - min_x
    depth = max_z - min_z
    
    print("Building terrain model...\n")
    interpolator = LinearNDInterpolator(verts[:, [0, 2]], verts[:, 1], fill_value = -9999)

    print("Loading Water Masks...")
    lake_data        = load_image_data(LAKE_MAP)
    (mask_h, mask_w) = lake_data.shape
    river_data       = load_image_data(RIVER_MAP, target_size = (mask_w, mask_h))

    print(f"Expanding water zones by {EXPAND_WATER_PIXELS} pixels...\n")
    
    # 1. Convert grayscale images to strict True/False (Water vs Land)
    lake_bool  = lake_data  > WATER_THRESHOLD
    river_bool = river_data > WATER_THRESHOLD
    
    # 2. Grow the True (Water) areas using binary_dilation
    if EXPAND_WATER_PIXELS > 0:
        lake_bool  = binary_dilation(lake_bool,  iterations = EXPAND_WATER_PIXELS)
        river_bool = binary_dilation(river_bool, iterations = EXPAND_WATER_PIXELS)

    # Debug Image setup
    debug_img = np.zeros((mask_h, mask_w, 3), dtype=np.uint8)

    print(f"Generating {GRASS_COUNT} blades...")
    print(f"  - Flip Y: {FLIP_IMAGE_Y}")
    print(f"  - Y Lift: {GRASS_Y_LIFT}")
    
    valid_points = []
    batch_size   = 50000 
    
    while len(valid_points) < GRASS_COUNT:
        rand_x = np.random.uniform(min_x, max_x, batch_size)
        rand_z = np.random.uniform(min_z, max_z, batch_size)
        
        # Calculate UVs
        u = (rand_x - min_x) / width
        v = (rand_z - min_z) / depth
        
        px_x = (u * (mask_w - 1)).astype(int)
        
        if FLIP_IMAGE_Y:
            px_y = ((1 - v) * (mask_h - 1)).astype(int)
        else:
            # Direct Image style (V = 0 is top)
            px_y = (v * (mask_h - 1)).astype(int)
            
        px_x = np.clip(px_x, 0, mask_w - 1)
        px_y = np.clip(px_y, 0, mask_h - 1)
        
        # Check Masks
        is_lake  = lake_bool[px_y, px_x]
        is_river = river_bool[px_y, px_x]
        
        cand_y_raw = interpolator(rand_x, rand_z)
        
        for i in range(len(rand_x)):
            y_raw = cand_y_raw[i]
            if y_raw < -9000: continue;

            vx = rand_x[i] * TERRAIN_SCALE
            vy = y_raw     * TERRAIN_SCALE
            vz = rand_z[i] * TERRAIN_SCALE

            (cx, cy) = (px_x[i], px_y[i])
            
            # 1. Check Water (Red in debug)
            if is_lake[i] or is_river[i]:
                debug_img[cy, cx] = [255, 0, 0] 
                continue;

            # 2. Check Height (Blue in debug)
            if vy < MIN_HEIGHT or vy > MAX_HEIGHT:
                debug_img[cy, cx] = [0, 0, 255]
                continue;
            
            # 3. Success (Green in debug)
            if len(valid_points) < GRASS_COUNT:
                vy += GRASS_Y_LIFT

                valid_points.append(f"{vx:.4f} {vy:.4f} {vz:.4f}")
                debug_img[cy, cx] = [0, 255, 0]

        print(f"  ... collected {len(valid_points)} / {GRASS_COUNT}")

    print(f"\nSaving to {OUTPUT_TXT}...")
    with open(OUTPUT_TXT, 'w') as f:
        f.write("\n".join(valid_points))
    
    # Save the debug image so you can verify the orientation!
    Image.fromarray(debug_img).save("debug_grass_orientation.png")
    print("Saved 'debug_grass_orientation.png'")

    return;

if __name__ == "__main__":
    bake_grass()
