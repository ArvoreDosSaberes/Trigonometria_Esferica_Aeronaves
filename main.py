# We'll demonstrate the geometry in Fig. 2.16:
# - define a target direction (AZ_T, EL_T) and a roll-axis direction (AZ_R, EL_R)
# - convert azimuth/elevation to 3D unit vectors
# - compute the great-circle angle J between them (angle from roll axis to target)
# - plot a unit sphere and both directions as arrows
#
# Notes:
# * Azimuth measured from "N" (x-axis here) toward "E" (y-axis), in radians
# * Elevation measured from horizon (xy-plane) toward +Z (local vertical)
# * You can change the angles below to test other situations

import numpy as np
import matplotlib.pyplot as plt

# ---------- helpers ----------
def azel_to_vec(az, el):
    """
    Convert azimuth (rad) and elevation (rad) to a 3D unit vector.
    Azimuth from +X toward +Y; Elevation from XY-plane toward +Z.
    """
    ce = np.cos(el)
    x = ce * np.cos(az)
    y = ce * np.sin(az)
    z = np.sin(el)
    v = np.array([x, y, z], dtype=float)
    return v / np.linalg.norm(v)

def angle_between(v1, v2):
    """Great-circle / central angle between two unit vectors (rad)."""
    # numerical safety
    c = np.clip(np.dot(v1, v2), -1.0, 1.0)
    return np.arccos(c)

# ---------- example angles (feel free to edit) ----------
AZ_T_deg, EL_T_deg = 40.0, 25.0     # target azimuth/elevation (degrees)
AZ_R_deg, EL_R_deg = 10.0, 5.0      # roll-axis azimuth/elevation (degrees)

AZ_T, EL_T = np.radians(AZ_T_deg), np.radians(EL_T_deg)
AZ_R, EL_R = np.radians(AZ_R_deg), np.radians(EL_R_deg)

# vectors
vT = azel_to_vec(AZ_T, EL_T)
vR = azel_to_vec(AZ_R, EL_R)

# angle J using dot product (equivalent to spherical law of cosines)
J = angle_between(vT, vR)
J_deg = np.degrees(J)

# Also show the analytic spherical-trig form for verification:
# cos J = sin(EL_T) sin(EL_R) + cos(EL_T) cos(EL_R) cos(AZ_T - AZ_R)
cosJ_trig = np.sin(EL_T)*np.sin(EL_R) + np.cos(EL_T)*np.cos(EL_R)*np.cos(AZ_T - AZ_R)
J_trig = np.degrees(np.arccos(np.clip(cosJ_trig, -1.0, 1.0)))

print(f"AZ_T={AZ_T_deg:.1f}°, EL_T={EL_T_deg:.1f}°   |   AZ_R={AZ_R_deg:.1f}°, EL_R={EL_R_deg:.1f}°")
print(f"Ângulo J (alvo a partir do eixo de rolagem) ≈ {J_deg:.3f}°  (verificação: {J_trig:.3f}°)")

# ---------- plot ----------
fig = plt.figure(figsize=(7, 6))
ax = fig.add_subplot(111, projection='3d')
# unit sphere wireframe
u, v = np.linspace(0, 2*np.pi, 60), np.linspace(0, np.pi, 30)
xs = np.outer(np.cos(u), np.sin(v))
ys = np.outer(np.sin(u), np.sin(v))
zs = np.outer(np.ones_like(u), np.cos(v))
ax.plot_wireframe(xs, ys, zs, linewidth=0.3, alpha=0.4)

# draw axes (N-E-Up)
L = 1.2
ax.plot([0, L], [0, 0], [0, 0])  # x: North
ax.plot([0, 0], [0, L], [0, 0])  # y: East
ax.plot([0, 0], [0, 0], [0, L])  # z: Up

# arrows for roll axis (R) and target (T)
ax.quiver(0, 0, 0, vR[0], vR[1], vR[2], length=1.0, arrow_length_ratio=0.08)
ax.quiver(0, 0, 0, vT[0], vT[1], vT[2], length=1.0, arrow_length_ratio=0.08)

# labels
ax.text(vR[0]*1.05, vR[1]*1.05, vR[2]*1.05, "R", fontsize=10)
ax.text(vT[0]*1.05, vT[1]*1.05, vT[2]*1.05, "T", fontsize=10)
ax.text(1.05, 0, 0, "N (AZ=0°)", fontsize=9)
ax.text(0, 1.05, 0, "E (AZ=90°)", fontsize=9)
ax.text(0, 0, 1.05, "Up", fontsize=9)

ax.set_title("Ângulo J entre o eixo de rolagem (R) e o alvo (T) em uma esfera unitária")
ax.set_xlim([-1.2, 1.2])
ax.set_ylim([-1.2, 1.2])
ax.set_zlim([-1.2, 1.2])
ax.set_box_aspect([1,1,1])
plt.show()
