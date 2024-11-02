<template>
  <canvas :id="canvasId" ref="scene" :width="canvasWidth" :height="canvasHeight" style="background-color: #000"></canvas>
</template>

<script>
import WsPlayerModule from "@/wasm/ws-player.js";
export default {
  name: "ws-player-wrapper",
  props: {
    canvasId: {
      type: String,
      require: true,
      default: "canvas",
    },
    canvasWidth: {
      type: Number,
      default: 320,
    },
    canvasHeight: {
      type: Number,
      default: 240,
    }
  },
  data() {
    return {
      wasmInst: null,
      ws: null,
      seqNum: 0,
      processor: null,
    };
  },
  created() {},
  async mounted() {
    this.wasmInst = await WsPlayerModule();
    let canvas = document.getElementById(this.canvasId);
    this.wasmInst.canvas = canvas;

    this.processor = this.wasmInst._CreateProcessor(
     canvas.width,
     canvas.height
    );

    // this.wasmInst._Start(canvas.width,canvas.height);
  },
  beforeDestroy() {
    if (this.ws) {
      this.ws.close();
    }

    //this.wasmInst._DestroyProcessor(this.processor);
    //this.processor = null;
    //this.wasmInst = null;
    console.log("beforeDestroy del wasm inst...");
  },
  methods: {
    wsDestroy() {
      if (this.ws) {
      this.ws.close();
    }

    this.wasmInst._DestroyProcessor(this.processor);
    this.wasmInst.PThread.terminateAllThreads();

    this.processor = null;

    if (this.wasmInst !== null) {
      // 将Wasm实例置为null，以便让垃圾回收器回收相关内存
      this.wasmInst = null;
      console.log("wsDestroy del wasm inst...");
    }
    },
    wsReset() {
    
    },
    play(srvUrl, srcUrl, task_id){
      if (this.ws) {
        console.log("已经存在ws...",this.ws);
        ws.close();
      }
      this.ws = new WebSocket(srvUrl);
      
      this.ws.binaryType = "arraybuffer";
      this.ws.onopen = () => {
        this.seqNum++;
        var obj = {
          task_id: task_id,
          action: 20,
          seq: this.seqNum,
          media_src: srcUrl,
        };

        this.ws.send(JSON.stringify(obj));
        console.log("发送转换请求...");
      };

      this.ws.onmessage = (evt) => {
        var recvData = evt.data;
        if (this.ws.binaryType == "arraybuffer") {
          let u8arr = new Int8Array(recvData);
          const parsePacket = (processor, dataArr) => {
            new Promise((resolve, reject) => {
                let res = this.wasmInst.ccall(
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
                // let ptr_data_size = this.WsPlayerModule._malloc(1 * 4);
                // let ptr_width = this.WsPlayerModule._malloc(1 * 4);
                // let ptr_height = this.WsPlayerModule._malloc(1 * 4);
                // let ptr_pts = this.WsPlayerModule._malloc(1 * 4);
                // let ptr_data = this.WsPlayerModule._GetFrameCacheData(
                //   this.processor[idx],
                //   ptr_data_size,
                //   ptr_width,
                //   ptr_height,
                //   ptr_pts
                // );
                // if (!ptr_data) {
                //   this.WsPlayerModule._free(ptr_data_size);
                //   this.WsPlayerModule._free(ptr_width);
                //   this.WsPlayerModule._free(ptr_height);
                //   this.WsPlayerModule._free(ptr_pts);
                //   return;
                // }
                // // //console.log(ptr_data, this.WsPlayerModule.HEAPU32[ptr_data_size>>2], this.WsPlayerModule.HEAP32[ptr_width>>2], this.WsPlayerModule.HEAP32[ptr_height>>2], this.WsPlayerModule.HEAPU32[ptr_pts>>2]);
                // let data_size = this.WsPlayerModule.HEAPU32[ptr_data_size >> 2];
                // let data = this.WsPlayerModule.HEAPU8.subarray(
                //   ptr_data,
                //   ptr_data + data_size
                // );
                // if (data) {
                //   this.$refs[`renderer${idx}`][0].renderImg(
                //     this.WsPlayerModule.HEAP32[ptr_width >> 2],
                //     this.WsPlayerModule.HEAP32[ptr_height >> 2],
                //     data
                //   );
                // }
                // this.WsPlayerModule._free(ptr_data_size);
                // this.WsPlayerModule._free(ptr_width);
                // this.WsPlayerModule._free(ptr_height);
                // this.WsPlayerModule._free(ptr_pts);
                // this.WsPlayerModule._FreeFrameData(ptr_data);
              })
              .catch((err) => {
                console.log(err);
              });
          };
          parsePacket(this.processor, u8arr);
        } else {
          console.log(recvData);
        }
      };

      this.ws.onclose = () => {
        console.log("连接中断...");
      };
    },
  },
};
</script>