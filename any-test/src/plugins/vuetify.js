// import Vue from 'vue';
// import Vuetify from 'vuetify/lib/framework';

// Vue.use(Vuetify);

// export default new Vuetify({
// });

// src/plugins/vuetify.js

import Vue from 'vue'
import Vuetify from 'vuetify'
import 'vuetify/dist/vuetify.min.css'

// import 'material-design-icons-iconfont/dist/material-design-icons.css' // Ensure you are using css-loader
// import '@mdi/font/css/materialdesignicons.css' // Ensure you are using css-loader

Vue.use(Vuetify)

// const opts = {}
const opts = {
    theme: {
        light: {
            primary: '#2196f3',
            secondary: '#3f51b5',
            accent: '#03a9f4',
            error: '#f44336',
            warning: '#ff9800',
            info: '#607d8b',
            success: '#4caf50'
        }
    },
    icons: {
        iconfont: 'mdi'
    }
}

export default new Vuetify(opts)