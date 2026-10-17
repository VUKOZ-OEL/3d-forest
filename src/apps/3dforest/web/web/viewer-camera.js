// Double-precision camera math; only camera-relative values are sent to WebGL.
export const add = (a, b) => a.map((x, i) => x + b[i]);
export const sub = (a, b) => a.map((x, i) => x - b[i]);
export const scale = (a, s) => a.map((x) => x * s);
export const dot = (a, b) => a.reduce((s, x, i) => s + x * b[i], 0);
export const length = (a) => Math.hypot(...a);
export const normalize = (a) => scale(a, 1 / (length(a) || 1));
export const cross = (a, b) => [
  a[1] * b[2] - a[2] * b[1],
  a[2] * b[0] - a[0] * b[2],
  a[0] * b[1] - a[1] * b[0],
];
function multiply(a, b) {
  return a.map((row) =>
    [0, 1, 2].map((c) => row.reduce((sum, x, i) => sum + x * b[i][c], 0)),
  );
}
export class ViewerCamera {
  constructor() {
    this.set({
      eye: [1, 1, 1],
      center: [0, 0, 0],
      up: [-1, -1, 1],
      perspective: false,
      lock2d: false,
      fov: 60,
    });
  }
  set(c) {
    this.center = [...c.center];
    this.distance = length(sub(c.eye, c.center));
    const back = normalize(sub(c.eye, c.center)),
      right = normalize(cross(c.up, back));
    this.rows = [right, cross(back, right), back];
    this.perspective = !!c.perspective;
    this.lock2d = !!c.lock2d;
  }
  get eye() {
    return add(this.center, scale(this.rows[2], this.distance));
  }
  get state() {
    return {
      eye: this.eye,
      center: [...this.center],
      up: [...this.rows[1]],
      fov: 60,
      perspective: this.perspective,
      lock2d: this.lock2d,
    };
  }
  rotate(dx, dy, w, h) {
    if (this.lock2d) return;
    const ax = (dy / Math.max(1, h)) * Math.PI,
      az = (dx / Math.max(1, w)) * Math.PI,
      cx = Math.cos(ax),
      sx = Math.sin(ax),
      cz = Math.cos(az),
      sz = Math.sin(az);
    this.rows = multiply(
      multiply(
        [
          [1, 0, 0],
          [0, cx, -sx],
          [0, sx, cx],
        ],
        this.rows,
      ),
      [
        [cz, -sz, 0],
        [sz, cz, 0],
        [0, 0, 1],
      ],
    );
    const back = normalize(this.rows[2]),
      right = normalize(this.rows[0]);
    this.rows = [right, normalize(cross(back, right)), back];
  }
  pan(dx, dy, w, h) {
    const shift = add(
      scale(this.rows[0], (dx / Math.max(1, w)) * this.distance),
      scale(this.rows[1], (-dy / Math.max(1, h)) * this.distance),
    );
    this.center = sub(this.center, shift);
  }
  zoom(dy, h) {
    this.distance = Math.max(
      1,
      Math.min(
        1e12,
        this.distance *
          Math.exp(Math.max(-20, Math.min(20, (4 * dy) / Math.max(1, h)))),
      ),
    );
  }
  get halfHeight() {
    return 0.5780000578 * this.distance;
  }
  get rotation() {
    const r = this.rows;
    return new Float32Array([
      r[0][0],
      r[1][0],
      r[2][0],
      r[0][1],
      r[1][1],
      r[2][1],
      r[0][2],
      r[1][2],
      r[2][2],
    ]);
  }
  depthRange(bounds) {
    if (bounds?.length !== 6) return this.distance * 100;
    const center = [0, 1, 2].map((i) => (bounds[i] + bounds[i + 3]) / 2),
      radius = length(sub(bounds.slice(3), bounds.slice(0, 3))) / 2;
    return Math.max(
      this.distance * 4,
      length(sub(center, this.center)) + radius + this.distance + 1,
    );
  }
  projection(w, h, bounds) {
    const aspect = w / Math.max(1, h),
      d = this.distance,
      far = this.depthRange(bounds),
      near = 0.01 * d;
    if (this.perspective) {
      const f = 1 / Math.tan(Math.PI / 6),
        nf = 1 / (near - far);
      return new Float32Array([
        f / aspect,
        0,
        0,
        0,
        0,
        f,
        0,
        0,
        0,
        0,
        (far + near) * nf,
        -1,
        0,
        0,
        2 * far * near * nf,
        0,
      ]);
    }
    const half = this.halfHeight;
    return new Float32Array([
      1 / (half * aspect),
      0,
      0,
      0,
      0,
      1 / half,
      0,
      0,
      0,
      0,
      -1 / far,
      0,
      0,
      0,
      0,
      1,
    ]);
  }
  project(point, w, h) {
    const p = sub(point, this.center),
      v = this.rows.map((r) => dot(r, p));
    v[2] -= this.distance;
    const aspect = w / Math.max(1, h);
    let x, y;
    if (this.perspective) {
      if (v[2] >= -0.01 * this.distance) return null;
      const f = 1 / Math.tan(Math.PI / 6);
      x = (v[0] * f) / (-v[2] * aspect);
      y = (v[1] * f) / -v[2];
    } else {
      x = v[0] / (this.halfHeight * aspect);
      y = v[1] / this.halfHeight;
    }
    return [((x + 1) * w) / 2, ((1 - y) * h) / 2, -v[2]];
  }
  ray(x, y, w, h, bounds) {
    const nx = (2 * x) / w - 1,
      ny = 1 - (2 * y) / h,
      aspect = w / Math.max(1, h),
      forward = scale(this.rows[2], -1);
    if (this.perspective) {
      const spread = Math.tan(Math.PI / 6);
      return {
        origin: this.eye,
        direction: normalize(
          add(
            forward,
            add(
              scale(this.rows[0], nx * aspect * spread),
              scale(this.rows[1], ny * spread),
            ),
          ),
        ),
      };
    }
    return {
      origin: add(
        add(this.center, scale(this.rows[2], this.depthRange(bounds))),
        add(
          scale(this.rows[0], nx * this.halfHeight * aspect),
          scale(this.rows[1], ny * this.halfHeight),
        ),
      ),
      direction: forward,
    };
  }
}
