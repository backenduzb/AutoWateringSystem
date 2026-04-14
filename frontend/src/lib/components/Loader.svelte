<script lang="ts">
    import Logo from "$lib/assets/logo.png";
</script>

<div class="w-full min-h-screen flex items-center justify-center bg-black">
    <div class="flex flex-col items-center">
        <img class="w-40 h-40" src={Logo} alt="logo">
        <div class="w-30 h-20 flex items-center justify-around">
            <span class="rounded-full w-4 h-4 bg-teal-600 animate-scale animate-scale-1"></span>
            <span class="rounded-full w-4 h-4 bg-teal-600 animate-scale animate-scale-2"></span>
            <span class="rounded-full w-4 h-4 bg-teal-600 animate-scale animate-scale-3"></span>
        </div>
    </div>
</div>