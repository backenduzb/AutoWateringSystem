<script lang="ts">
    import { page } from "$app/stores";
    import { goto } from "$app/navigation";
    import { menuOpen, closeMenu } from "../../stores/menu";

    const navItems = [
        {
            href: "/dashboard",
            labelKey: "Bosh sahifa",
            icon: `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" class="size-5"><path stroke-linecap="round" stroke-linejoin="round" d="M3.75 6A2.25 2.25 0 0 1 6 3.75h2.25A2.25 2.25 0 0 1 10.5 6v2.25a2.25 2.25 0 0 1-2.25 2.25H6a2.25 2.25 0 0 1-2.25-2.25V6ZM3.75 15.75A2.25 2.25 0 0 1 6 13.5h2.25a2.25 2.25 0 0 1 2.25 2.25V18a2.25 2.25 0 0 1-2.25 2.25H6A2.25 2.25 0 0 1 3.75 18v-2.25ZM13.5 6a2.25 2.25 0 0 1 2.25-2.25H18A2.25 2.25 0 0 1 20.25 6v2.25A2.25 2.25 0 0 1 18 10.5h-2.25a2.25 2.25 0 0 1-2.25-2.25V6ZM13.5 15.75a2.25 2.25 0 0 1 2.25-2.25H18a2.25 2.25 0 0 1 2.25 2.25V18A2.25 2.25 0 0 1 18 20.25h-2.25A2.25 2.25 0 0 1 13.5 18v-2.25Z" /></svg>`,
        },
        {
            href: "/profile",
            labelKey: "Profil",
            icon: `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" class="size-5"><path stroke-linecap="round" stroke-linejoin="round" d="M15.75 6a3.75 3.75 0 1 1-7.5 0 3.75 3.75 0 0 1 7.5 0ZM4.501 20.118a7.5 7.5 0 0 1 14.998 0A17.933 17.933 0 0 1 12 21.75c-2.676 0-5.216-.584-7.499-1.632Z" /></svg>`,
        },
    ];

    function handleNavClick(href: string) {
        if (window.innerWidth < 768) closeMenu();
        goto(href);
    }

    function isActive(href: string) {
        return $page.url.pathname.startsWith(href);
    }
</script>
{#if $menuOpen}
    <div
        class="fixed inset-0 bg-black/40 z-30 md:hidden"
        onclick={closeMenu}
        aria-hidden="true"
    ></div>

<aside
    class={`
        fixed md:static inset-y-0 left-0
        z-40 md:z-auto
        w-64 shrink-0
        flex flex-col
        bg-black
        border-r  dark:border-teal-800
        min-h-screen
        transform transition-transform duration-300 ease-in-out
        ${$menuOpen ? "translate-x-0" : "-translate-x-full md:translate-x-0"}
    `}
>
    <div
        class="h-16 flex items-center justify-between px-5 border-b  border-teal-800 shrink-0"
    >
        <span
            class="text-base font-bold text-teal-400 tracking-tight"
        >
            Zamonaviy fermer
        </span>

        <button
            onclick={closeMenu}
            class="md:hidden flex items-center justify-center w-8 h-8 rounded-lg text-slate-500  hover:bg-green-100/10  transition cursor-pointer"
            aria-label="Close menu"
        >
            <svg
                xmlns="http://www.w3.org/2000/svg"
                viewBox="0 0 24 24"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                class="size-5"
            >
                <path
                    stroke-linecap="round"
                    stroke-linejoin="round"
                    d="M6 18 18 6M6 6l12 12"
                />
            </svg>
        </button>
    </div>
    <nav class="flex-1 p-3 space-y-0.5 overflow-y-auto">
        {#each navItems as item}
            <button
                onclick={() => handleNavClick(item.href)}
                class={`
                    w-full flex items-center gap-3 px-3 py-2.5 rounded-xl text-sm font-medium
                    transition-all duration-150 cursor-pointer text-left
                    ${
                        isActive(item.href)
                            ? "bg-teal-900/20 text-teal-400"
                            : "text-slate-600  hover:bg-green-100/10  hover:text-slate-800 dark:hover:text-slate-200"
                    }
                `}
            >
                <span
                    class={`absolute left-0 w-1 h-6 rounded-r-full bg-teal-600 dark:bg-teal-400 transition-opacity ${isActive(item.href) ? "opacity-100" : "opacity-0"}`}
                ></span>

                <span class="shrink-0">{@html item.icon}</span>

                <span>{item.labelKey}</span>
            </button>
        {/each}
    </nav>

    <div
        class="px-5 py-4 border-t border-teal-800 shrink-0"
    >
        <p class="text-xs text-teal-500">v1.0.0</p>
    </div>
</aside>
{/if}