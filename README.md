# Project Winter ❄️ | Graphics and Virtual Reality (ECE AK709)

**Academic Year 2025/2026**

---

## Project Overview
This project involves the creation of a dynamic 3D winter environment implemented in **C++** using **OpenGL** and **GLSL**. The goal was to develop a realistic landscape that combines static world-building with dynamic evolution through snowfall, lake freezing, and interactive physics.

### Watch the Gameplay
[![Project Winter Showcase](https://img.shields.io/badge/YouTube-Video-red?style=for-the-badge&logo=youtube)](https://youtu.be/MUBdYW6Ok9U?si=vT2lgmGccFznEbaI)

---

## 🌟 Key Features

### 🏔️ Procedural World Generation
* **Terrain:** Realistic mountainous geometry with a central valley created using **Gaea** (procedural erosion and river simulation).
* **Volumetric Clouds:** Developed using instanced billboard particles with dual-noise texture blending.
* **Vegetation:** Procedural placement of grass and trees using Python scripts, featuring dynamic wind-sway animations.

### 🌨️ Dynamic Weather & Environment
* **Snow Accumulation:** Real-time snowfall using 4,000 particles. Snow builds up on surfaces exposed to the sky (calculated via Shadow Mapping) and utilizes **Taubin Smoothing** for volume inflation.
* **Frozen Lake:** The water transitions to ice by halting displacement mapping and generating procedural cracks via **Voronoi noise**.
* **Planar Reflections:** Realistic environment reflections on the ice/water surface using a dedicated FBO and mirrored view matrices.

### 🎮 Gameplay & Interaction
* **Interactive Boat:** Inertia-based rowing physics with procedural "wobble" and audio-synced oar animations.
* **Snowman System:** Players can roll snowballs that dynamically increase in radius and mass. A Frustum Visibility check replaces the balls with a final model once the player looks away.
* **Arcade Integration:** A fully functional **Java-based arcade game** ("The Forbidden Spaceship") rendered inside the 3D cabin by capturing Java window pixels in real-time via the Windows API.

---

## 🛠️ Technical Implementation

### 🚀 Optimizations
* **View Frustum Culling:** Environment divided into chunks; only visible chunks are rendered based on AABB checks.
* **Shadow Mapping:** Uses **Frustum Fitting** to maximize shadow map resolution within the camera's view.
* **Collision System:** Efficient Sphere-to-Triangle collision detection for the player and world objects.

### 🕹️ Advanced Techniques
* **Ray Marching:** Used for world interaction (opening doors, accessing the arcade).
* **Dynamic Audio:** Footstep sounds change automatically based on terrain type (wood, snow, ice, dirt).
* **AI Narration:** Voiceovers generated via **ElevenLabs**.
* **UI:** Pause menu and settings management via **ImGui**.

---

## 📂 Repository Structure

```text
.
├── Labs/          # Complete answers to all semester lab exercises (Lab1 -> Lab7)
└── ProjectWinter/ # Main project source code and shaders
