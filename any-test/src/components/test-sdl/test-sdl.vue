<template>
  <v-container>
    <v-row class="d-flex justify-center">
      <div class="ml-2 mt-2 text-title font-weight-black primary--text">
        sdl测试
      </div>
    </v-row>

    <v-row class="d-flex justify-center">
      <v-col cols="6" v-for="(item, index) in sceneNum" :key="index">
        <sdl-wasm-wrapper
          :ref="'scene' + index"
          :canvasId="'canvas' + index"
        ></sdl-wasm-wrapper>
        <span>{{ item }} - {{ index }}</span>
      </v-col>
    </v-row>
  </v-container>
</template>

<script>
import SDLWasmWrapper from "./sdl-wasm-wrapper.vue";
export default {
  name: "SDL",
  components: {
    "sdl-wasm-wrapper": SDLWasmWrapper,
  },
  data() {
    return {
      timer: null,
      sceneNum: 4,
    };
  },
  created() {
    this.timer = setInterval(() => {
      for (let i = 0; i < this.sceneNum; i++) {
        let r = Math.floor(Math.random() * 255);
        let g = Math.floor(Math.random() * 255);
        let b = Math.floor(Math.random() * 255);

        let wrapper = this.$refs[`scene${i}`][0];
        let w = wrapper.rendererWidth();
        let h = wrapper.rendererHeight();

        this.$refs[`scene${i}`][0].draw(
          10 + 20 * i,
          10 + 20 * i,
          w / 4,
          h / 4,
          r,
          g,
          b
        );
      }
    }, 1000);
  },
  beforeDestroy() {
    if (this.timer) {
      clearInterval(this.timer);
      this.timer = null;
    }
  },
};
</script>