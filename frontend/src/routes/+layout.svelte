<script lang="ts">
    import { onMount } from "svelte";
    import { loading, isAuthenticated } from "../stores/auth";
    import { checkAuth } from "$lib/auth";
    import { goto } from "$app/navigation";
    import { page } from "$app/stores";

    import Loader from "$lib/components/Loader.svelte";
    import Sidebar from "$lib/components/Menu.svelte";

    import "./app.css";

    let authChecked = false;

    onMount(async () => {
        await checkAuth();
        authChecked = true;
    });
    $effect(() => {
        if ($loading) return;
    
        const path = $page.url.pathname;
    
        if (!$isAuthenticated && path !== "/") {
            goto("/", { replaceState: true });
            return;
        }
    
        if ($isAuthenticated && path === "/") {
            goto("/dashboard", { replaceState: true });
            return;
        }
    });
</script>

{#if $loading}
    <Loader />
{:else if $isAuthenticated}
    <div class="flex min-h-screen bg-black">
        <Sidebar />
        <div class="flex flex-col flex-1 min-w-0">
            <main class="flex-1 overflow-x-hidden">
                <slot />
            </main>
        </div>
    </div>
{:else}
    <slot />
{/if}