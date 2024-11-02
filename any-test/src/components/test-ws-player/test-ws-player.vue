<template>
  <v-container fluid>
    <v-row class="d-flex justify-center">
      <div class="ml-2 mt-2 text-title font-weight-black primary--text">
        ws-player测试
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
        <v-btn class="primary" small @click="onClose">关闭</v-btn>
      </v-row>
    </div>
    <v-row>
      <v-col cols="6" v-for="(item, idx) in taskInfo" :key="idx" v-if="player.display">
        <ws-player :canvasId="'canvas' + idx" :ref="'player' + idx" :canvasWidth="1280" :canvasHeight="720"></ws-player>
      </v-col>
    </v-row>
  </v-container>
</template>

<script>
  import WsPlayerWrapper from "./ws-player-wrapper.vue";
  export default {
    name: "TestWsPlayer",
    components: {
      "ws-player": WsPlayerWrapper,
    },
    data() {
      return {
        srvUrl: "wss://172.16.41.87:20443/media-conv",

        //  srcUrl: ["rtsp://10.1.40.204/live/chn0",
        //  "rtsp://10.1.40.204/live/chn0",],
        taskInfo: [{
          msrc: "sensor://local/000/000/video0",
          taskid: "1"
        }, 
        // {
        //   msrc: "sensor://local/000/000/video0",
        //   taskid: "1"
        // }, {
        //   msrc: "sensor://local/000/000/video0",
        //   taskid: "1"
        // }, {
        //   msrc: "sensor://local/000/000/video0",
        //   taskid: "1"
        // }, {
        //   msrc: "sensor://local/000/000/video0",
        //   taskid: "1"
        // }, {
        //   msrc: "sensor://local/000/000/video0",
        //   taskid: "1"
        // },
        ],
        player: {
          display: true,
        }
      };
    },
    created() {
    },
    mounted() {
    },
    methods: {
      onOpen() {
        this.player.display = true;

        setTimeout(()=>{
          for (let i = 0; i < this.taskInfo.length; i++) {
            this.$refs[`player${i}`][0].play(this.srvUrl, this.taskInfo[i].msrc, this.taskInfo[i].taskid);
          }
        },4000)
        
      },
      onClose() {
        for (let i = 0; i < this.taskInfo.length; i++) {
          this.$refs[`player${i}`][0].wsDestroy();
          //console.log("this.$refs[`player${i}`][0]",this.$refs[`player${i}`][0]);
        }
        this.player.display = false;
      },
      onReset() {
        this.player.display = true;
      }
    },
  };
</script>