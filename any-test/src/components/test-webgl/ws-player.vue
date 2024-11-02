<template>
  <v-main>
    <v-container>
      <v-row class="d-flex justify-center">
        <div class="ml-2 mt-2 text-title font-weight-black primary--text">
          webgl测试
        </div>
      </v-row>
      <div>
        <v-row class="ma-1 d-flex align-baseline">
          <v-col>
            <v-text-field v-model="srvUrl" label="服务器地址" />
          </v-col>
          <!-- <v-col>
                    <v-text-field v-model="srcUrl" label="视频源地址" />
                </v-col> -->
          <v-btn class="primary" small @click="onOpen">打开</v-btn>
        </v-row>
      </div>
      <v-row>
        <v-col cols="6" v-for="(item, idx) in srcUrl" :key="idx">
          <d-video-renderer
            :ref="'renderer' + idx"
            :id="`render${idx}`"
            :width="renderWidth"
            :height="renderHeight"
          ></d-video-renderer>
        </v-col>
      </v-row>
    </v-container>
  </v-main>
</template>

<script>
import DVideoRenderer from "./DVideoRenderer.vue";
import WsDecModule from "@/wasm/ema-ws-decoder.js";
export default {
  name: "WsPlayer",
  components: {
    "d-video-renderer": DVideoRenderer,
  },
  data() {
    return {
      DecModule: null,
      processor: [],
      ws: [],
      renderWidth: "400",
      renderHeight: "320",

    //   srvUrl: "ws://10.1.40.164:9255/media-conv",
      srvUrl: "ws://192.168.0.115:9255/media-conv",
      // srcUrl: ["rtsp://10.1.40.9/live/1",
      //     "rtsp://10.1.40.9/live/2",
      //     "rtsp://10.1.40.9/live/3",
      //     "rtsp://10.1.40.9/live/4"],

      srcUrl: [
        "rtsp://admin:dxh123456@192.168.0.168/Streaming/channels/101",
        "rtsp://admin:dxh123456@192.168.0.168/Streaming/channels/101",
      ],

      // srcUrl: ["rtsp://10.1.40.9/live/3"],

      // srcUrl: ["rtsp://10.1.40.9/live/2",
      //     "rtsp://10.1.40.9/live/3"],

      // srcUrl: ["rtsp://10.1.40.9/live/2",
      //     "rtsp://10.1.40.9/live/3",
      //     "rtsp://10.1.40.9/live/4"],
      seqNum: 0,

      frmCnt: [], //test code
    };
  },
  async created() {
    this.DecModule = await WsDecModule();
    for (let i = 0; i < this.srcUrl.length; i++) {
      this.processor.push(this.DecModule._CreateProcessor());
      this.ws.push(null);

      this.frmCnt.push(0);
    }
  },
  mounted() {},
  beforeDestroy() {
    for (let i = 0; i < this.ws.length; i++) {
      if (this.ws[i]) {
        this.ws[i].close();
      }
    }

    if (this.timer) {
      clearInterval(this.timer);
      this.timer = null;
    }
  },
  methods: {
    onOpen() {
      for (let i = 0; i < this.ws.length; i++) {
        this.play(i);
      }
      // this.play(0)

      //test code
      if (this.timer) {
        clearInterval(this.timer);
        this.timer = null;
      }
      this.timer = setInterval(() => {
        console.log(
          "@@@",
          this.frmCnt[0],
          this.frmCnt[1],
          this.frmCnt[2],
          this.frmCnt[3]
        );
        this.frmCnt[0] = this.frmCnt[1] = this.frmCnt[2] = this.frmCnt[3] = 0;
      }, 1000);
    },
    play(idx) {
      this.ws[idx] = new WebSocket(this.srvUrl);
      this.ws[idx].binaryType = "arraybuffer";
      this.ws[idx].onopen = () => {
        this.seqNum++;
        var obj = {
          action: 20,
          seq: this.seqNum,
          media_src: this.srcUrl[idx],
        };

        this.ws[idx].send(JSON.stringify(obj));
        console.log("发送转换请求...");
      };

      this.ws[idx].onmessage = (evt) => {
        var recvData = evt.data;
        if (this.ws[idx].binaryType == "arraybuffer") {
          let u8arr = new Int8Array(recvData);
          const parsePacket = (processor, dataArr) => {
            new Promise((resolve, reject) => {
              this.frmCnt[idx]++; //test code

              let res = this.DecModule.ccall(
                "AsyncParsePacket",
                "number",
                ["number", "array", "number"],
                [processor, dataArr, dataArr.byteLength]
              );
              if (res < 0) {
                reject(-1);
              } else {
                resolve(0);
              }
            })
              .then((res) => {
                let ptr_data_size = this.DecModule._malloc(1 * 4);
                let ptr_width = this.DecModule._malloc(1 * 4);
                let ptr_height = this.DecModule._malloc(1 * 4);
                let ptr_pts = this.DecModule._malloc(1 * 4);
                let ptr_data = this.DecModule._GetFrameCacheData(
                  this.processor[idx],
                  ptr_data_size,
                  ptr_width,
                  ptr_height,
                  ptr_pts
                );

                if (!ptr_data) {
                  this.DecModule._free(ptr_data_size);
                  this.DecModule._free(ptr_width);
                  this.DecModule._free(ptr_height);
                  this.DecModule._free(ptr_pts);
                  return;
                }

                // //console.log(ptr_data, this.DecModule.HEAPU32[ptr_data_size>>2], this.DecModule.HEAP32[ptr_width>>2], this.DecModule.HEAP32[ptr_height>>2], this.DecModule.HEAPU32[ptr_pts>>2]);
                let data_size = this.DecModule.HEAPU32[ptr_data_size >> 2];
                let data = this.DecModule.HEAPU8.subarray(
                  ptr_data,
                  ptr_data + data_size
                );
                if (data) {
                  this.$refs[`renderer${idx}`][0].renderImg(
                    this.DecModule.HEAP32[ptr_width >> 2],
                    this.DecModule.HEAP32[ptr_height >> 2],
                    data
                  );
                }

                this.DecModule._free(ptr_data_size);
                this.DecModule._free(ptr_width);
                this.DecModule._free(ptr_height);
                this.DecModule._free(ptr_pts);

                this.DecModule._FreeFrameData(ptr_data);
              })
              .catch((err) => {
                console.log(err);
              });
          };
          parsePacket(this.processor[idx], u8arr);

          // let ptr_data_size = this.DecModule._malloc(1 * 4);
          // let ptr_width = this.DecModule._malloc(1 * 4);
          // let ptr_height = this.DecModule._malloc(1 * 4);
          // let ptr_pts = this.DecModule._malloc(1 * 4);
          // let ptr_data = this.DecModule._GetFrameData(this.processor[idx], ptr_data_size, ptr_width, ptr_height, ptr_pts);

          // if (!ptr_data) {
          //     this.DecModule._free(ptr_data_size);
          //     this.DecModule._free(ptr_width);
          //     this.DecModule._free(ptr_height);
          //     this.DecModule._free(ptr_pts);
          //     return;
          // }

          // //console.log(ptr_data, this.DecModule.HEAPU32[ptr_data_size>>2], this.DecModule.HEAP32[ptr_width>>2], this.DecModule.HEAP32[ptr_height>>2], this.DecModule.HEAPU32[ptr_pts>>2]);
          // let data_size = this.DecModule.HEAPU32[ptr_data_size >> 2];
          // let data = this.DecModule.HEAPU8.subarray(ptr_data, ptr_data + data_size);
          // if (data) {
          //     this.$refs[`renderer${idx}`][0].renderImg(this.DecModule.HEAP32[ptr_width >> 2], this.DecModule.HEAP32[ptr_height >> 2], data);
          // }

          // this.DecModule._free(ptr_data_size);
          // this.DecModule._free(ptr_width);
          // this.DecModule._free(ptr_height);
          // this.DecModule._free(ptr_pts);
          // this.DecModule._FreeFrameData(ptr_data);
        } else {
          console.log(recvData);
        }
      };

      this.ws[idx].onclose = () => {
        console.log("连接中断...");
      };
    },
  },
};
</script>