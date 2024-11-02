<template>
  <v-container>
    <v-btn class="primary mx-1" @click="onTestWebGL">webgl测试</v-btn>
    <v-btn class="primary mx-1" @click="onTestWsPlayer">ws播放测试</v-btn>
    <v-btn class="primary mx-1" @click="onTestSDL">sdl测试</v-btn>
    <v-btn class="primary mx-1" @click="onTestPthreads">多线程测试</v-btn>
    <v-btn class="primary mx-1" @click="onTestDmath">dmath测试</v-btn>
  </v-container>
</template>

<script>
export default {
  name: "HelloWorld",
  data: () => ({}),
  methods: {
    onTestWebGL() {
      this.$router.push({
        path: "/test-webgl",
      });
    },
    onTestWsPlayer() {
      this.$router.push({
        path: "/test-ws-player",
      });
    },
    onTestSDL() {
      this.$router.push({
        path: "/test-sdl",
      });
    },
    onTestPthreads() {
      this.$router.push({
        path: "/test-pthreads",
      });
    },
    onTestDmath() {
      this.$router.push({
        path: "/test-dmath",
      });
    },
  },
};
</script>

<style scoped>
.v-btn {
  text-transform: none;
}
</style>