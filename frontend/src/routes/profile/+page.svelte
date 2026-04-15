<script lang="ts">
    import { onMount } from "svelte";
    import { user } from "../../stores/auth";
    import { logout } from "$lib/auth";
    import { ROOT_URL } from "$lib/index";

    let first_name = "";
    let last_name = "";
    let password = "";
    let showPassword = false;

    let submitting = false;
    let fieldErrors: Record<string, string> = {};
    let successMessage = "";

    onMount(() => {
        const u = $user?.data;
        if (u) {
            first_name = u.first_name ?? "";
            last_name = u.last_name ?? "";
        }
    });

    async function handleUpdate(e: Event) {
        e.preventDefault();
        fieldErrors = {};
        successMessage = "";
        submitting = true;

        const body: Record<string, string> = { first_name, last_name };
        if (password.trim()) body.password = password;

        try {
            const token = localStorage.getItem("access");
            const res = await fetch(`${ROOT_URL}/accounts/me/`, {
                method: "PATCH",
                headers: {
                    "Content-Type": "application/json",
                    Authorization: `Token ${token}`,
                },
                body: JSON.stringify(body),
            });

            const data = await res.json();

            if (res.ok) {
                successMessage = "Ma'lumotlar muvaffaqiyatli yangilandi";
                password = "";
                user.update((u) => {
                    if (!u) return u;
                    return {
                        ...u,
                        data: { ...u.data, ...data.data },
                    };
                });
            } else {
                if (typeof data === "object" && !Array.isArray(data)) {
                    for (const key in data) {
                        const val = data[key];
                        fieldErrors[key] = Array.isArray(val)
                            ? val[0]
                            : String(val);
                    }
                } else {
                    fieldErrors.general =
                        "Ma'lumotlarni yangilashda xato yuz berdi";
                }
            }
        } catch {
            fieldErrors.general = "Serverga ulanishda xato yuz berdi";
        } finally {
            submitting = false;
        }
    }

    const inputClass = (field: string) =>
        `w-full h-11 px-3 rounded-lg border transition focus:outline-none focus:ring-3 bg-transparent bg-teal-700 text-white ${
            fieldErrors[field]
                ? "border-red-500 focus:ring-red-500/50 focus:border-red-500 dark:border-red-400"
                : "border-teal-300 focus:ring-teal-500/50 focus:border-teal-500"
        }`;
</script>

