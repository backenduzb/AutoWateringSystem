/**@type {import('tailwindcss').Config} */

module.exports = {
    content: [
        "./src/**/*.{html, js, ts, svelte}",
        "./node_modules/flowbite/**/*.js"
    ],
    extend: {
        fontFamily: {
            sans: ['Inter', 'sans-serif'],
        },
    },
    plugins: [
        require('flowbite/plugin')
    ],
};