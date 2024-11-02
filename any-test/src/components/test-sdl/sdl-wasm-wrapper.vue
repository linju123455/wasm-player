<template>
  <canvas :id="canvasId" style="background-color: #000"></canvas>
</template>

<script>
import SDLModule from "@/wasm/test-sdl";
export default {
  name: "SDLWasmWrapper",
  props: {
      canvasId: {
          type: String,
          require: true,
          default: "canvas",
      }
  },
  data() {
    return {
      sdlInst: null,
    };
  },
  async created() {
    this.sdlInst = await SDLModule();
    let canvas = document.getElementById(this.canvasId)
    this.sdlInst.canvas = canvas;
    this.sdlInst._Init(canvas.width, canvas.height);
  },
  methods: {
      draw(x, y, w, h, r, g, b) {
          this.sdlInst._DrawRect(x, y, w, h, r, g, b);
      },
      rendererWidth() {
          return document.getElementById(this.canvasId).width;
      },
      rendererHeight() {
          return document.getElementById(this.canvasId).height;
      }
  }
};
</script>