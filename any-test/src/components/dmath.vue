<template>
    <v-container>
        <v-row class="d-flex justify-center">
            <div class="ml-2 mt-2 text-title font-weight-black primary--text">dmath测试</div>
        </v-row>
        <v-row class="d-flex justify-start ma-2">
            <v-btn class="primary" @click="onAdd">add</v-btn>
        </v-row>
    </v-container>
</template>

<script>
import DMathModule from '@/wasm/dmath.js'
export default {
    name: 'dmath',
    data() {
        return {
            dMathModule: null,
        };
    },
    async created() {
        this.dMathModule = await new DMathModule();
        console.log(this.dMathModule)
    },
    mounted() {
    },
    methods: {
        onAdd() {
            if (this.dMathModule) {
                console.log("1+2=", this.dMathModule._dmath_add(1,2));
            }
        }
    }
}
</script>

<style scoped>
.v-btn {
    text-transform: none;
}
</style>
