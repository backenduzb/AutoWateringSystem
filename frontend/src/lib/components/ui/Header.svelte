<script lang="ts">
    import { user } from "../../../stores/auth";
    import { logout } from "$lib/auth";
    import { goto } from "$app/navigation";
    import { toggleMenu } from "../../../stores/menu";

    let userDropdownOpen = $state(false);

    function toggleUserDropdown() {
        userDropdownOpen = !userDropdownOpen;
    }

    function handleClickOutside(node: HTMLElement) {
        const onClick = (e: MouseEvent) => {
            if (!node.contains(e.target as Node)) {
                userDropdownOpen = false;
            }
        };
        document.addEventListener("click", onClick, true);
        return {
            destroy() {
                document.removeEventListener("click", onClick, true);
            },
        };
    }

    let firstName = $derived($user?.data?.first_name ?? "");
    let lastName = $derived($user?.data?.last_name ?? "");

    let initials = $derived(
        (firstName?.[0] ?? "").toUpperCase() +
            (lastName?.[0] ?? "").toUpperCase(),
    );

    let fullName = $derived(
        [firstName, lastName].filter(Boolean).join(" ") || "common.user",
    );
</script>

<header
    class="sticky top-0 z-30 h-16 w-full bg-transparent
           border-b border-teal-700 backdrop-blur-sm
           shadow-sm flex items-center justify-between px-4 gap-3"
>
    <button
        onclick={toggleMenu}
        aria-label="Toggle sidebar"
        class="flex items-center justify-center w-9 h-9 rounded-lg
               text-teal-500
               hover:bg-teal-400/10
               transition cursor-pointer shrink-0"
    >
        <svg
            xmlns="http://www.w3.org/2000/svg"
            viewBox="0 0 24 24"
            fill="none"
            stroke="currentColor"
            stroke-width="1.8"
            class="size-5"
        >
            <path
                stroke-linecap="round"
                stroke-linejoin="round"
                d="M3.75 6.75h16.5M3.75 12h16.5m-16.5 5.25h16.5"
            />
        </svg>
    </button>

    <div class="flex items-center gap-1 sm:gap-2 ml-auto">
        <div class="relative" use:handleClickOutside>
            <button
                onclick={toggleUserDropdown}
                class="flex items-center gap-2 pl-2 pr-1.5 py-1.5 rounded-xl
                       hover:bg-teal-600/20
                       transition cursor-pointer"
            >
                <span
                    class="text-sm font-medium text-teal-400 hidden sm:block max-w-64 truncate"
                >
                    {fullName}
                </span>

                <div
                    class="w-8 h-8 rounded-full bg-teal-600
                            flex items-center justify-center
                            text-white text-sm font-semibold select-none shrink-0"
                >
                    {#if initials}
                        {initials}
                    {:else}
                        <svg
                            xmlns="http://www.w3.org/2000/svg"
                            viewBox="0 0 24 24"
                            fill="currentColor"
                            class="size-5"
                        >
                            <path
                                fill-rule="evenodd"
                                d="M7.5 6a4.5 4.5 0 1 1 9 0 4.5 4.5 0 0 1-9 0ZM3.751 20.105a8.25 8.25 0 0 1 16.498 0 .75.75 0 0 1-.437.695A18.683 18.683 0 0 1 12 22.5c-2.786 0-5.433-.608-7.812-1.7a.75.75 0 0 1-.437-.695Z"
                                clip-rule="evenodd"
                            />
                        </svg>
                    {/if}
                </div>

                <svg
                    xmlns="http://www.w3.org/2000/svg"
                    viewBox="0 0 20 20"
                    fill="currentColor"
                    class={`size-4 text-slate-400 dark:text-slate-500
                             transition-transform duration-200 shrink-0
                             ${userDropdownOpen ? "rotate-180" : ""}`}
                >
                    <path
                        fill-rule="evenodd"
                        d="M5.22 8.22a.75.75 0 0 1 1.06 0L10 11.94l3.72-3.72a.75.75 0 1 1 1.06 1.06l-4.25 4.25a.75.75 0 0 1-1.06 0L5.22 9.28a.75.75 0 0 1 0-1.06Z"
                        clip-rule="evenodd"
                    />
                </svg>
            </button>

            {#if userDropdownOpen}
                <div
                    class="absolute right-0 top-full mt-2 w-52
                            bg-teal-500/10 
                            border border-teal-700
                            rounded-xl shadow-lg overflow-hidden z-50"
                >
                    <div
                        class="px-4 py-3 border-b border-teal-700 bg-teal-400/20 "
                    >
                        <p
                            class="text-xs text-teal-500 font-medium uppercase tracking-wide"
                        >
                            Akkaunt
                        </p>
                        <p
                            class="text-sm font-semibold text-teal-400 mt-0.5 truncate"
                        >
                            {fullName}
                        </p>
                    </div>

                    <ul class="py-1">
                        <li>
                            <button
                                onclick={() => {
                                    userDropdownOpen = false;
                                    goto("/profile");
                                }}
                                class="w-full flex items-center gap-3 px-4 py-2.5 text-sm
                                       text-teal-400 
                                       hover:bg-teal-700/10
                                       transition cursor-pointer"
                            >
                                <svg
                                    xmlns="http://www.w3.org/2000/svg"
                                    viewBox="0 0 24 24"
                                    fill="none"
                                    stroke="currentColor"
                                    stroke-width="1.8"
                                    class="size-4 text-teal-400"
                                >
                                    <path
                                        stroke-linecap="round"
                                        stroke-linejoin="round"
                                        d="M15.75 6a3.75 3.75 0 1 1-7.5 0 3.75 3.75 0 0 1 7.5 0ZM4.501 20.118a7.5 7.5 0 0 1 14.998 0A17.933 17.933 0 0 1 12 21.75c-2.676 0-5.216-.584-7.499-1.632Z"
                                    />
                                </svg>
                                Sozlamalar
                            </button>
                        </li>
                        <li
                            class="border-t border-slate-100 dark:border-slate-700 mt-1 pt-1"
                        >
                            <button
                                onclick={() => {
                                    userDropdownOpen = false;
                                    logout();
                                }}
                                class="w-full flex items-center gap-3 px-4 py-2.5 text-sm
                                       text-red-600 dark:text-red-400
                                       hover:bg-red-50 dark:hover:bg-red-900/20
                                       transition cursor-pointer"
                            >
                                <svg
                                    xmlns="http://www.w3.org/2000/svg"
                                    viewBox="0 0 24 24"
                                    fill="none"
                                    stroke="currentColor"
                                    stroke-width="1.8"
                                    class="size-4"
                                >
                                    <path
                                        stroke-linecap="round"
                                        stroke-linejoin="round"
                                        d="M15.75 9V5.25A2.25 2.25 0 0 0 13.5 3h-6a2.25 2.25 0 0 0-2.25 2.25v13.5A2.25 2.25 0 0 0 7.5 21h6a2.25 2.25 0 0 0 2.25-2.25V15m3 0 3-3m0 0-3-3m3 3H9"
                                    />
                                </svg>
                                Chiqish
                            </button>
                        </li>
                    </ul>
                </div>
            {/if}
        </div>
    </div>
</header>
