import Vue from 'vue'
import VueRouter from 'vue-router'
import Hello from '@/components/HelloWorld.vue'
import dmath from '@/components/dmath.vue'
import WsPlayer from '@/components/test-webgl/ws-player.vue'
import TestWsPlayer from '@/components/test-ws-player/test-ws-player.vue'
import TestSDL from '@/components/test-sdl/test-sdl.vue'

Vue.use(VueRouter)

const routes = [
  {
    path: '/',
    name: 'Hello',
    component: Hello
  },
  {
    path: '/test-dmath',
    component: dmath
  },
  {
    path: '/test-webgl',
    component: WsPlayer,
  },
  {
    path: '/test-ws-player',
    component: TestWsPlayer,
  },
  {
    path: '/test-sdl',
    component: TestSDL,
  }
]

const router = new VueRouter({
  mode: 'history',
  base: process.env.BASE_URL,
  routes
})

export default router