<div class="min-h-full bg-black">
    <main class="max-w-xl mx-auto px-4 py-8 space-y-5">
        <div
            class="flex items-center gap-4 border border-teal-800 rounded-2xl shadow-sm px-6 py-5"
        >
            <div
                class="w-14 h-14 rounded-full bg-teal-500 flex items-center justify-center text-white text-xl font-bold select-none shrink-0"
            >
                {($user?.data?.first_name?.[0] ?? "").toUpperCase()}{(
                    $user?.data?.last_name?.[0] ?? ""
                ).toUpperCase()}
            </div>
            <div>
                <p class="text-lg font-semibold text-teal-800">
                    {$user?.data?.first_name ?? ""}
                    {$user?.data?.last_name ?? ""}
                </p>
                <p class="text-sm dark:text-slate-400">
                    @{$user?.data?.username ?? ""}
                </p>
                {#if $user?.data?.department}
                    <span
                        class="inline-block mt-1 text-xs font-medium bg-blue-50 dark:bg-blue-900/20 text-blue-600 dark:text-blue-400 border border-blue-100 dark:border-blue-800 rounded-full px-2.5 py-0.5"
                    >
                    </span>
                {/if}
            </div>
        </div>

        <div
            class="rounded-2xl border dark:border-teal-700 shadow-sm overflow-hidden"
        >
            <div class="px-6 py-4 border-b border-teal-700">
                <h2 class="text-base font-semibold text-teal-500">
                    Tahrirlash
                </h2>
            </div>

            <form onsubmit={handleUpdate} class="px-6 py-5 space-y-4">
                <div class="grid grid-cols-1 sm:grid-cols-2 gap-4">
                    <div class="flex flex-col gap-1.5">
                        <p class="text-sm font-medium text-teal-500">
                            Username
                        </p>
                        <div
                            class="w-full h-11 px-3 rounded-lg border border-teal-700 flex items-center text-teal-500 text-sm select-none"
                        >
                            @{$user?.data?.username ?? ""}
                        </div>
                    </div>
                </div>

                <hr class=" border-teal-700" />

                <div class="grid grid-cols-1 sm:grid-cols-2 gap-4">
                    <div class="flex flex-col gap-1.5">
                        <label
                            for="first_name"
                            class="text-sm font-medium text-teal-400"
                        >
                            Ism
                        </label>
                        <input
                            id="first_name"
                            type="text"
                            bind:value={first_name}
                            class={inputClass("first_name")}
                        />
                        {#if fieldErrors.first_name}
                            <span class="text-xs  text-red-400"
                                >{fieldErrors.first_name}</span
                            >
                        {/if}
                    </div>

                    <div class="flex flex-col gap-1.5">
                        <label
                            for="last_name"
                            class="text-sm font-medium text-teal-400"
                        >
                            Familya
                        </label>
                        <input
                            id="last_name"
                            type="text"
                            bind:value={last_name}
                            class={inputClass("last_name")}
                        />
                        {#if fieldErrors.last_name}
                            <span class="text-xs  text-red-400"
                                >{fieldErrors.last_name}</span
                            >
                        {/if}
                    </div>
                </div>

                <div class="flex flex-col gap-1.5">
                    <label
                        for="password"
                        class="text-sm font-medium text-teal-400"
                    >
                        Yangi parol
                        <span class="dark:text-teal-500 font-normal"></span>
                    </label>
                    <div class="relative">
                        <input
                            id="password"
                            type={showPassword ? "text" : "password"}
                            bind:value={password}
                            placeholder="Parol"
                            class={`${inputClass("password")} pr-11`}
                        />
                        <button
                            type="button"
                            onclick={() => (showPassword = !showPassword)}
                            class="absolute right-3 top-1/2 -translate-y-1/2 text-teal-400 hover:text-teal-600 transition cursor-pointer"
                            tabindex="-1"
                        >
                            {#if showPassword}
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
                                        d="M3.98 8.223A10.477 10.477 0 0 0 1.934 12C3.226 16.338 7.244 19.5 12 19.5c.993 0 1.953-.138 2.863-.395M6.228 6.228A10.451 10.451 0 0 1 12 4.5c4.756 0 8.773 3.162 10.065 7.498a10.522 10.522 0 0 1-4.293 5.774M6.228 6.228 3 3m3.228 3.228 3.65 3.65m7.894 7.894L21 21m-3.228-3.228-3.65-3.65m0 0a3 3 0 1 0-4.243-4.243m4.242 4.242L9.88 9.88"
                                    />
                                </svg>
                            {:else}
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
                                        d="M2.036 12.322a1.012 1.012 0 0 1 0-.639C3.423 7.51 7.36 4.5 12 4.5c4.638 0 8.573 3.007 9.963 7.178.07.207.07.431 0 .639C20.577 16.49 16.64 19.5 12 19.5c-4.638 0-8.573-3.007-9.963-7.178Z"
                                    />
                                    <path
                                        stroke-linecap="round"
                                        stroke-linejoin="round"
                                        d="M15 12a3 3 0 1 1-6 0 3 3 0 0 1 6 0Z"
                                    />
                                </svg>
                            {/if}
                        </button>
                    </div>
                    {#if fieldErrors.password}
                        <span class="text-xs  text-red-400"
                            >{fieldErrors.password}</span
                        >
                    {/if}
                </div>

                {#if fieldErrors.general}
                    <p
                        class="text-sm text-red-600 dark:text-red-400 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg px-4 py-2.5"
                    >
                        {fieldErrors.general}
                    </p>
                {/if}

                {#if successMessage}
                    <p
                        class="text-sm text-green-700 dark:text-green-400 bg-green-50 dark:bg-green-900/20 border border-green-200 dark:border-green-800 rounded-lg px-4 py-2.5"
                    >
                        ✓ Muvofaqiyatli yangilandi
                    </p>
                {/if}

                <div class="flex justify-end pt-1">
                    <button
                        type="submit"
                        disabled={submitting}
                        class="h-10 px-6 rounded-lg bg-teal-600 dark:bg-teal-500 text-white text-sm font-medium
                               hover:bg-teal-700 dark:hover:bg-teal-600 active:scale-[0.98] transition
                               disabled:opacity-60 disabled:cursor-not-allowed cursor-pointer"
                    >
                        {submitting ? "Saqlanmoqda" : "Saqlash"}
                    </button>
                </div>
            </form>
        </div>

        <div
            class="bg-transparent rounded-2xl border border-red-200 dark:border-red-900/50 shadow-sm overflow-hidden"
        >
            <div
                class="px-6 py-4 border-b border-red-100 dark:border-red-900/50 bg-red-50/10 dark:bg-red-900/20"
            >
                <h2
                    class="text-base font-semibold text-red-700 dark:text-red-400"
                >
                    Xafli zona
                </h2>
                <p class="text-sm text-red-400">
                    Tizimdan chiqish
                </p>
            </div>
            <div class="px-6 py-5 flex items-center justify-between gap-4">
                <div>
                    <p class="text-sm font-medium text-teal-400">
                        Avtorizatsiya va authenticatsiya
                    </p>
                    <p class="text-xs text-teal-400 mt-0.5">Chiqish</p>
                </div>
                <button
                    onclick={logout}
                    class="shrink-0 h-10 px-5 rounded-lg border border-red-300 dark:border-red-800 text-red-600 dark:text-red-400 text-sm font-medium
                           hover:bg-red-50 dark:hover:bg-red-900/20 active:scale-[0.98] transition cursor-pointer"
                >
                    Chiqish
                </button>
            </div>
        </div>
    </main>
</div>
